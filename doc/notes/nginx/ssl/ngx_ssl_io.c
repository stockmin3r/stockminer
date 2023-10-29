ngx_int_t
ngx_ssl_create_connection(ngx_ssl_t *ssl, ngx_connection_t *c, ngx_uint_t flags)
{
    ngx_ssl_connection_t  *sc;

    sc              = ngx_pcalloc(c->pool, sizeof(ngx_ssl_connection_t));
    sc->buffer      = ((flags & NGX_SSL_BUFFER) != 0);
    sc->buffer_size = ssl->buffer_size;
    sc->session_ctx = ssl->ctx;
    sc->connection  = SSL_new(ssl->ctx);
    SSL_set_fd(sc->connection, c->fd);

    if (flags & NGX_SSL_CLIENT)
        SSL_set_connect_state(sc->connection);
    else
        SSL_set_accept_state(sc->connection);

    SSL_set_ex_data(sc->connection, ngx_ssl_connection_index, c);
    c->ssl = sc;
    return NGX_OK;
}

ngx_int_t
ngx_ssl_handshake(ngx_connection_t *c)
{
    int        n, sslerr;
    ngx_err_t  err;
    ngx_int_t  rc;

    if (c->ssl->in_ocsp)
        return ngx_ssl_ocsp_validate(c);

    ngx_ssl_clear_error(c->log);
    n = SSL_do_handshake(c->ssl->connection);
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_do_handshake: %d", n);

    if (n == 1) {
        ngx_handle_read_event (c->read,  0);
        ngx_handle_write_event(c->write, 0);
        c->recv         = ngx_ssl_recv;
        c->send         = ngx_ssl_write;
        c->recv_chain   = ngx_ssl_recv_chain;
        c->send_chain   = ngx_ssl_send_chain;
        c->read->ready  = 1;
        c->write->ready = 1;

#ifdef BIO_get_ktls_send
        if (BIO_get_ktls_send(SSL_get_wbio(c->ssl->connection)) == 1) {
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "BIO_get_ktls_send(): 1");
            c->ssl->sendfile = 1;
        }
#endif
        rc = ngx_ssl_ocsp_validate(c);
        if (rc == NGX_ERROR)
            return NGX_ERROR;

        if (rc == NGX_AGAIN) {
            c->read->handler  = ngx_ssl_handshake_handler;
            c->write->handler = ngx_ssl_handshake_handler;
            return NGX_AGAIN;
        }
        c->ssl->handshaked = 1;
        return NGX_OK;
    }

    sslerr = SSL_get_error(c->ssl->connection, n);
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_get_error: %d", sslerr);

    if (sslerr == SSL_ERROR_WANT_READ) {
        c->read->ready    = 0;
        c->read->handler  = ngx_ssl_handshake_handler;
        c->write->handler = ngx_ssl_handshake_handler;
        ngx_handle_read_event (c->read,  0);
        ngx_handle_write_event(c->write, 0);
        return NGX_AGAIN;
    }

    if (sslerr == SSL_ERROR_WANT_WRITE) {
        c->write->ready   = 0;
        c->read->handler  = ngx_ssl_handshake_handler;
        c->write->handler = ngx_ssl_handshake_handler;
        ngx_handle_read_event (c->read,  0);
        ngx_handle_write_event(c->write, 0);
        return NGX_AGAIN;
    }

    err                      = (sslerr == SSL_ERROR_SYSCALL) ? ngx_errno : 0;
    c->ssl->no_wait_shutdown = 1;
    c->ssl->no_send_shutdown = 1;
    c->read->eof             = 1;

    if (sslerr == SSL_ERROR_ZERO_RETURN || ERR_peek_error() == 0) {
        ngx_connection_error(c, err, "peer closed connection in SSL handshake");
        return NGX_ERROR;
    }

    if (c->ssl->handshake_rejected) {
        ngx_connection_error(c, err, "handshake rejected");
        ERR_clear_error();
        return NGX_ERROR;
    }
    c->read->error = 1;
    ngx_ssl_connection_error(c, sslerr, err, "SSL_do_handshake() failed");
    return NGX_ERROR;
}

static void
ngx_ssl_handshake_handler(ngx_event_t *ev)
{
    ngx_connection_t  *c;

    c = ev->data;
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL handshake handler: %d", ev->write);

    if (ev->timedout) {
        c->ssl->handler(c);
        return;
    }

    if (ngx_ssl_handshake(c) == NGX_AGAIN)
        return;
    c->ssl->handler(c);
}

