ngx_int_t
ngx_ssl_get_early_data(ngx_connection_t *c, ngx_pool_t *pool, ngx_str_t *s)
{
    s->len = 0;

#ifdef SSL_ERROR_EARLY_DATA_REJECTED

    /* BoringSSL */

    if (SSL_in_early_data(c->ssl->connection)) {
        ngx_str_set(s, "1");
    }

#elif defined SSL_READ_EARLY_DATA_SUCCESS

    /* OpenSSL */

    if (!SSL_is_init_finished(c->ssl->connection)) {
        ngx_str_set(s, "1");
    }

#endif

    return NGX_OK;
}

ngx_int_t
ngx_ssl_early_data(ngx_conf_t *cf, ngx_ssl_t *ssl, ngx_uint_t enable)
{
    if (!enable) {
        return NGX_OK;
    }

#ifdef SSL_ERROR_EARLY_DATA_REJECTED

    /* BoringSSL */

    SSL_CTX_set_early_data_enabled(ssl->ctx, 1);

#elif defined SSL_READ_EARLY_DATA_SUCCESS

    /* OpenSSL */

    SSL_CTX_set_max_early_data(ssl->ctx, NGX_SSL_BUFSIZE);

#else
    ngx_log_error(NGX_LOG_WARN, ssl->log, 0,
                  "\"ssl_early_data\" is not supported on this platform, "
                  "ignored");
#endif

    return NGX_OK;
}

static ssize_t
ngx_ssl_write_early(ngx_connection_t *c, u_char *data, size_t size)
{
    int        n, sslerr;
    size_t     written;
    ngx_err_t  err;

    ngx_ssl_clear_error(c->log);

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL to write: %uz", size);

    written = 0;

    n = SSL_write_early_data(c->ssl->connection, data, size, &written);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "SSL_write_early_data: %d, %uz", n, written);

    if (n > 0) {

        if (c->ssl->saved_read_handler) {

            c->read->handler = c->ssl->saved_read_handler;
            c->ssl->saved_read_handler = NULL;
            c->read->ready = 1;

            if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
                return NGX_ERROR;
            }

            ngx_post_event(c->read, &ngx_posted_events);
        }

        if (c->ssl->write_blocked) {
            c->ssl->write_blocked = 0;
            ngx_post_event(c->read, &ngx_posted_events);
        }

        c->sent += written;

        return written;
    }

    sslerr = SSL_get_error(c->ssl->connection, n);

    err = (sslerr == SSL_ERROR_SYSCALL) ? ngx_errno : 0;

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_get_error: %d", sslerr);

    if (sslerr == SSL_ERROR_WANT_WRITE) {

        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "SSL_write_early_data: want write");

        if (c->ssl->saved_read_handler) {

            c->read->handler = c->ssl->saved_read_handler;
            c->ssl->saved_read_handler = NULL;
            c->read->ready = 1;

            if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
                return NGX_ERROR;
            }

            ngx_post_event(c->read, &ngx_posted_events);
        }

        /*
         * OpenSSL 1.1.1a fails to handle SSL_read_early_data()
         * if an SSL_write_early_data() call blocked on writing,
         * see https://github.com/openssl/openssl/issues/7757
         */

        c->ssl->write_blocked = 1;

        c->write->ready = 0;
        return NGX_AGAIN;
    }

    if (sslerr == SSL_ERROR_WANT_READ) {

        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "SSL_write_early_data: want read");

        c->read->ready = 0;

        if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
            return NGX_ERROR;
        }

        /*
         * we do not set the timer because there is already
         * the write event timer
         */

        if (c->ssl->saved_read_handler == NULL) {
            c->ssl->saved_read_handler = c->read->handler;
            c->read->handler = ngx_ssl_read_handler;
        }

        return NGX_AGAIN;
    }

    c->ssl->no_wait_shutdown = 1;
    c->ssl->no_send_shutdown = 1;
    c->write->error = 1;

    ngx_ssl_connection_error(c, sslerr, err, "SSL_write_early_data() failed");

    return NGX_ERROR;
}

