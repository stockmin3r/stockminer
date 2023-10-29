ngx_int_t
ngx_ssl_check_host(ngx_connection_t *c, ngx_str_t *name)
{
    X509   *cert = SSL_get_peer_certificate(c->ssl->connection);

    /* X509_check_host() is only available in OpenSSL 1.0.2+ */
    if (name->len == 0)
        goto failed;

    if (X509_check_host(cert, (char *) name->data, name->len, 0, NULL) != 1) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "X509_check_host(): no match");
        goto failed;
    }
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "X509_check_host(): match");
    goto found;
failed:
    X509_free(cert);
    return NGX_ERROR;
found:
    X509_free(cert);
    return NGX_OK;
}

ngx_int_t
ngx_ssl_get_raw_certificate(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    size_t   len;
    BIO     *bio;
    X509    *cert;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "BIO_new() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

    if (PEM_write_bio_X509(bio, cert) == 0) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "PEM_write_bio_X509() failed");
        goto failed;
    }

    len = BIO_pending(bio);
    s->len = len;

    s->data = ngx_pnalloc(pool, len);
    if (s->data == NULL) {
        goto failed;
    }

    BIO_read(bio, s->data, len);

    BIO_free(bio);
    X509_free(cert);

    return NGX_OK;

failed:

    BIO_free(bio);
    X509_free(cert);

    return NGX_ERROR;
}


ngx_int_t
ngx_ssl_get_certificate(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    u_char      *p;
    size_t       len;
    ngx_uint_t   i;
    ngx_str_t    cert;

    if (ngx_ssl_get_raw_certificate(c, pool, &cert) != NGX_OK) {
        return NGX_ERROR;
    }

    if (cert.len == 0) {
        s->len = 0;
        return NGX_OK;
    }

    len = cert.len - 1;

    for (i = 0; i < cert.len - 1; i++) {
        if (cert.data[i] == LF) {
            len++;
        }
    }

    s->len = len;
    s->data = ngx_pnalloc(pool, len);
    if (s->data == NULL) {
        return NGX_ERROR;
    }

    p = s->data;

    for (i = 0; i < cert.len - 1; i++) {
        *p++ = cert.data[i];
        if (cert.data[i] == LF) {
            *p++ = '\t';
        }
    }

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_escaped_certificate(ngx_connection_t *c, ngx_pool_t *pool,
    ngx_str_t *s)
{
    ngx_str_t  cert;
    uintptr_t  n;

    if (ngx_ssl_get_raw_certificate(c, pool, &cert) != NGX_OK) {
        return NGX_ERROR;
    }

    if (cert.len == 0) {
        s->len = 0;
        return NGX_OK;
    }

    n = ngx_escape_uri(NULL, cert.data, cert.len, NGX_ESCAPE_URI_COMPONENT);

    s->len = cert.len + n * 2;
    s->data = ngx_pnalloc(pool, s->len);
    if (s->data == NULL) {
        return NGX_ERROR;
    }

    ngx_escape_uri(s->data, cert.data, cert.len, NGX_ESCAPE_URI_COMPONENT);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_subject_dn(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    BIO        *bio;
    X509       *cert;
    X509_NAME  *name;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    name = X509_get_subject_name(cert);
    if (name == NULL) {
        X509_free(cert);
        return NGX_ERROR;
    }

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "BIO_new() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

    if (X509_NAME_print_ex(bio, name, 0, XN_FLAG_RFC2253) < 0) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "X509_NAME_print_ex() failed");
        goto failed;
    }

    s->len = BIO_pending(bio);
    s->data = ngx_pnalloc(pool, s->len);
    if (s->data == NULL) {
        goto failed;
    }

    BIO_read(bio, s->data, s->len);

    BIO_free(bio);
    X509_free(cert);

    return NGX_OK;

failed:

    BIO_free(bio);
    X509_free(cert);

    return NGX_ERROR;
}


ngx_int_t
ngx_ssl_get_issuer_dn(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    BIO        *bio;
    X509       *cert;
    X509_NAME  *name;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    name = X509_get_issuer_name(cert);
    if (name == NULL) {
        X509_free(cert);
        return NGX_ERROR;
    }

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "BIO_new() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

    if (X509_NAME_print_ex(bio, name, 0, XN_FLAG_RFC2253) < 0) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "X509_NAME_print_ex() failed");
        goto failed;
    }

    s->len = BIO_pending(bio);
    s->data = ngx_pnalloc(pool, s->len);
    if (s->data == NULL) {
        goto failed;
    }

    BIO_read(bio, s->data, s->len);

    BIO_free(bio);
    X509_free(cert);

    return NGX_OK;

