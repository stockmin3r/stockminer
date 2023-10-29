ngx_ssl_session_t *
ngx_ssl_get_session(ngx_connection_t *c)
{
#ifdef TLS1_3_VERSION
    if (c->ssl->session) {
        SSL_SESSION_up_ref(c->ssl->session);
        return c->ssl->session;
    }
#endif
    return SSL_get1_session(c->ssl->connection);
}


ngx_ssl_session_t *
ngx_ssl_get0_session(ngx_connection_t *c)
{
    if (c->ssl->session)
        return c->ssl->session;
    return SSL_get0_session(c->ssl->connection);
}

ngx_int_t
ngx_ssl_set_session(ngx_connection_t *c, ngx_ssl_session_t *session)
{
    if (session) {
        if (SSL_set_session(c->ssl->connection, session) == 0) {
            ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "SSL_set_session() failed");
            return NGX_ERROR;
        }
    }
    return NGX_OK;
}

ngx_int_t
ngx_ssl_get_session_id(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    u_char        *buf;
    SSL_SESSION   *sess;
    unsigned int   len;

    sess = SSL_get0_session(c->ssl->connection);
    if (sess == NULL) {
        s->len = 0;
        return NGX_OK;
    }

    buf = (u_char *) SSL_SESSION_get_id(sess, &len);

    s->len = 2 * len;
    s->data = ngx_pnalloc(pool, 2 * len);
    if (s->data == NULL) {
        return NGX_ERROR;
    }

    ngx_hex_dump(s->data, buf, len);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_session_reused(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    if (SSL_session_reused(c->ssl->connection)) {
        ngx_str_set(s, "r");

    } else {
        ngx_str_set(s, ".");
    }

    return NGX_OK;
}

ngx_int_t
ngx_ssl_session_cache(ngx_ssl_t *ssl, ngx_str_t *sess_ctx,
    ngx_array_t *certificates, ssize_t builtin_session_cache,
    ngx_shm_zone_t *shm_zone, time_t timeout)
{
    long  cache_mode;

    SSL_CTX_set_timeout(ssl->ctx, (long) timeout);

    if (ngx_ssl_session_id_context(ssl, sess_ctx, certificates) != NGX_OK) {
        return NGX_ERROR;
    }

    if (builtin_session_cache == NGX_SSL_NO_SCACHE) {
        SSL_CTX_set_session_cache_mode(ssl->ctx, SSL_SESS_CACHE_OFF);
        return NGX_OK;
    }

    if (builtin_session_cache == NGX_SSL_NONE_SCACHE) {

        /*
         * If the server explicitly says that it does not support
         * session reuse (see SSL_SESS_CACHE_OFF above), then
         * Outlook Express fails to upload a sent email to
         * the Sent Items folder on the IMAP server via a separate IMAP
         * connection in the background.  Therefore we have a special
         * mode (SSL_SESS_CACHE_SERVER|SSL_SESS_CACHE_NO_INTERNAL_STORE)
         * where the server pretends that it supports session reuse,
         * but it does not actually store any session.
         */

        SSL_CTX_set_session_cache_mode(ssl->ctx,
                                       SSL_SESS_CACHE_SERVER
                                       |SSL_SESS_CACHE_NO_AUTO_CLEAR
                                       |SSL_SESS_CACHE_NO_INTERNAL_STORE);

        SSL_CTX_sess_set_cache_size(ssl->ctx, 1);

        return NGX_OK;
    }

    cache_mode = SSL_SESS_CACHE_SERVER;

    if (shm_zone && builtin_session_cache == NGX_SSL_NO_BUILTIN_SCACHE) {
        cache_mode |= SSL_SESS_CACHE_NO_INTERNAL;
    }

    SSL_CTX_set_session_cache_mode(ssl->ctx, cache_mode);

    if (builtin_session_cache != NGX_SSL_NO_BUILTIN_SCACHE) {

        if (builtin_session_cache != NGX_SSL_DFLT_BUILTIN_SCACHE) {
            SSL_CTX_sess_set_cache_size(ssl->ctx, builtin_session_cache);
        }
    }

    if (shm_zone) {
        SSL_CTX_sess_set_new_cb(ssl->ctx, ngx_ssl_new_session);
        SSL_CTX_sess_set_get_cb(ssl->ctx, ngx_ssl_get_cached_session);
        SSL_CTX_sess_set_remove_cb(ssl->ctx, ngx_ssl_remove_session);

        if (SSL_CTX_set_ex_data(ssl->ctx, ngx_ssl_session_cache_index, shm_zone)
            == 0)
        {
            ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                          "SSL_CTX_set_ex_data() failed");
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_ssl_session_id_context(ngx_ssl_t *ssl, ngx_str_t *sess_ctx,
    ngx_array_t *certificates)
{
    int                   n, i;
    X509                 *cert;
    X509_NAME            *name;
    ngx_str_t            *certs;
    ngx_uint_t            k;
    EVP_MD_CTX           *md;
    unsigned int          len;
    STACK_OF(X509_NAME)  *list;
    u_char                buf[EVP_MAX_MD_SIZE];

    /*
     * Session ID context is set based on the string provided,
     * the server certificates, and the client CA list.
     */

    md = EVP_MD_CTX_create();
    if (md == NULL) {
        return NGX_ERROR;
    }

    if (EVP_DigestInit_ex(md, EVP_sha1(), NULL) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "EVP_DigestInit_ex() failed");
        goto failed;
    }

    if (EVP_DigestUpdate(md, sess_ctx->data, sess_ctx->len) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "EVP_DigestUpdate() failed");
        goto failed;
    }

    for (cert = SSL_CTX_get_ex_data(ssl->ctx, ngx_ssl_certificate_index);
         cert;
         cert = X509_get_ex_data(cert, ngx_ssl_next_certificate_index))
    {
        if (X509_digest(cert, EVP_sha1(), buf, &len) == 0) {
            ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                          "X509_digest() failed");
            goto failed;
        }

        if (EVP_DigestUpdate(md, buf, len) == 0) {
            ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                          "EVP_DigestUpdate() failed");
            goto failed;
        }
    }

    if (SSL_CTX_get_ex_data(ssl->ctx, ngx_ssl_certificate_index) == NULL
        && certificates != NULL)
    {
        /*
         * If certificates are loaded dynamically, we use certificate
         * names as specified in the configuration (with variables).
         */

        certs = certificates->elts;
        for (k = 0; k < certificates->nelts; k++) {

            if (EVP_DigestUpdate(md, certs[k].data, certs[k].len) == 0) {
                ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                              "EVP_DigestUpdate() failed");
                goto failed;
            }
        }
    }

    list = SSL_CTX_get_client_CA_list(ssl->ctx);

    if (list != NULL) {
        n = sk_X509_NAME_num(list);

        for (i = 0; i < n; i++) {
            name = sk_X509_NAME_value(list, i);

            if (X509_NAME_digest(name, EVP_sha1(), buf, &len) == 0) {
                ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                              "X509_NAME_digest() failed");
                goto failed;
            }

            if (EVP_DigestUpdate(md, buf, len) == 0) {
                ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                              "EVP_DigestUpdate() failed");
                goto failed;
            }
        }
    }

    if (EVP_DigestFinal_ex(md, buf, &len) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "EVP_DigestFinal_ex() failed");
        goto failed;
    }

    EVP_MD_CTX_destroy(md);

    if (SSL_CTX_set_session_id_context(ssl->ctx, buf, len) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_set_session_id_context() failed");
        return NGX_ERROR;
    }

    return NGX_OK;

