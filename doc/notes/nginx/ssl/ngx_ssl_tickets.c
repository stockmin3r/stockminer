#ifdef SSL_CTRL_SET_TLSEXT_TICKET_KEY_CB

ngx_int_t
ngx_ssl_session_ticket_keys(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_array_t *paths)
{
    u_char                         buf[80];
    size_t                         size;
    ssize_t                        n;
    ngx_str_t                     *path;
    ngx_file_t                     file;
    ngx_uint_t                     i;
    ngx_array_t                   *keys;
    ngx_file_info_t                fi;
    ngx_pool_cleanup_t            *cln;
    ngx_ssl_session_ticket_key_t  *key;

    if (paths == NULL) {
        return NGX_OK;
    }

    keys = ngx_array_create(cf->pool, paths->nelts,
                            sizeof(ngx_ssl_session_ticket_key_t));
    if (keys == NULL) {
        return NGX_ERROR;
    }

    cln = ngx_pool_cleanup_add(cf->pool, 0);
    if (cln == NULL) {
        return NGX_ERROR;
    }

    cln->handler = ngx_ssl_session_ticket_keys_cleanup;
    cln->data = keys;

    path = paths->elts;
    for (i = 0; i < paths->nelts; i++) {

        if (ngx_conf_full_name(cf->cycle, &path[i], 1) != NGX_OK) {
            return NGX_ERROR;
        }

        ngx_memzero(&file, sizeof(ngx_file_t));
        file.name = path[i];
        file.log = cf->log;

        file.fd = ngx_open_file(file.name.data, NGX_FILE_RDONLY,
                                NGX_FILE_OPEN, 0);

        if (file.fd == NGX_INVALID_FILE) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, ngx_errno,
                               ngx_open_file_n " \"%V\" failed", &file.name);
            return NGX_ERROR;
        }

        if (ngx_fd_info(file.fd, &fi) == NGX_FILE_ERROR) {
            ngx_conf_log_error(NGX_LOG_CRIT, cf, ngx_errno,
                               ngx_fd_info_n " \"%V\" failed", &file.name);
            goto failed;
        }

        size = ngx_file_size(&fi);

        if (size != 48 && size != 80) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "\"%V\" must be 48 or 80 bytes", &file.name);
            goto failed;
        }

        n = ngx_read_file(&file, buf, size, 0);

        if (n == NGX_ERROR) {
            ngx_conf_log_error(NGX_LOG_CRIT, cf, ngx_errno,
                               ngx_read_file_n " \"%V\" failed", &file.name);
            goto failed;
        }

        if ((size_t) n != size) {
            ngx_conf_log_error(NGX_LOG_CRIT, cf, 0,
                               ngx_read_file_n " \"%V\" returned only "
                               "%z bytes instead of %uz", &file.name, n, size);
            goto failed;
        }

        key = ngx_array_push(keys);
        if (key == NULL) {
            goto failed;
        }

        if (size == 48) {
            key->size = 48;
            ngx_memcpy(key->name, buf, 16);
            ngx_memcpy(key->aes_key, buf + 16, 16);
            ngx_memcpy(key->hmac_key, buf + 32, 16);

        } else {
            key->size = 80;
            ngx_memcpy(key->name, buf, 16);
            ngx_memcpy(key->hmac_key, buf + 16, 32);
            ngx_memcpy(key->aes_key, buf + 48, 32);
        }

        if (ngx_close_file(file.fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, cf->log, ngx_errno,
                          ngx_close_file_n " \"%V\" failed", &file.name);
        }

        ngx_explicit_memzero(&buf, 80);
    }

    if (SSL_CTX_set_ex_data(ssl->ctx, ngx_ssl_session_ticket_keys_index, keys)
        == 0)
    {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_set_ex_data() failed");
        return NGX_ERROR;
    }

    if (SSL_CTX_set_tlsext_ticket_key_cb(ssl->ctx,
                                         ngx_ssl_session_ticket_key_callback)
        == 0)
    {
        ngx_log_error(NGX_LOG_WARN, cf->log, 0,
                      "nginx was built with Session Tickets support, however, "
                      "now it is linked dynamically to an OpenSSL library "
                      "which has no tlsext support, therefore Session Tickets "
                      "are not available");
    }

    return NGX_OK;

failed:

    if (ngx_close_file(file.fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, cf->log, ngx_errno,
                      ngx_close_file_n " \"%V\" failed", &file.name);
    }

    ngx_explicit_memzero(&buf, 80);

    return NGX_ERROR;
}