failed:

    BIO_free(bio);
    X509_free(cert);

    return NGX_ERROR;
}


ngx_int_t
ngx_ssl_get_subject_dn_legacy(ngx_connection_t *c, ngx_pool_t *pool,
    ngx_str_t *s)
{
    char       *p;
    size_t      len;
    X509       *cert;
    X509_NAME  *name;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    name = X509_get_subject_name(cert);
    if (name == NULL) {
        X509_free(cert);
        return NGX_ERROR;
    }

    p = X509_NAME_oneline(name, NULL, 0);
    if (p == NULL) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "X509_NAME_oneline() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

    for (len = 0; p[len]; len++) { /* void */ }

    s->len = len;
    s->data = ngx_pnalloc(pool, len);
    if (s->data == NULL) {
        OPENSSL_free(p);
        X509_free(cert);
        return NGX_ERROR;
    }

    ngx_memcpy(s->data, p, len);

    OPENSSL_free(p);
    X509_free(cert);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_issuer_dn_legacy(ngx_connection_t *c, ngx_pool_t *pool,
    ngx_str_t *s)
{
    char       *p;
    size_t      len;
    X509       *cert;
    X509_NAME  *name;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    name = X509_get_issuer_name(cert);
    if (name == NULL) {
        X509_free(cert);
        return NGX_ERROR;
    }

    p = X509_NAME_oneline(name, NULL, 0);
    if (p == NULL) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "X509_NAME_oneline() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

    for (len = 0; p[len]; len++) { /* void */ }

    s->len = len;
    s->data = ngx_pnalloc(pool, len);
    if (s->data == NULL) {
        OPENSSL_free(p);
        X509_free(cert);
        return NGX_ERROR;
    }

    ngx_memcpy(s->data, p, len);

    OPENSSL_free(p);
    X509_free(cert);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_serial_number(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    size_t   len;
    X509    *cert;
    BIO     *bio;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "BIO_new() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

    i2a_ASN1_INTEGER(bio, X509_get_serialNumber(cert));
    len = BIO_pending(bio);

    s->len = len;
    s->data = ngx_pnalloc(pool, len);
    if (s->data == NULL) {
        BIO_free(bio);
        X509_free(cert);
        return NGX_ERROR;
    }

    BIO_read(bio, s->data, len);
    BIO_free(bio);
    X509_free(cert);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_fingerprint(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    X509          *cert;
    unsigned int   len;
    u_char         buf[EVP_MAX_MD_SIZE];

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    if (!X509_digest(cert, EVP_sha1(), buf, &len)) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "X509_digest() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

    s->len = 2 * len;
    s->data = ngx_pnalloc(pool, 2 * len);
    if (s->data == NULL) {
        X509_free(cert);
        return NGX_ERROR;
    }

    ngx_hex_dump(s->data, buf, len);

    X509_free(cert);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_client_verify(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    X509        *cert;
    long         rc;
    const char  *str;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        ngx_str_set(s, "NONE");
        return NGX_OK;
    }

    X509_free(cert);

    rc = SSL_get_verify_result(c->ssl->connection);

    if (rc == X509_V_OK) {
        if (ngx_ssl_ocsp_get_status(c, &str) == NGX_OK) {
            ngx_str_set(s, "SUCCESS");
            return NGX_OK;
        }

    } else {
        str = X509_verify_cert_error_string(rc);
    }

    s->data = ngx_pnalloc(pool, sizeof("FAILED:") - 1 + ngx_strlen(str));
    if (s->data == NULL) {
        return NGX_ERROR;
    }

    s->len = ngx_sprintf(s->data, "FAILED:%s", str) - s->data;

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_client_v_start(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    BIO     *bio;
    X509    *cert;
    size_t   len;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "BIO_new() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

#if OPENSSL_VERSION_NUMBER > 0x10100000L
    ASN1_TIME_print(bio, X509_get0_notBefore(cert));
#else
    ASN1_TIME_print(bio, X509_get_notBefore(cert));
#endif

    len = BIO_pending(bio);

    s->len = len;
    s->data = ngx_pnalloc(pool, len);
    if (s->data == NULL) {
        BIO_free(bio);
        X509_free(cert);
        return NGX_ERROR;
    }

    BIO_read(bio, s->data, len);
    BIO_free(bio);
    X509_free(cert);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_client_v_end(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    BIO     *bio;
    X509    *cert;
    size_t   len;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        ngx_ssl_error(NGX_LOG_ALERT, c->log, 0, "BIO_new() failed");
        X509_free(cert);
        return NGX_ERROR;
    }

#if OPENSSL_VERSION_NUMBER > 0x10100000L
    ASN1_TIME_print(bio, X509_get0_notAfter(cert));
#else
    ASN1_TIME_print(bio, X509_get_notAfter(cert));
#endif

    len = BIO_pending(bio);

    s->len = len;
    s->data = ngx_pnalloc(pool, len);
    if (s->data == NULL) {
        BIO_free(bio);
        X509_free(cert);
        return NGX_ERROR;
    }

    BIO_read(bio, s->data, len);
    BIO_free(bio);
    X509_free(cert);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_get_client_v_remain(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    X509    *cert;
    time_t   now, end;

    s->len = 0;

    cert = SSL_get_peer_certificate(c->ssl->connection);
    if (cert == NULL) {
        return NGX_OK;
    }

#if OPENSSL_VERSION_NUMBER > 0x10100000L
    end = ngx_ssl_parse_time(X509_get0_notAfter(cert), c->log);
#else
    end = ngx_ssl_parse_time(X509_get_notAfter(cert), c->log);
#endif

    if (end == (time_t) NGX_ERROR) {
        X509_free(cert);
        return NGX_OK;
    }

    now = ngx_time();

    if (end < now + 86400) {
        ngx_str_set(s, "0");
        X509_free(cert);
        return NGX_OK;
    }

    s->data = ngx_pnalloc(pool, NGX_TIME_T_LEN);
    if (s->data == NULL) {
        X509_free(cert);
        return NGX_ERROR;
    }

    s->len = ngx_sprintf(s->data, "%T", (end - now) / 86400) - s->data;

    X509_free(cert);

    return NGX_OK;
}

ngx_int_t
ngx_ssl_certificates(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_array_t *certs,
    ngx_array_t *keys, ngx_array_t *passwords)
{
    ngx_str_t   *cert, *key;
    ngx_uint_t   i;

    cert = certs->elts;
    key = keys->elts;

    for (i = 0; i < certs->nelts; i++) {

        if (ngx_ssl_certificate(cf, ssl, &cert[i], &key[i], passwords)
            != NGX_OK)
        {
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


ngx_int_t
ngx_ssl_certificate(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_str_t *cert,
    ngx_str_t *key, ngx_array_t *passwords)
{
    char            *err;
    X509            *x509;
    EVP_PKEY        *pkey;
    STACK_OF(X509)  *chain;

    x509 = ngx_ssl_load_certificate(cf->pool, &err, cert, &chain);
    if (x509 == NULL) {
        if (err != NULL) {
            ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                          "cannot load certificate \"%s\": %s",
                          cert->data, err);
        }

        return NGX_ERROR;
    }

    if (SSL_CTX_use_certificate(ssl->ctx, x509) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_use_certificate(\"%s\") failed", cert->data);
        X509_free(x509);
        sk_X509_pop_free(chain, X509_free);
        return NGX_ERROR;
    }

    if (X509_set_ex_data(x509, ngx_ssl_certificate_name_index, cert->data)
        == 0)
    {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0, "X509_set_ex_data() failed");
        X509_free(x509);
        sk_X509_pop_free(chain, X509_free);
        return NGX_ERROR;
    }

    if (X509_set_ex_data(x509, ngx_ssl_next_certificate_index,
                      SSL_CTX_get_ex_data(ssl->ctx, ngx_ssl_certificate_index))
        == 0)
    {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0, "X509_set_ex_data() failed");
        X509_free(x509);
        sk_X509_pop_free(chain, X509_free);
        return NGX_ERROR;
    }

    if (SSL_CTX_set_ex_data(ssl->ctx, ngx_ssl_certificate_index, x509) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_set_ex_data() failed");
        X509_free(x509);
        sk_X509_pop_free(chain, X509_free);
        return NGX_ERROR;
    }

    /*
     * Note that x509 is not freed here, but will be instead freed in
     * ngx_ssl_cleanup_ctx().  This is because we need to preserve all
     * certificates to be able to iterate all of them through exdata
     * (ngx_ssl_certificate_index, ngx_ssl_next_certificate_index),
     * while OpenSSL can free a certificate if it is replaced with another
     * certificate of the same type.
     */

#ifdef SSL_CTX_set0_chain

    if (SSL_CTX_set0_chain(ssl->ctx, chain) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_set0_chain(\"%s\") failed", cert->data);
        sk_X509_pop_free(chain, X509_free);
        return NGX_ERROR;
    }

#else
    {
    int  n;

    /* SSL_CTX_set0_chain() is only available in OpenSSL 1.0.2+ */

    n = sk_X509_num(chain);

    while (n--) {
        x509 = sk_X509_shift(chain);

        if (SSL_CTX_add_extra_chain_cert(ssl->ctx, x509) == 0) {
            ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                          "SSL_CTX_add_extra_chain_cert(\"%s\") failed",
                          cert->data);
            sk_X509_pop_free(chain, X509_free);
            return NGX_ERROR;
        }
    }

    sk_X509_free(chain);
    }
#endif

    pkey = ngx_ssl_load_certificate_key(cf->pool, &err, key, passwords);
    if (pkey == NULL) {
        if (err != NULL) {
            ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                          "cannot load certificate key \"%s\": %s",
                          key->data, err);
        }

        return NGX_ERROR;
    }

    if (SSL_CTX_use_PrivateKey(ssl->ctx, pkey) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_use_PrivateKey(\"%s\") failed", key->data);
        EVP_PKEY_free(pkey);
        return NGX_ERROR;
    }

    EVP_PKEY_free(pkey);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_connection_certificate(ngx_connection_t *c, ngx_pool_t *pool,
    ngx_str_t *cert, ngx_str_t *key, ngx_array_t *passwords)
{
    char            *err;
    X509            *x509;
    EVP_PKEY        *pkey;
    STACK_OF(X509)  *chain;

    x509 = ngx_ssl_load_certificate(pool, &err, cert, &chain);
    if (x509 == NULL) {
        if (err != NULL) {
            ngx_ssl_error(NGX_LOG_ERR, c->log, 0,
                          "cannot load certificate \"%s\": %s",
                          cert->data, err);
        }

        return NGX_ERROR;
    }

    if (SSL_use_certificate(c->ssl->connection, x509) == 0) {
        ngx_ssl_error(NGX_LOG_ERR, c->log, 0,
                      "SSL_use_certificate(\"%s\") failed", cert->data);
        X509_free(x509);
        sk_X509_pop_free(chain, X509_free);
        return NGX_ERROR;
    }

    X509_free(x509);

#ifdef SSL_set0_chain

    /*
     * SSL_set0_chain() is only available in OpenSSL 1.0.2+,
     * but this function is only called via certificate callback,
     * which is only available in OpenSSL 1.0.2+ as well
     */

    if (SSL_set0_chain(c->ssl->connection, chain) == 0) {
        ngx_ssl_error(NGX_LOG_ERR, c->log, 0,
                      "SSL_set0_chain(\"%s\") failed", cert->data);
        sk_X509_pop_free(chain, X509_free);
        return NGX_ERROR;
    }

#endif

    pkey = ngx_ssl_load_certificate_key(pool, &err, key, passwords);
    if (pkey == NULL) {
        if (err != NULL) {
            ngx_ssl_error(NGX_LOG_ERR, c->log, 0,
                          "cannot load certificate key \"%s\": %s",
                          key->data, err);
        }

        return NGX_ERROR;
    }

    if (SSL_use_PrivateKey(c->ssl->connection, pkey) == 0) {
        ngx_ssl_error(NGX_LOG_ERR, c->log, 0,
                      "SSL_use_PrivateKey(\"%s\") failed", key->data);
        EVP_PKEY_free(pkey);
        return NGX_ERROR;
    }

    EVP_PKEY_free(pkey);

    return NGX_OK;
}


static X509 *
ngx_ssl_load_certificate(ngx_pool_t *pool, char **err, ngx_str_t *cert,
    STACK_OF(X509) **chain)
{
    BIO     *bio;
    X509    *x509, *temp;
    u_long   n;

    if (ngx_strncmp(cert->data, "data:", sizeof("data:") - 1) == 0) {

        bio = BIO_new_mem_buf(cert->data + sizeof("data:") - 1,
                              cert->len - (sizeof("data:") - 1));
        if (bio == NULL) {
            *err = "BIO_new_mem_buf() failed";
            return NULL;
        }

    } else {

        if (ngx_get_full_name(pool, (ngx_str_t *) &ngx_cycle->conf_prefix, cert)
            != NGX_OK)
        {
            *err = NULL;
            return NULL;
        }

        bio = BIO_new_file((char *) cert->data, "r");
        if (bio == NULL) {
            *err = "BIO_new_file() failed";
            return NULL;
        }
    }

    /* certificate itself */

    x509 = PEM_read_bio_X509_AUX(bio, NULL, NULL, NULL);
    if (x509 == NULL) {
        *err = "PEM_read_bio_X509_AUX() failed";
        BIO_free(bio);
        return NULL;
    }

    /* rest of the chain */

    *chain = sk_X509_new_null();
    if (*chain == NULL) {
        *err = "sk_X509_new_null() failed";
        BIO_free(bio);
        X509_free(x509);
        return NULL;
    }

    for ( ;; ) {

        temp = PEM_read_bio_X509(bio, NULL, NULL, NULL);
        if (temp == NULL) {
            n = ERR_peek_last_error();

            if (ERR_GET_LIB(n) == ERR_LIB_PEM
                && ERR_GET_REASON(n) == PEM_R_NO_START_LINE)
            {
                /* end of file */
                ERR_clear_error();
                break;
            }

            /* some real error */

            *err = "PEM_read_bio_X509() failed";
            BIO_free(bio);
            X509_free(x509);
            sk_X509_pop_free(*chain, X509_free);
            return NULL;
        }

        if (sk_X509_push(*chain, temp) == 0) {
            *err = "sk_X509_push() failed";
            BIO_free(bio);
            X509_free(x509);
            sk_X509_pop_free(*chain, X509_free);
            return NULL;
        }
    }

    BIO_free(bio);

    return x509;
}


static EVP_PKEY *
ngx_ssl_load_certificate_key(ngx_pool_t *pool, char **err,
    ngx_str_t *key, ngx_array_t *passwords)
{
    BIO              *bio;
    EVP_PKEY         *pkey;
    ngx_str_t        *pwd;
    ngx_uint_t        tries;
    pem_password_cb  *cb;

    if (ngx_strncmp(key->data, "engine:", sizeof("engine:") - 1) == 0) {

#ifndef OPENSSL_NO_ENGINE

        u_char  *p, *last;
        ENGINE  *engine;

        p = key->data + sizeof("engine:") - 1;
        last = (u_char *) ngx_strchr(p, ':');

        if (last == NULL) {
            *err = "invalid syntax";
            return NULL;
        }

        *last = '\0';

        engine = ENGINE_by_id((char *) p);

        if (engine == NULL) {
            *err = "ENGINE_by_id() failed";
            return NULL;
        }

        *last++ = ':';

        pkey = ENGINE_load_private_key(engine, (char *) last, 0, 0);

        if (pkey == NULL) {
            *err = "ENGINE_load_private_key() failed";
            ENGINE_free(engine);
            return NULL;
        }

        ENGINE_free(engine);

        return pkey;

#else

        *err = "loading \"engine:...\" certificate keys is not supported";
        return NULL;

#endif
    }

    if (ngx_strncmp(key->data, "data:", sizeof("data:") - 1) == 0) {

        bio = BIO_new_mem_buf(key->data + sizeof("data:") - 1,
                              key->len - (sizeof("data:") - 1));
        if (bio == NULL) {
            *err = "BIO_new_mem_buf() failed";
            return NULL;
        }

    } else {

        if (ngx_get_full_name(pool, (ngx_str_t *) &ngx_cycle->conf_prefix, key)
            != NGX_OK)
        {
            *err = NULL;
            return NULL;
        }

        bio = BIO_new_file((char *) key->data, "r");
        if (bio == NULL) {
            *err = "BIO_new_file() failed";
            return NULL;
        }
    }

    if (passwords) {
        tries = passwords->nelts;
        pwd = passwords->elts;
        cb = ngx_ssl_password_callback;

    } else {
        tries = 1;
        pwd = NULL;
        cb = NULL;
    }

    for ( ;; ) {

        pkey = PEM_read_bio_PrivateKey(bio, NULL, cb, pwd);
        if (pkey != NULL) {
            break;
        }

        if (tries-- > 1) {
            ERR_clear_error();
            (void) BIO_reset(bio);
            pwd++;
            continue;
        }

        *err = "PEM_read_bio_PrivateKey() failed";
        BIO_free(bio);
        return NULL;
    }

    BIO_free(bio);

    return pkey;
}


static int
ngx_ssl_password_callback(char *buf, int size, int rwflag, void *userdata)
{
    ngx_str_t *pwd = userdata;

    if (rwflag) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                      "ngx_ssl_password_callback() is called for encryption");
        return 0;
    }

    if (pwd == NULL) {
        return 0;
    }

    if (pwd->len > (size_t) size) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                      "password is truncated to %d bytes", size);
    } else {
        size = pwd->len;
    }

    ngx_memcpy(buf, pwd->data, size);

    return size;
}