failed:

    EVP_MD_CTX_destroy(md);

    return NGX_ERROR;
}


ngx_int_t
ngx_ssl_session_cache_init(ngx_shm_zone_t *shm_zone, void *data)
{
    size_t                    len;
    ngx_slab_pool_t          *shpool;
    ngx_ssl_session_cache_t  *cache;

    if (data) {
        shm_zone->data = data;
        return NGX_OK;
    }

    shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;

    if (shm_zone->shm.exists) {
        shm_zone->data = shpool->data;
        return NGX_OK;
    }

    cache = ngx_slab_alloc(shpool, sizeof(ngx_ssl_session_cache_t));
    if (cache == NULL) {
        return NGX_ERROR;
    }

    shpool->data = cache;
    shm_zone->data = cache;

    ngx_rbtree_init(&cache->session_rbtree, &cache->sentinel,
                    ngx_ssl_session_rbtree_insert_value);

    ngx_queue_init(&cache->expire_queue);

    len = sizeof(" in SSL session shared cache \"\"") + shm_zone->shm.name.len;

    shpool->log_ctx = ngx_slab_alloc(shpool, len);
    if (shpool->log_ctx == NULL) {
        return NGX_ERROR;
    }

    ngx_sprintf(shpool->log_ctx, " in SSL session shared cache \"%V\"%Z",
                &shm_zone->shm.name);

    shpool->log_nomem = 0;

    return NGX_OK;
}