static int
ngx_ssl_session_ticket_key_callback(ngx_ssl_conn_t *ssl_conn,
    unsigned char *name, unsigned char *iv, EVP_CIPHER_CTX *ectx,
    HMAC_CTX *hctx, int enc)
{
    size_t                         size;
    SSL_CTX                       *ssl_ctx;
    ngx_uint_t                     i;
    ngx_array_t                   *keys;
    ngx_connection_t              *c;
    ngx_ssl_session_ticket_key_t  *key;
    const EVP_MD                  *digest;
    const EVP_CIPHER              *cipher;

    c = ngx_ssl_get_connection(ssl_conn);
    ssl_ctx = c->ssl->session_ctx;

#ifdef OPENSSL_NO_SHA256
    digest = EVP_sha1();
#else
    digest = EVP_sha256();
#endif

    keys = SSL_CTX_get_ex_data(ssl_ctx, ngx_ssl_session_ticket_keys_index);
    if (keys == NULL) {
        return -1;
    }

    key = keys->elts;

    if (enc == 1) {
        /* encrypt session ticket */

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "ssl session ticket encrypt, key: \"%*xs\" (%s session)",
                       (size_t) 16, key[0].name,
                       SSL_session_reused(ssl_conn) ? "reused" : "new");

        if (key[0].size == 48) {
            cipher = EVP_aes_128_cbc();
            size = 16;

        } else {
            cipher = EVP_aes_256_cbc();
            size = 32;
        }

        if (RAND_bytes(iv, EVP_CIPHER_iv_length(cipher)) != 1) {
            ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "RAND_bytes() failed");
            return -1;
        }

        if (EVP_EncryptInit_ex(ectx, cipher, NULL, key[0].aes_key, iv) != 1) {
            ngx_ssl_error(NGX_LOG_ALERT, c->log, 0,
                          "EVP_EncryptInit_ex() failed");
            return -1;
        }

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
        if (HMAC_Init_ex(hctx, key[0].hmac_key, size, digest, NULL) != 1) {
            ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "HMAC_Init_ex() failed");
            return -1;
        }
#else
        HMAC_Init_ex(hctx, key[0].hmac_key, size, digest, NULL);
#endif

        ngx_memcpy(name, key[0].name, 16);

        return 1;

    } else {
        /* decrypt session ticket */

        for (i = 0; i < keys->nelts; i++) {
            if (ngx_memcmp(name, key[i].name, 16) == 0) {
                goto found;
            }
        }

        ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "ssl session ticket decrypt, key: \"%*xs\" not found",
                       (size_t) 16, name);

        return 0;

    found:

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "ssl session ticket decrypt, key: \"%*xs\"%s",
                       (size_t) 16, key[i].name, (i == 0) ? " (default)" : "");

        if (key[i].size == 48) {
            cipher = EVP_aes_128_cbc();
            size = 16;

        } else {
            cipher = EVP_aes_256_cbc();
            size = 32;
        }

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
        if (HMAC_Init_ex(hctx, key[i].hmac_key, size, digest, NULL) != 1) {
            ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "HMAC_Init_ex() failed");
            return -1;
        }
#else
        HMAC_Init_ex(hctx, key[i].hmac_key, size, digest, NULL);
#endif

        if (EVP_DecryptInit_ex(ectx, cipher, NULL, key[i].aes_key, iv) != 1) {
            ngx_ssl_error(NGX_LOG_ALERT, c->log, 0,
                          "EVP_DecryptInit_ex() failed");
            return -1;
        }

        /* renew if TLSv1.3 */

#ifdef TLS1_3_VERSION
        if (SSL_version(ssl_conn) == TLS1_3_VERSION) {
            return 2;
        }
#endif

        /* renew if non-default key */

        if (i != 0) {
            return 2;
        }

        return 1;
    }
}


static void
ngx_ssl_session_ticket_keys_cleanup(void *data)
{
    ngx_array_t  *keys = data;

    ngx_explicit_memzero(keys->elts,
                         keys->nelts * sizeof(ngx_ssl_session_ticket_key_t));
}

#else

ngx_int_t
ngx_ssl_session_ticket_keys(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_array_t *paths)
{
    if (paths) {
        ngx_log_error(NGX_LOG_WARN, ssl->log, 0,
                      "\"ssl_session_ticket_key\" ignored, not supported");
    }

    return NGX_OK;
}

#endif