ngx_int_t
ngx_ssl_ciphers(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_str_t *ciphers,
    ngx_uint_t prefer_server_ciphers)
{
    if (SSL_CTX_set_cipher_list(ssl->ctx, (char *) ciphers->data) == 0) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_set_cipher_list(\"%V\") failed",
                      ciphers);
        return NGX_ERROR;
    }

    if (prefer_server_ciphers) {
        SSL_CTX_set_options(ssl->ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
    }

    return NGX_OK;
}


ngx_int_t
ngx_ssl_client_certificate(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_str_t *cert,
    ngx_int_t depth)
{
    STACK_OF(X509_NAME)  *list;

    SSL_CTX_set_verify(ssl->ctx, SSL_VERIFY_PEER, ngx_ssl_verify_callback);

    SSL_CTX_set_verify_depth(ssl->ctx, depth);

    if (cert->len == 0) {
        return NGX_OK;
    }

    if (ngx_conf_full_name(cf->cycle, cert, 1) != NGX_OK) {
        return NGX_ERROR;
    }

    if (SSL_CTX_load_verify_locations(ssl->ctx, (char *) cert->data, NULL)
        == 0)
    {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_load_verify_locations(\"%s\") failed",
                      cert->data);
        return NGX_ERROR;
    }

    /*
     * SSL_CTX_load_verify_locations() may leave errors in the error queue
     * while returning success
     */

    ERR_clear_error();

    list = SSL_load_client_CA_file((char *) cert->data);

    if (list == NULL) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_load_client_CA_file(\"%s\") failed", cert->data);
        return NGX_ERROR;
    }

    SSL_CTX_set_client_CA_list(ssl->ctx, list);

    return NGX_OK;
}