/*
 * The length of the session id is 16 bytes for SSLv2 sessions and
 * between 1 and 32 bytes for SSLv3/TLSv1, typically 32 bytes.
 * It seems that the typical length of the external ASN1 representation
 * of a session is 118 or 119 bytes for SSLv3/TSLv1.
 *
 * Thus on 32-bit platforms we allocate separately an rbtree node,
 * a session id, and an ASN1 representation, they take accordingly
 * 64, 32, and 128 bytes.
 *
 * On 64-bit platforms we allocate separately an rbtree node + session_id,
 * and an ASN1 representation, they take accordingly 128 and 128 bytes.
 *
 * OpenSSL's i2d_SSL_SESSION() and d2i_SSL_SESSION are slow,
 * so they are outside the code locked by shared pool mutex
 */

static int
ngx_ssl_new_session(ngx_ssl_conn_t *ssl_conn, ngx_ssl_session_t *sess)
{
    int                       len;
    u_char                   *p, *id, *cached_sess, *session_id;
    uint32_t                  hash;
    SSL_CTX                  *ssl_ctx;
    unsigned int              session_id_length;
    ngx_shm_zone_t           *shm_zone;
    ngx_connection_t         *c;
    ngx_slab_pool_t          *shpool;
    ngx_ssl_sess_id_t        *sess_id;
    ngx_ssl_session_cache_t  *cache;
    u_char                    buf[NGX_SSL_MAX_SESSION_SIZE];

    len = i2d_SSL_SESSION(sess, NULL);

    /* do not cache too big session */

    if (len > (int) NGX_SSL_MAX_SESSION_SIZE) {
        return 0;
    }

    p = buf;
    i2d_SSL_SESSION(sess, &p);

    c = ngx_ssl_get_connection(ssl_conn);

    ssl_ctx = c->ssl->session_ctx;
    shm_zone = SSL_CTX_get_ex_data(ssl_ctx, ngx_ssl_session_cache_index);

    cache = shm_zone->data;
    shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;

    ngx_shmtx_lock(&shpool->mutex);

    /* drop one or two expired sessions */
    ngx_ssl_expire_sessions(cache, shpool, 1);

    cached_sess = ngx_slab_alloc_locked(shpool, len);

    if (cached_sess == NULL) {

        /* drop the oldest non-expired session and try once more */

        ngx_ssl_expire_sessions(cache, shpool, 0);

        cached_sess = ngx_slab_alloc_locked(shpool, len);

        if (cached_sess == NULL) {
            sess_id = NULL;
            goto failed;
        }
    }

    sess_id = ngx_slab_alloc_locked(shpool, sizeof(ngx_ssl_sess_id_t));

    if (sess_id == NULL) {

        /* drop the oldest non-expired session and try once more */

        ngx_ssl_expire_sessions(cache, shpool, 0);

        sess_id = ngx_slab_alloc_locked(shpool, sizeof(ngx_ssl_sess_id_t));

        if (sess_id == NULL) {
            goto failed;
        }
    }

    session_id = (u_char *) SSL_SESSION_get_id(sess, &session_id_length);

#if (NGX_PTR_SIZE == 8)

    id = sess_id->sess_id;

#else

    id = ngx_slab_alloc_locked(shpool, session_id_length);

    if (id == NULL) {

        /* drop the oldest non-expired session and try once more */

        ngx_ssl_expire_sessions(cache, shpool, 0);

        id = ngx_slab_alloc_locked(shpool, session_id_length);

        if (id == NULL) {
            goto failed;
        }
    }

#endif

    ngx_memcpy(cached_sess, buf, len);

    ngx_memcpy(id, session_id, session_id_length);

    hash = ngx_crc32_short(session_id, session_id_length);

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "ssl new session: %08XD:%ud:%d",
                   hash, session_id_length, len);

    sess_id->node.key = hash;
    sess_id->node.data = (u_char) session_id_length;
    sess_id->id = id;
    sess_id->len = len;
    sess_id->session = cached_sess;

    sess_id->expire = ngx_time() + SSL_CTX_get_timeout(ssl_ctx);

    ngx_queue_insert_head(&cache->expire_queue, &sess_id->queue);

    ngx_rbtree_insert(&cache->session_rbtree, &sess_id->node);

    ngx_shmtx_unlock(&shpool->mutex);

    return 0;