ssize_t
ngx_ssl_recv_chain(ngx_connection_t *c, ngx_chain_t *cl, off_t limit)
{
    u_char     *last;
    ssize_t     n, bytes, size;
    ngx_buf_t  *b;

    bytes = 0;
    b     = cl->buf;
    last  = b->last;
    for ( ;; ) {
        size = b->end - last;
        if (limit) {
            if (bytes >= limit)
                return bytes;
            if (bytes + size > limit)
                size = (ssize_t) (limit - bytes);
        }
        n = ngx_ssl_recv(c, last, size);
        if (n > 0) {
            last  += n;
            bytes += n;
            if (!c->read->ready)
                return bytes;

            if (last == b->end) {
                cl = cl->next;
                if (cl == NULL)
                    return bytes;
                b    = cl->buf;
                last = b->last;
            }
            continue;
        }
        if (bytes) {
            if (n == 0 || n == NGX_ERROR)
                c->read->ready = 1;
            return bytes;
        }
        return n;
    }
}

ssize_t
ngx_ssl_recv(ngx_connection_t *c, u_char *buf, size_t size)
{
    int  n, bytes;

#ifdef SSL_READ_EARLY_DATA_SUCCESS
    if (c->ssl->in_early) {
        return ngx_ssl_recv_early(c, buf, size);
    }
#endif

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
    /*
     * SSL_read() may return data in parts, so try to read until SSL_read() would return no data
     */
    for ( ;; ) {
        n = SSL_read(c->ssl->connection, buf, size);
        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_read: %d", n);
        if (n > 0)
            bytes += n;

        c->ssl->last = ngx_ssl_handle_recv(c, n);
        if (c->ssl->last == NGX_OK) {
            size -= n;
            if (size == 0) {
                c->read->ready = 1;
                if (c->read->available >= 0) {
                    c->read->available -= bytes;
                    /*
                     * there can be data buffered at SSL layer,
                     * so we post an event to continue reading on the next
                     * iteration of the event loop
                     */
                    if (c->read->available < 0) {
                        c->read->available = 0;
                        c->read->ready     = 0;
                        if (c->read->posted)
                            ngx_delete_posted_event(c->read);
                        ngx_post_event(c->read, &ngx_posted_next_events);
                    }
                    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_read: avail:%d", c->read->available);
                } else {
#if (NGX_HAVE_FIONREAD)
                    if (ngx_socket_nread(c->fd, &c->read->available) == -1) {
                        c->read->error = 1;
                        ngx_connection_error(c, ngx_socket_errno, ngx_socket_nread_n " failed");
                        return NGX_ERROR;
                    }
                    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_read: avail:%d", c->read->available);
#endif
                }
                return bytes;
            }
            buf += n;
            continue;
        }

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

static ngx_int_t
ngx_ssl_handle_recv(ngx_connection_t *c, int n)
{
    int        sslerr;
    ngx_err_t  err;

    if (n > 0) {
        if (c->ssl->saved_write_handler) {
            c->write->handler = c->ssl->saved_write_handler;
            c->ssl->saved_write_handler = NULL;
            c->write->ready   = 1;
            ngx_handle_write_event(c->write, 0);
            ngx_post_event(c->write, &ngx_posted_events);
        }
        return NGX_OK;
    }

    sslerr = SSL_get_error(c->ssl->connection, n);
    err    = (sslerr == SSL_ERROR_SYSCALL) ? ngx_errno : 0;
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_get_error: %d", sslerr);
    if (sslerr == SSL_ERROR_WANT_READ) {
        if (c->ssl->saved_write_handler) {
            c->write->handler = c->ssl->saved_write_handler;
            c->ssl->saved_write_handler = NULL;
            c->write->ready   = 1;
            ngx_handle_write_event(c->write, 0);
            ngx_post_event(c->write, &ngx_posted_events);
        }
        c->read->ready = 0;
        return NGX_AGAIN;
    }

    if (sslerr == SSL_ERROR_WANT_WRITE) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_read: want write");
        c->write->ready = 0;
        ngx_handle_write_event(c->write, 0);
        /* we do not set the timer because there is already the read event timer */
        if (c->ssl->saved_write_handler == NULL) {
            c->ssl->saved_write_handler = c->write->handler;
            c->write->handler           = ngx_ssl_write_handler;
        }
        return NGX_AGAIN;
    }

    c->ssl->no_wait_shutdown = 1;
    c->ssl->no_send_shutdown = 1;
    if (sslerr == SSL_ERROR_ZERO_RETURN || ERR_peek_error() == 0) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "peer shutdown SSL cleanly");
        return NGX_DONE;
    }
    ngx_ssl_connection_error(c, sslerr, err, "SSL_read() failed");
    return NGX_ERROR;
}


