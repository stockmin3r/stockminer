static int
ngx_ssl_verify_callback(int ok, X509_STORE_CTX *x509_store)
{
#if (NGX_DEBUG)
    char              *subject, *issuer;
    int                err, depth;
    X509              *cert;
    X509_NAME         *sname, *iname;
    ngx_connection_t  *c;
    ngx_ssl_conn_t    *ssl_conn;

    ssl_conn = X509_STORE_CTX_get_ex_data(x509_store, SSL_get_ex_data_X509_STORE_CTX_idx());
    c        = ngx_ssl_get_connection(ssl_conn);

    if (!(c->log->log_level & NGX_LOG_DEBUG_EVENT))
        return 1;

    cert  = X509_STORE_CTX_get_current_cert(x509_store);
    err   = X509_STORE_CTX_get_error(x509_store);
    depth = X509_STORE_CTX_get_error_depth(x509_store);
    sname = X509_get_subject_name(cert);

    if (sname) {
        subject = X509_NAME_oneline(sname, NULL, 0);
    } else {
        subject = NULL;
    }

    iname = X509_get_issuer_name(cert);
    if (iname) {
        issuer = X509_NAME_oneline(iname, NULL, 0);
    } else {
        issuer = NULL;
    }

    ngx_log_debug5(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "verify:%d, error:%d, depth:%d, "
                   "subject:\"%s\", issuer:\"%s\"",
                   ok, err, depth,
                   subject ? subject : "(none)",
                   issuer ? issuer : "(none)");

    if (subject)
        OPENSSL_free(subject);
    if (issuer)
        OPENSSL_free(issuer);
#endif
    return 1;
}


#if (NGX_DEBUG)

static void
ngx_ssl_handshake_log(ngx_connection_t *c)
{
    char         buf[129], *s, *d;
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
    const
#endif
    SSL_CIPHER  *cipher;

    if (!(c->log->log_level & NGX_LOG_DEBUG_EVENT)) {
        return;
    }

    cipher = SSL_get_current_cipher(c->ssl->connection);

    if (cipher) {
        SSL_CIPHER_description(cipher, &buf[1], 128);

        for (s = &buf[1], d = buf; *s; s++) {
            if (*s == ' ' && *d == ' ') {
                continue;
            }

            if (*s == LF || *s == CR) {
                continue;
            }

            *++d = *s;
        }

        if (*d != ' ') {
            d++;
        }

        *d = '\0';

        ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "SSL: %s, cipher: \"%s\"",
                       SSL_get_version(c->ssl->connection), &buf[1]);

        if (SSL_session_reused(c->ssl->connection)) {
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0,
                           "SSL reused session");
        }

    } else {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "SSL no shared ciphers");
    }
}

#endif