failed:

    if (cached_sess) {
        ngx_slab_free_locked(shpool, cached_sess);
    }

    if (sess_id) {
        ngx_slab_free_locked(shpool, sess_id);
    }

    ngx_shmtx_unlock(&shpool->mutex);

    ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                  "could not allocate new session%s", shpool->log_ctx);

    return 0;
}


static ngx_ssl_session_t *
ngx_ssl_get_cached_session(ngx_ssl_conn_t *ssl_conn,
#if OPENSSL_VERSION_NUMBER >= 0x10100003L
    const
#endif
    u_char *id, int len, int *copy)
{
    size_t                    slen;
    uint32_t                  hash;
    ngx_int_t                 rc;
    const u_char             *p;
    ngx_shm_zone_t           *shm_zone;
    ngx_slab_pool_t          *shpool;
    ngx_rbtree_node_t        *node, *sentinel;
    ngx_ssl_session_t        *sess;
    ngx_ssl_sess_id_t        *sess_id;
    ngx_ssl_session_cache_t  *cache;
    u_char                    buf[NGX_SSL_MAX_SESSION_SIZE];
    ngx_connection_t         *c;

    hash = ngx_crc32_short((u_char *) (uintptr_t) id, (size_t) len);
    *copy = 0;

    c = ngx_ssl_get_connection(ssl_conn);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "ssl get session: %08XD:%d", hash, len);

    shm_zone = SSL_CTX_get_ex_data(c->ssl->session_ctx,
                                   ngx_ssl_session_cache_index);

    cache = shm_zone->data;

    sess = NULL;

    shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;

    ngx_shmtx_lock(&shpool->mutex);

    node = cache->session_rbtree.root;
    sentinel = cache->session_rbtree.sentinel;

    while (node != sentinel) {

        if (hash < node->key) {
            node = node->left;
            continue;
        }

        if (hash > node->key) {
            node = node->right;
            continue;
        }

        /* hash == node->key */

        sess_id = (ngx_ssl_sess_id_t *) node;

        rc = ngx_memn2cmp((u_char *) (uintptr_t) id, sess_id->id,
                          (size_t) len, (size_t) node->data);

        if (rc == 0) {

            if (sess_id->expire > ngx_time()) {
                slen = sess_id->len;

                ngx_memcpy(buf, sess_id->session, slen);

                ngx_shmtx_unlock(&shpool->mutex);

                p = buf;
                sess = d2i_SSL_SESSION(NULL, &p, slen);

                return sess;
            }

            ngx_queue_remove(&sess_id->queue);

            ngx_rbtree_delete(&cache->session_rbtree, node);

            ngx_slab_free_locked(shpool, sess_id->session);
#if (NGX_PTR_SIZE == 4)
            ngx_slab_free_locked(shpool, sess_id->id);
#endif
            ngx_slab_free_locked(shpool, sess_id);

            sess = NULL;

            goto done;
        }

        node = (rc < 0) ? node->left : node->right;
    }

done:

    ngx_shmtx_unlock(&shpool->mutex);

    return sess;
}