static void
ngx_ssl_write_handler(ngx_event_t *wev)
{
    ngx_connection_t  *c;

    c = wev->data;
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL write handler");
    c->read->handler(c->read);
}


/*
 * OpenSSL has no SSL_writev() so we copy several bufs into our 16K buffer
 * before the SSL_write() call to decrease SSL overhead.
 *
 * Besides for protocols such as HTTP it is possible to always buffer
 * the output to decrease SSL overhead some more.
 */

ngx_chain_t *
ngx_ssl_send_chain(ngx_connection_t *c, ngx_chain_t *in, off_t limit)
{
    int           n;
    ngx_uint_t    flush;
    ssize_t       send, size, file_size;
    ngx_buf_t    *buf;
    ngx_chain_t  *cl;

    if (!c->ssl->buffer) {
        while (in) {
            if (ngx_buf_special(in->buf)) {
                in = in->next;
                continue;
            }

            n = ngx_ssl_write(c, in->buf->pos, in->buf->last - in->buf->pos);
            if (n == NGX_ERROR)
                return NGX_CHAIN_ERROR;

            if (n == NGX_AGAIN)
                return in;

            in->buf->pos += n;
            if (in->buf->pos == in->buf->last)
                in = in->next;
        }
        return in;
    }


    /* the maximum limit size is the maximum int32_t value - the page size */
    if (limit == 0 || limit > (off_t) (NGX_MAX_INT32_VALUE - ngx_pagesize))
        limit = NGX_MAX_INT32_VALUE - ngx_pagesize;

    buf = c->ssl->buf;
    if (buf == NULL) {
        buf = ngx_create_temp_buf(c->pool, c->ssl->buffer_size);
        c->ssl->buf = buf;
    }

    if (buf->start == NULL) {
        buf->start = ngx_palloc(c->pool, c->ssl->buffer_size);
        buf->pos   = buf->start;
        buf->last  = buf->start;
        buf->end   = buf->start + c->ssl->buffer_size;
    }

    send  = buf->last - buf->pos;
    flush = (in == NULL) ? 1 : buf->flush;
    for ( ;; ) {
        while (in && buf->last < buf->end && send < limit) {
            if (in->buf->last_buf || in->buf->flush)
                flush = 1;

            if (ngx_buf_special(in->buf)) {
                in = in->next;
                continue;
            }

            if (in->buf->in_file && c->ssl->sendfile) {
                flush = 1;
                break;
            }

            size = in->buf->last - in->buf->pos;
            if (size > buf->end - buf->last)
                size = buf->end - buf->last;

            if (send + size > limit)
                size = (ssize_t) (limit - send);

            ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL buf copy: %z", size);
            ngx_memcpy(buf->last, in->buf->pos, size);

            buf->last    += size;
            in->buf->pos += size;
            send         += size;
            if (in->buf->pos == in->buf->last)
                in = in->next;
        }

        if (!flush && send < limit && buf->last < buf->end)
            break;

        size = buf->last - buf->pos;
        if (size == 0) {
            if (in && in->buf->in_file && send < limit) {
                /* coalesce the neighbouring file bufs */
                cl        = in;
                file_size = (size_t) ngx_chain_coalesce_file(&cl, limit - send);
                n         = ngx_ssl_sendfile(c, in->buf, file_size);

                if (n == NGX_ERROR)
                    return NGX_CHAIN_ERROR;
                if (n == NGX_AGAIN)
                    break;

                in    = ngx_chain_update_sent(in, n);
                send += n;
                flush = 0;
                continue;
            }

            buf->flush   = 0;
            c->buffered &= ~NGX_SSL_BUFFERED;
            return in;
        }

        n = ngx_ssl_write(c, buf->pos, size);
        if (n == NGX_ERROR)
            return NGX_CHAIN_ERROR;

        if (n == NGX_AGAIN)
            break;

        buf->pos += n;
        if (n < size)
            break;

        flush     = 0;
        buf->pos  = buf->start;
        buf->last = buf->start;

        if (in == NULL || send >= limit)
            break;
    }

    buf->flush = flush;
    if (buf->pos < buf->last) {
        c->buffered |= NGX_SSL_BUFFERED;
    } else {
        c->buffered &= ~NGX_SSL_BUFFERED;
    }
    return in;
}