ngx_int_t
ngx_ssl_trusted_certificate(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_str_t *cert,
    ngx_int_t depth)
{
    SSL_CTX_set_verify(ssl->ctx, SSL_CTX_get_verify_mode(ssl->ctx),
                       ngx_ssl_verify_callback);

    SSL_CTX_set_verify_depth(ssl->ctx, depth);

    if (cert->len == 0) {
        return NGX_OK;
    }

    if (ngx_conf_full_name(cf->cycle, cert, 1) != NGX_OK) {
        return NGX_ERROR;
    }

    if (SSL_CTX_load_verify_locations(ssl->ctx, (char *) cert->data, NULL)
        == 0)
    {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_load_verify_locations(\"%s\") failed",
                      cert->data);
        return NGX_ERROR;
    }

    /*
     * SSL_CTX_load_verify_locations() may leave errors in the error queue
     * while returning success
     */

    ERR_clear_error();

    return NGX_OK;
}


ngx_int_t
ngx_ssl_crl(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_str_t *crl)
{
    X509_STORE   *store;
    X509_LOOKUP  *lookup;

    if (crl->len == 0) {
        return NGX_OK;
    }

    if (ngx_conf_full_name(cf->cycle, crl, 1) != NGX_OK) {
        return NGX_ERROR;
    }

    store = SSL_CTX_get_cert_store(ssl->ctx);

    if (store == NULL) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "SSL_CTX_get_cert_store() failed");
        return NGX_ERROR;
    }

    lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());

    if (lookup == NULL) {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "X509_STORE_add_lookup() failed");
        return NGX_ERROR;
    }

    if (X509_LOOKUP_load_file(lookup, (char *) crl->data, X509_FILETYPE_PEM)
        == 0)
    {
        ngx_ssl_error(NGX_LOG_EMERG, ssl->log, 0,
                      "X509_LOOKUP_load_file(\"%s\") failed", crl->data);
        return NGX_ERROR;
    }

    X509_STORE_set_flags(store,
                         X509_V_FLAG_CRL_CHECK|X509_V_FLAG_CRL_CHECK_ALL);

    return NGX_OK;
}