void
ngx_ssl_remove_cached_session(SSL_CTX *ssl, ngx_ssl_session_t *sess)
{
    SSL_CTX_remove_session(ssl, sess);

    ngx_ssl_remove_session(ssl, sess);
}


static void
ngx_ssl_remove_session(SSL_CTX *ssl, ngx_ssl_session_t *sess)
{
    u_char                   *id;
    uint32_t                  hash;
    ngx_int_t                 rc;
    unsigned int              len;
    ngx_shm_zone_t           *shm_zone;
    ngx_slab_pool_t          *shpool;
    ngx_rbtree_node_t        *node, *sentinel;
    ngx_ssl_sess_id_t        *sess_id;
    ngx_ssl_session_cache_t  *cache;

    shm_zone = SSL_CTX_get_ex_data(ssl, ngx_ssl_session_cache_index);

    if (shm_zone == NULL) {
        return;
    }

    cache = shm_zone->data;

    id = (u_char *) SSL_SESSION_get_id(sess, &len);

    hash = ngx_crc32_short(id, len);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ngx_cycle->log, 0,
                   "ssl remove session: %08XD:%ud", hash, len);

    shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;

    ngx_shmtx_lock(&shpool->mutex);

    node = cache->session_rbtree.root;
    sentinel = cache->session_rbtree.sentinel;

    while (node != sentinel) {

        if (hash < node->key) {
            node = node->left;
            continue;
        }

        if (hash > node->key) {
            node = node->right;
            continue;
        }

        /* hash == node->key */

        sess_id = (ngx_ssl_sess_id_t *) node;

        rc = ngx_memn2cmp(id, sess_id->id, len, (size_t) node->data);

        if (rc == 0) {

            ngx_queue_remove(&sess_id->queue);

            ngx_rbtree_delete(&cache->session_rbtree, node);

            ngx_slab_free_locked(shpool, sess_id->session);
#if (NGX_PTR_SIZE == 4)
            ngx_slab_free_locked(shpool, sess_id->id);
#endif
            ngx_slab_free_locked(shpool, sess_id);

            goto done;
        }

        node = (rc < 0) ? node->left : node->right;
    }

done:

    ngx_shmtx_unlock(&shpool->mutex);
}


static void
ngx_ssl_expire_sessions(ngx_ssl_session_cache_t *cache,
    ngx_slab_pool_t *shpool, ngx_uint_t n)
{
    time_t              now;
    ngx_queue_t        *q;
    ngx_ssl_sess_id_t  *sess_id;

    now = ngx_time();

    while (n < 3) {

        if (ngx_queue_empty(&cache->expire_queue)) {
            return;
        }

        q = ngx_queue_last(&cache->expire_queue);

        sess_id = ngx_queue_data(q, ngx_ssl_sess_id_t, queue);

        if (n++ != 0 && sess_id->expire > now) {
            return;
        }

        ngx_queue_remove(q);

        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, ngx_cycle->log, 0,
                       "expire session: %08Xi", sess_id->node.key);

        ngx_rbtree_delete(&cache->session_rbtree, &sess_id->node);

        ngx_slab_free_locked(shpool, sess_id->session);
#if (NGX_PTR_SIZE == 4)
        ngx_slab_free_locked(shpool, sess_id->id);
#endif
        ngx_slab_free_locked(shpool, sess_id);
    }
}


static void
ngx_ssl_session_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel)
{
    ngx_rbtree_node_t  **p;
    ngx_ssl_sess_id_t   *sess_id, *sess_id_temp;

    for ( ;; ) {

        if (node->key < temp->key) {

            p = &temp->left;

        } else if (node->key > temp->key) {

            p = &temp->right;

        } else { /* node->key == temp->key */

            sess_id = (ngx_ssl_sess_id_t *) node;
            sess_id_temp = (ngx_ssl_sess_id_t *) temp;

            p = (ngx_memn2cmp(sess_id->id, sess_id_temp->id,
                              (size_t) node->data, (size_t) temp->data)
                 < 0) ? &temp->left : &temp->right;
        }

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);
}