ssize_t
ngx_ssl_write(ngx_connection_t *c, u_char *data, size_t size)
{
    int        n, sslerr;
    ngx_err_t  err;

    ngx_ssl_clear_error(c->log);
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL to write: %uz", size);
    n = SSL_write(c->ssl->connection, data, size);
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_write: %d", n);

    if (n > 0) {
        if (c->ssl->saved_read_handler) {
            c->read->handler           = c->ssl->saved_read_handler;
            c->ssl->saved_read_handler = NULL;
            c->read->ready             = 1;
            ngx_handle_read_event(c->read, 0);
            ngx_post_event       (c->read, &ngx_posted_events);
        }
        c->sent += n;
        return n;
    }

    sslerr = SSL_get_error(c->ssl->connection, n);
    err    = (sslerr == SSL_ERROR_SYSCALL) ? ngx_errno : 0;
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_get_error: %d", sslerr);
    if (sslerr == SSL_ERROR_WANT_WRITE) {
        if (c->ssl->saved_read_handler) {
            c->read->handler           = c->ssl->saved_read_handler;
            c->ssl->saved_read_handler = NULL;
            c->read->ready             = 1;
            ngx_handle_read_event(c->read, 0);
            ngx_post_event       (c->read, &ngx_posted_events);
        }

        c->write->ready = 0;
        return NGX_AGAIN;
    }

    if (sslerr == SSL_ERROR_WANT_READ) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_write: want read");
        c->read->ready = 0;
        ngx_handle_read_event(c->read, 0);
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
    ngx_ssl_connection_error(c, sslerr, err, "SSL_write() failed");
    return NGX_ERROR;
}

#ifdef BIO_get_ktls_send
static ssize_t
ngx_ssl_sendfile(ngx_connection_t *c, ngx_buf_t *file, size_t size)
{
    int        sslerr, flags;
    ssize_t    n;
    ngx_err_t  err;

    ngx_ssl_clear_error(c->log);
    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL to sendfile: @%O %uz", file->file_pos, size);
    ngx_set_errno(0);
    flags = (c->busy_count <= 2) ? SF_NODISKIO : 0;
    if (file->file->directio)
        flags |= SF_NOCACHE;

    n = SSL_sendfile(c->ssl->connection, file->file->fd, file->file_pos, size, flags);
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_sendfile: %d", n);
    if (n > 0) {
        if (c->ssl->saved_read_handler) {
            c->read->handler = c->ssl->saved_read_handler;
            c->ssl->saved_read_handler = NULL;
            c->read->ready   = 1;
            ngx_handle_read_event(c->read, 0);
            ngx_post_event(c->read, &ngx_posted_events);
        }
        c->busy_count = 0;
        c->sent += n;
        return n;
    }

    if (n == 0) {
        /*
         * if sendfile returns zero, then someone has truncated the file,
         * so the offset became beyond the end of the file
         */
        ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                      "SSL_sendfile() reported that \"%s\" was truncated at %O",
                      file->file->name.data, file->file_pos);
        return NGX_ERROR;
    }

    sslerr = SSL_get_error(c->ssl->connection, n);
    if (sslerr == SSL_ERROR_SSL
        && ERR_GET_REASON(ERR_peek_error()) == SSL_R_UNINITIALIZED
        && ngx_errno != 0)
    {
        /*
         * OpenSSL fails to return SSL_ERROR_SYSCALL if an error
         * happens in sendfile(), and returns SSL_ERROR_SSL with
         * SSL_R_UNINITIALIZED reason instead
         */
        sslerr = SSL_ERROR_SYSCALL;
    }

    err = (sslerr == SSL_ERROR_SYSCALL) ? ngx_errno : 0;
    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_get_error: %d", sslerr);

    if (sslerr == SSL_ERROR_WANT_WRITE) {
        if (c->ssl->saved_read_handler) {
            c->read->handler           = c->ssl->saved_read_handler;
            c->ssl->saved_read_handler = NULL;
            c->read->ready             = 1;
            ngx_handle_read_event(c->read, 0);
            ngx_post_event(c->read, &ngx_posted_events);
        }

        if (ngx_errno == EBUSY) {
            c->busy_count++;
            ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_sendfile() busy, count:%d", c->busy_count);
            if (c->write->posted)
                ngx_delete_posted_event(c->write);
            ngx_post_event(c->write, &ngx_posted_next_events);
        }
        c->write->ready = 0;
        return NGX_AGAIN;
    }

    if (sslerr == SSL_ERROR_WANT_READ) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_sendfile: want read");
        c->read->ready = 0;
        ngx_handle_read_event(c->read, 0);
        /* we do not set the timer because there is already the write event timer */
        if (c->ssl->saved_read_handler == NULL) {
            c->ssl->saved_read_handler = c->read->handler;
            c->read->handler           = ngx_ssl_read_handler;
        }
        return NGX_AGAIN;
    }
    c->ssl->no_wait_shutdown = 1;
    c->ssl->no_send_shutdown = 1;
    c->write->error = 1;
    ngx_ssl_connection_error(c, sslerr, err, "SSL_sendfile() failed");
    return NGX_ERROR;
}
#endif