static ssize_t
ngx_ssl_recv_early(ngx_connection_t *c, u_char *buf, size_t size)
{
    int        n, bytes;
    size_t     readbytes;

    if (c->ssl->last == NGX_ERROR) {
        c->read->error = 1;
        return NGX_ERROR;
    }

    if (c->ssl->last == NGX_DONE) {
        c->read->ready = 0;
        c->read->eof = 1;
        return 0;
    }

    bytes = 0;

    ngx_ssl_clear_error(c->log);

    if (c->ssl->early_preread) {

        if (size == 0) {
            c->read->ready = 0;
            c->read->eof = 1;
            return 0;
        }

        *buf = c->ssl->early_buf;

        c->ssl->early_preread = 0;

        bytes = 1;
        size -= 1;
        buf += 1;
    }

    if (c->ssl->write_blocked) {
        return NGX_AGAIN;
    }

    /*
     * SSL_read_early_data() may return data in parts, so try to read
     * until SSL_read_early_data() would return no data
     */
    for ( ;; ) {
        readbytes = 0;
        n = SSL_read_early_data(c->ssl->connection, buf, size, &readbytes);
        ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,"SSL_read_early_data: %d, %uz", n, readbytes);

        if (n == SSL_READ_EARLY_DATA_SUCCESS) {
            c->ssl->last = ngx_ssl_handle_recv(c, 1);
            bytes += readbytes;
            size -= readbytes;
            if (size == 0) {
                c->read->ready = 1;
                return bytes;
            }
            buf += readbytes;
            continue;
        }
        if (n == SSL_READ_EARLY_DATA_FINISH) {
            c->ssl->last = ngx_ssl_handle_recv(c, 1);
            c->ssl->in_early = 0;
            if (bytes) {
                c->read->ready = 1;
                return bytes;
            }
            return ngx_ssl_recv(c, buf, size);
        }
        /* SSL_READ_EARLY_DATA_ERROR */
        c->ssl->last = ngx_ssl_handle_recv(c, 0);
        if (bytes) {
            if (c->ssl->last != NGX_AGAIN)
                c->read->ready = 1;
            return bytes;
        }

        switch (c->ssl->last) {
        case NGX_DONE:
            c->read->ready = 0;
            c->read->eof = 1;
            return 0;
        case NGX_ERROR:
            c->read->error = 1;
        case NGX_AGAIN:
            return c->ssl->last;
        }
    }
}
#endif


static ngx_int_t
ngx_ssl_try_early_data(ngx_connection_t *c)
{
    int        n, sslerr;
    u_char     buf;
    size_t     readbytes;
    ngx_err_t  err;
    ngx_int_t  rc;

    ngx_ssl_clear_error(c->log);

    readbytes = 0;

    n = SSL_read_early_data(c->ssl->connection, &buf, 1, &readbytes);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "SSL_read_early_data: %d, %uz", n, readbytes);

    if (n == SSL_READ_EARLY_DATA_FINISH) {
        c->ssl->try_early_data = 0;
        return ngx_ssl_handshake(c);
    }

    if (n == SSL_READ_EARLY_DATA_SUCCESS) {

        if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
            return NGX_ERROR;
        }

        if (ngx_handle_write_event(c->write, 0) != NGX_OK) {
            return NGX_ERROR;
        }

#if (NGX_DEBUG)
        ngx_ssl_handshake_log(c);
#endif

        c->ssl->try_early_data = 0;

        c->ssl->early_buf = buf;
        c->ssl->early_preread = 1;

        c->ssl->in_early = 1;

        c->recv = ngx_ssl_recv;
        c->send = ngx_ssl_write;
        c->recv_chain = ngx_ssl_recv_chain;
        c->send_chain = ngx_ssl_send_chain;

        c->read->ready = 1;
        c->write->ready = 1;

#ifdef BIO_get_ktls_send

        if (BIO_get_ktls_send(SSL_get_wbio(c->ssl->connection)) == 1) {
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0,
                           "BIO_get_ktls_send(): 1");
            c->ssl->sendfile = 1;
        }

#endif

        rc = ngx_ssl_ocsp_validate(c);

        if (rc == NGX_ERROR) {
            return NGX_ERROR;
        }

        if (rc == NGX_AGAIN) {
            c->read->handler = ngx_ssl_handshake_handler;
            c->write->handler = ngx_ssl_handshake_handler;
            return NGX_AGAIN;
        }

        c->ssl->handshaked = 1;

        return NGX_OK;
    }

    /* SSL_READ_EARLY_DATA_ERROR */

    sslerr = SSL_get_error(c->ssl->connection, n);

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_get_error: %d", sslerr);

    if (sslerr == SSL_ERROR_WANT_READ) {
        c->read->ready = 0;
        c->read->handler = ngx_ssl_handshake_handler;
        c->write->handler = ngx_ssl_handshake_handler;

        if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
            return NGX_ERROR;
        }

        if (ngx_handle_write_event(c->write, 0) != NGX_OK) {
            return NGX_ERROR;
        }

        return NGX_AGAIN;
    }

    if (sslerr == SSL_ERROR_WANT_WRITE) {
        c->write->ready = 0;
        c->read->handler = ngx_ssl_handshake_handler;
        c->write->handler = ngx_ssl_handshake_handler;

        if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
            return NGX_ERROR;
        }

        if (ngx_handle_write_event(c->write, 0) != NGX_OK) {
            return NGX_ERROR;
        }

        return NGX_AGAIN;
    }

    err = (sslerr == SSL_ERROR_SYSCALL) ? ngx_errno : 0;

    c->ssl->no_wait_shutdown = 1;
    c->ssl->no_send_shutdown = 1;
    c->read->eof = 1;

    if (sslerr == SSL_ERROR_ZERO_RETURN || ERR_peek_error() == 0) {
        ngx_connection_error(c, err,
                             "peer closed connection in SSL handshake");

        return NGX_ERROR;
    }

    c->read->error = 1;

    ngx_ssl_connection_error(c, sslerr, err, "SSL_read_early_data() failed");

    return NGX_ERROR;
}

#endif