static void
ngx_ssl_read_handler(ngx_event_t *rev)
{
    ngx_connection_t  *c;

    c = rev->data;
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL read handler");
    c->write->handler(c->write);
}


void
ngx_ssl_free_buffer(ngx_connection_t *c)
{
    if (c->ssl->buf && c->ssl->buf->start) {
        if (ngx_pfree(c->pool, c->ssl->buf->start) == NGX_OK) {
            c->ssl->buf->start = NULL;
        }
    }
}


ngx_int_t
ngx_ssl_shutdown(ngx_connection_t *c)
{
    int         n, sslerr, mode;
    ngx_int_t   rc;
    ngx_err_t   err;
    ngx_uint_t  tries;

    rc = NGX_OK;
    ngx_ssl_ocsp_cleanup(c);
    if (c->timedout || c->error || c->buffered) {
        mode = SSL_RECEIVED_SHUTDOWN|SSL_SENT_SHUTDOWN;
        SSL_set_quiet_shutdown(c->ssl->connection, 1);
    } else {
        mode = SSL_get_shutdown(c->ssl->connection);
        if (c->ssl->no_wait_shutdown)
            mode |= SSL_RECEIVED_SHUTDOWN;

        if (c->ssl->no_send_shutdown)
            mode |= SSL_SENT_SHUTDOWN;

        if (c->ssl->no_wait_shutdown && c->ssl->no_send_shutdown)
            SSL_set_quiet_shutdown(c->ssl->connection, 1);
    }

    SSL_set_shutdown(c->ssl->connection, mode);
    ngx_ssl_clear_error(c->log);
    tries = 2;
    for ( ;; ) {
        /*
         * For bidirectional shutdown, SSL_shutdown() needs to be called
         * twice: first call sends the "close notify" alert and returns 0,
         * second call waits for the peer's "close notify" alert.
         */

        n = SSL_shutdown(c->ssl->connection);
        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_shutdown: %d", n);
        if (n == 1)
            goto done;

        if (n == 0 && tries-- > 1)
            continue;

        sslerr = SSL_get_error(c->ssl->connection, n);
        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_get_error: %d", sslerr);
        if (sslerr == SSL_ERROR_WANT_READ || sslerr == SSL_ERROR_WANT_WRITE) {
            c->read->handler  = ngx_ssl_shutdown_handler;
            c->write->handler = ngx_ssl_shutdown_handler;

            if (sslerr == SSL_ERROR_WANT_READ) {
                c->read->ready  = 0;
            } else {
                c->write->ready = 0;
            }

            ngx_handle_read_event (c->read,  0);
            ngx_handle_write_event(c->write, 0);
            ngx_add_timer         (c->read,  3000);
            return NGX_AGAIN;
        }

        if (sslerr == SSL_ERROR_ZERO_RETURN || ERR_peek_error() == 0)
            goto done;

        err = (sslerr == SSL_ERROR_SYSCALL) ? ngx_errno : 0;
        ngx_ssl_connection_error(c, sslerr, err, "SSL_shutdown() failed");
        break;
    }

failed:
    rc = NGX_ERROR;
done:
    if (c->ssl->shutdown_without_free) {
        c->ssl->shutdown_without_free = 0;
        c->recv = ngx_recv;
        return rc;
    }

    SSL_free(c->ssl->connection);
    c->ssl  = NULL;
    c->recv = ngx_recv;
    return rc;
}


static void
ngx_ssl_shutdown_handler(ngx_event_t *ev)
{
    ngx_connection_t           *c;
    ngx_connection_handler_pt   handler;

    c       = ev->data;
    handler = c->ssl->handler;
    if (ev->timedout)
        c->timedout = 1;

    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, ev->log, 0, "SSL shutdown handler");
    if (ngx_ssl_shutdown(c) == NGX_AGAIN)
        return;
    handler(c);
}


