// redis
typedef struct tls_connection {
    connection c;
    int        flags;
    SSL       *ssl;
    char      *ssl_error;
    listNode  *pending_list_node;
} tls_connection;


static connection *createTLSConnection(int client_side) {
    SSL_CTX *ctx = redis_tls_ctx;
    if (client_side && redis_tls_client_ctx)
        ctx = redis_tls_client_ctx;

    tls_connection *conn = zcalloc(sizeof(tls_connection));
    conn->c.type         = &CT_TLS;
    conn->c.fd           = -1;
    conn->c.iovcnt       = IOV_MAX;
    conn->ssl            = SSL_new(ctx);
    return (connection *)conn;
}

static int handleSSLReturnCode(tls_connection *conn, int ret_value, WantIOType *want) {
    if (ret_value <= 0) {
        int ssl_err = SSL_get_error(conn->ssl, ret_value);
        switch (ssl_err) {
            case SSL_ERROR_WANT_WRITE:
                *want = WANT_WRITE;
                return 0;
            case SSL_ERROR_WANT_READ:
                *want = WANT_READ;
                return 0;
            case SSL_ERROR_SYSCALL:
                conn->c.last_errno = errno;
                if (conn->ssl_error) zfree(conn->ssl_error);
                conn->ssl_error = errno ? zstrdup(strerror(errno)) : NULL;
                break;
            default:
                /* Error! */
                updateTLSError(conn);
                break;
        }
        return ssl_err;
    }
    return 0;
}
/* Handle OpenSSL return code following SSL_write() or SSL_read():
 *
 * - Updates conn state and last_errno.
 * - If update_event is nonzero, calls updateSSLEvent() when necessary.
 *
 * Returns ret_value, or -1 on error or dropped connection.
 */
static int updateStateAfterSSLIO(tls_connection *conn, int ret_value, int update_event) {
    /* If system call was interrupted, there's no need to go through the full
     * OpenSSL error handling and just report this for the caller to retry the operation.
     */
    if (errno == EINTR) {
        conn->c.last_errno = EINTR;
        return -1;
    }

    if (ret_value <= 0) {
        WantIOType want = 0;
        int ssl_err;
        if (!(ssl_err = handleSSLReturnCode(conn, ret_value, &want))) {
            if (want == WANT_READ)  conn->flags |= TLS_CONN_FLAG_WRITE_WANT_READ;
            if (want == WANT_WRITE) conn->flags |= TLS_CONN_FLAG_READ_WANT_WRITE;
            if (update_event)
				updateSSLEvent(conn);
            errno = EAGAIN;
            return -1;
        } else {
            if (ssl_err == SSL_ERROR_ZERO_RETURN || ((ssl_err == SSL_ERROR_SYSCALL && !errno))) {
                conn->c.state = CONN_STATE_CLOSED;
                return -1;
            } else {
                conn->c.state = CONN_STATE_ERROR;
                return -1;
            }
        }
    }
    return ret_value;
}

static void registerSSLEvent(tls_connection *conn, WantIOType want) {
    int mask = aeGetFileEvents(server.el, conn->c.fd);

    switch (want) {
        case WANT_READ:
            if   (mask & AE_WRITABLE)  aeDeleteFileEvent(server.el, conn->c.fd, AE_WRITABLE);
            if (!(mask & AE_READABLE)) aeCreateFileEvent(server.el, conn->c.fd, AE_READABLE, tlsEventHandler, conn);
            break;
        case WANT_WRITE:
            if   (mask & AE_READABLE)  aeDeleteFileEvent(server.el, conn->c.fd, AE_READABLE);
            if (!(mask & AE_WRITABLE)) aeCreateFileEvent(server.el, conn->c.fd, AE_WRITABLE, tlsEventHandler, conn);
            break;
        default:
            serverAssert(0);
            break;
    }
}

static void updateSSLEvent(tls_connection *conn) {
    int mask       = aeGetFileEvents(server.el, conn->c.fd);
    int need_read  = conn->c.read_handler  || (conn->flags & TLS_CONN_FLAG_WRITE_WANT_READ);
    int need_write = conn->c.write_handler || (conn->flags & TLS_CONN_FLAG_READ_WANT_WRITE);

    if (need_read && !(mask & AE_READABLE))
        aeCreateFileEvent(server.el, conn->c.fd, AE_READABLE, tlsEventHandler, conn);
    if (!need_read && (mask & AE_READABLE))
        aeDeleteFileEvent(server.el, conn->c.fd, AE_READABLE);

    if (need_write && !(mask & AE_WRITABLE))
        aeCreateFileEvent(server.el, conn->c.fd, AE_WRITABLE, tlsEventHandler, conn);
    if (!need_write && (mask & AE_WRITABLE))
        aeDeleteFileEvent(server.el, conn->c.fd, AE_WRITABLE);
}

/* ae.c */
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileProc *proc, void *clientData)
{
    if (fd >= eventLoop->setsize) {
        errno = ERANGE;
        return AE_ERR;
    }
    aeFileEvent *fe = &eventLoop->events[fd];

	/*** epoll/kqueue ***/
    aeApiAddEvent(eventLoop, fd, mask);

    fe->mask |= mask;
    if (mask & AE_READABLE) fe->rfileProc = proc;
    if (mask & AE_WRITABLE) fe->wfileProc = proc;
    fe->clientData = clientData;
    if (fd > eventLoop->maxfd)
        eventLoop->maxfd = fd;
    return AE_OK;
}

static int aeApiAddEvent(aeEventLoop *eventLoop, int fd, int mask)
{
    aeApiState        *state = eventLoop->apidata;
    struct epoll_event ee    = {0};
    /* If the fd was already monitored for some event, we need a MOD operation. Otherwise we need an ADD operation. */
    int op = eventLoop->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    mask     |= eventLoop->events[fd].mask; /* Merge old events */
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.fd = fd;
    epoll_ctl(state->epfd,op,fd,&ee);
    return 0;
}

static int connTLSAccept(connection *_conn, ConnectionCallbackFunc accept_handler) {
    tls_connection *conn = (tls_connection *) _conn;
    int ret;

    if (conn->c.state != CONN_STATE_ACCEPTING) return C_ERR;
    ERR_clear_error();

    /* Try to accept */
    conn->c.conn_handler = accept_handler;
    ret = SSL_accept(conn->ssl);

    if (ret <= 0) {
        WantIOType want = 0;
        if (!handleSSLReturnCode(conn, ret, &want)) {
            registerSSLEvent(conn, want);   /* We'll fire back */
            return C_OK;
        } else {
            conn->c.state = CONN_STATE_ERROR;
            return C_ERR;
        }
    }

    conn->c.state = CONN_STATE_CONNECTED;
    if (!callHandler((connection *) conn, conn->c.conn_handler)) return C_OK;
    conn->c.conn_handler = NULL;
    return C_OK;
}

static int connTLSConnect(connection *conn_, const char *addr, int port, const char *src_addr, ConnectionCallbackFunc connect_handler) {
    tls_connection *conn = (tls_connection *) conn_;
    unsigned char addr_buf[sizeof(struct in6_addr)];

    if (conn->c.state != CONN_STATE_NONE) return C_ERR;
    ERR_clear_error();

    /* Check whether addr is an IP address, if not, use the value for Server Name Indication */
    if (inet_pton(AF_INET, addr, addr_buf) != 1 && inet_pton(AF_INET6, addr, addr_buf) != 1)
        SSL_set_tlsext_host_name(conn->ssl, addr);

    /* Initiate Socket connection first */
    if (connectionTypeTcp()->connect(conn_, addr, port, src_addr, connect_handler) == C_ERR) return C_ERR;

    /* Return now, once the socket is connected we'll initiate TLS connection from the event handler. */
    return C_OK;
}

static int connTLSWrite(connection *conn_, const void *data, size_t data_len) {
    tls_connection *conn = (tls_connection *) conn_;
    int ret;

    if (conn->c.state != CONN_STATE_CONNECTED) return -1;
    ERR_clear_error();
    ret = SSL_write(conn->ssl, data, data_len);
    return updateStateAfterSSLIO(conn, ret, 1);
}


/* ***********************************************************
 *
 *          NGINX
 *
 * - NGX_CLEAR_EVENT == EPOLLET (on linux) - (Edge Triggered)
 *
 * - ngx_epoll_process_events()
 * - ngx_http_ssl_handshake()
 * - ngx_ssl_create_connection()
 * - ngx_handle_read/write_event(): flags is unused on bsd/linux
 *************************************************************/
int ngx_handle_read_event(ngx_event_t *rev, ngx_uint_t flags)
{
    if (ngx_event_flags & NGX_USE_CLEAR_EVENT) {
        if (!rev->active && !rev->ready)
            ngx_add_event(rev, NGX_READ_EVENT, NGX_CLEAR_EVENT);
        return NGX_OK;
	}
    return NGX_OK;
}

int ngx_handle_write_event(ngx_event_t *wev, size_t lowat)
{
    ngx_connection_t  *c;

    if (lowat) {
        c = wev->data;
        ngx_send_lowat(c, lowat);
    }
    if (ngx_event_flags & NGX_USE_CLEAR_EVENT) {
        if (!wev->active && !wev->ready)
            ngx_add_event(wev, NGX_WRITE_EVENT, NGX_CLEAR_EVENT | (lowat ? NGX_LOWAT_EVENT : 0));
        return NGX_OK;
    }
    return NGX_OK;
}

int ngx_ssl_create_connection(ngx_ssl_t *ssl, ngx_connection_t *c, ngx_uint_t flags)
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


static void ngx_http_ssl_handshake(ngx_event_t *rev)
{
    u_char                    *p, buf[NGX_PROXY_PROTOCOL_MAX_HEADER + 1];
    size_t                     size;
    ssize_t                    n;
    ngx_err_t                  err;
    ngx_int_t                  rc;
    ngx_connection_t          *c;
    ngx_http_connection_t     *hc;
    ngx_http_ssl_srv_conf_t   *sscf;
    ngx_http_core_loc_conf_t  *clcf;
    ngx_http_core_srv_conf_t  *cscf;

    c  = rev->data;
    hc = c->data;
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, rev->log, 0, "http check ssl handshake");

    if (rev->timedout) {
        ngx_log_error(NGX_LOG_INFO, c->log, NGX_ETIMEDOUT, "client timed out");
        ngx_http_close_connection(c);
        return;
    }

    if (c->close) {
        ngx_http_close_connection(c);
        return;
    }

    size = hc->proxy_protocol ? sizeof(buf) : 1;
    n    = recv(c->fd, (char *) buf, size, MSG_PEEK);
    err  = ngx_socket_errno;
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, rev->log, 0, "http recv(): %z", n);

    if (n == -1) {
        if (err == NGX_EAGAIN) {
            rev->ready = 0;
            if (!rev->timer_set) {
                cscf = ngx_http_get_module_srv_conf(hc->conf_ctx, ngx_http_core_module);
                ngx_add_timer(rev, cscf->client_header_timeout);
                ngx_reusable_connection(c, 1);
            }

            if (ngx_handle_read_event(rev, 0) != NGX_OK)
                ngx_http_close_connection(c);
            return;
        }
        ngx_connection_error(c, err, "recv() failed");
        ngx_http_close_connection(c);
        return;
    }

    if (n == 1) {
        if (buf[0] & 0x80 /* SSLv2 */ || buf[0] == 0x16 /* SSLv3/TLSv1 */) {
            ngx_log_debug1(NGX_LOG_DEBUG_HTTP, rev->log, 0, "https ssl handshake: 0x%02Xd", buf[0]);

            clcf = ngx_http_get_module_loc_conf(hc->conf_ctx, ngx_http_core_module);
            if (clcf->tcp_nodelay && ngx_tcp_nodelay(c) != NGX_OK) {
                ngx_http_close_connection(c);
                return;
            }

            sscf = ngx_http_get_module_srv_conf(hc->conf_ctx, ngx_http_ssl_module);
            ngx_ssl_create_connection(&sscf->ssl, c, NGX_SSL_BUFFER);
            ngx_reusable_connection(c, 0);

            rc = ngx_ssl_handshake(c);
            if (rc == NGX_AGAIN) {
                if (!rev->timer_set) {
                    cscf = ngx_http_get_module_srv_conf(hc->conf_ctx, ngx_http_core_module);
                    ngx_add_timer(rev, cscf->client_header_timeout);
                }
                c->ssl->handler = ngx_http_ssl_handshake_handler;
                return;
            }
            ngx_http_ssl_handshake_handler(c);
            return;
        }

        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, rev->log, 0, "plain http");
        c->log->action = "waiting for request";
        rev->handler   = ngx_http_wait_request_handler;
        ngx_http_wait_request_handler(rev);
        return;
    }
    ngx_log_error(NGX_LOG_INFO, c->log, 0, "client closed connection");
    ngx_http_close_connection(c);
}

static void
ngx_http_ssl_handshake_handler(ngx_connection_t *c)
{
    if (c->ssl->handshaked) {
        c->ssl->no_wait_shutdown = 1;
        c->log->action           = "waiting for request";
        c->read->handler         = ngx_http_wait_request_handler;
        c->write->handler        = ngx_http_empty_handler; /* STUB: epoll edge */ 
        ngx_reusable_connection(c, 1);
        ngx_http_wait_request_handler(c->read);
        return;
    }

    if (c->read->timedout)
        ngx_log_error(NGX_LOG_INFO, c->log, NGX_ETIMEDOUT, "client timed out");
    ngx_http_close_connection(c);
}

int ngx_ssl_handshake(ngx_connection_t *c)
{
    int n, sslerr, err, rc;

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

    err = (sslerr == SSL_ERROR_SYSCALL) ? ngx_errno : 0;
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

static void ngx_ssl_handshake_handler(ngx_event_t *ev)
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

int ngx_ssl_recv(ngx_connection_t *c, u_char *buf, size_t size)
{
    int  n, bytes;

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
    /* SSL_read() may return data in parts, so try to read until SSL_read() would return no data */
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
                        c->read->ready = 0;
                        if (c->read->posted)
                            ngx_delete_posted_event(c->read);
                        ngx_post_event(c->read, &ngx_posted_next_events);
                    }
                    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_read: avail:%d", c->read->available);
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
            c->read->eof   = 1;
            return 0;
        case NGX_ERROR:
            c->read->error = 1;
        case NGX_AGAIN:
            return c->ssl->last;
        }
    }
}

int ngx_ssl_handle_recv(ngx_connection_t *c, int n)
{
    int sslerr, err;

    if (n > 0) {
        if (c->ssl->saved_write_handler) {
            c->write->handler = c->ssl->saved_write_handler;
            c->ssl->saved_write_handler = NULL;
            c->write->ready   = 1;
            ngx_handle_write_event(c->write, 0);
            ngx_post_event        (c->write, &ngx_posted_events);
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
            ngx_post_event        (c->write, &ngx_posted_events);
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
ngx_ssl_read_handler(ngx_event_t *rev)
{
    ngx_connection_t  *c;

    c = rev->data;
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL read handler");
    c->write->handler(c->write);
}

void ngx_ssl_write_handler(ngx_event_t *wev)
{
    ngx_connection_t  *c;

    c = wev->data;
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL write handler");
    c->read->handler(c->read);
}

int ngx_ssl_write(ngx_connection_t *c, u_char *data, size_t size)
{
    int n, sslerr, err;

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
            c->read->handler = c->ssl->saved_read_handler;
            c->ssl->saved_read_handler = NULL;
            c->read->ready = 1;
            ngx_handle_read_event(c->read, 0);
            ngx_post_event(c->read, &ngx_posted_events);
        }

        c->write->ready = 0;
        return NGX_AGAIN;
    }

    if (sslerr == SSL_ERROR_WANT_READ) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL_write: want read");
        c->read->ready = 0;
        ngx_handle_read_event(c->read, 0);
        /*
         * we do not set the timer because there is already the write event timer
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

static int ngx_epoll_add_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
    int                  op;
    uint32_t             events, prev;
    ngx_event_t         *e;
    ngx_connection_t    *c;
    struct epoll_event   ee;

    c      = ev->data;
    events = (uint32_t) event;
    if (event == NGX_READ_EVENT) {
        e      = c->write;
        prev   = EPOLLOUT;
        events = EPOLLIN|EPOLLRDHUP;
    } else {
        e      = c->read;
        prev   = EPOLLIN|EPOLLRDHUP;
        events = EPOLLOUT;
    }

    if (e->active) {
        op = EPOLL_CTL_MOD;
        events |= prev;
    } else {
        op = EPOLL_CTL_ADD;
    }

    if (flags & NGX_EXCLUSIVE_EVENT)
        events &= ~EPOLLRDHUP;

    ee.events   = events | (uint32_t) flags;
    ee.data.ptr = (void *) ((uintptr_t) c | ev->instance);
    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0, "epoll add event: fd:%d op:%d ev:%08XD", c->fd, op, ee.events);
    epoll_ctl(ep, op, c->fd, &ee);
    ev->active = 1;
    return NGX_OK;
}


static ngx_int_t
ngx_epoll_add_connection(ngx_connection_t *c)
{
    struct epoll_event  ee;

    ee.events   = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLRDHUP;
    ee.data.ptr = (void *) ((uintptr_t) c | c->read->instance);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0, "epoll add connection: fd:%d ev:%08XD", c->fd, ee.events);
    epoll_ctl(ep, EPOLL_CTL_ADD, c->fd, &ee);
    c->read->active  = 1;
    c->write->active = 1;
    return NGX_OK;
}




/* ****************************************************
 *
 *          HA-PROXY
 *
 *
 *****************************************************/

/* Methods to implement OpenSSL BIO */
static int ha_ssl_write(BIO *h, const char *buf, int num)
{
	struct buffer tmpbuf;
	struct ssl_sock_ctx *ctx;
	uint flags;
	int ret;

	ctx = BIO_get_data(h);
	tmpbuf.size = num;
	tmpbuf.area = (void *)(uintptr_t)buf;
	tmpbuf.data = num;
	tmpbuf.head = 0;
	flags = (ctx->xprt_st & SSL_SOCK_SEND_MORE) ? CO_SFL_MSG_MORE : 0;
	ret   = ctx->xprt->snd_buf(ctx->conn, ctx->xprt_ctx, &tmpbuf, num, flags);
	if (ret == 0 && !(ctx->conn->flags & (CO_FL_ERROR | CO_FL_SOCK_WR_SH))) {
		BIO_set_retry_write(h);
		ret = -1;
	} else if (ret == 0)
		 BIO_clear_retry_flags(h);
	return ret;
}

static int ha_ssl_read(BIO *h, char *buf, int size)
{
	struct buffer tmpbuf;
	struct ssl_sock_ctx *ctx;
	int ret;

	ctx         = BIO_get_data(h);
	tmpbuf.size = size;
	tmpbuf.area = buf;
	tmpbuf.data = 0;
	tmpbuf.head = 0;
	ret         = ctx->xprt->rcv_buf(ctx->conn, ctx->xprt_ctx, &tmpbuf, size, 0);
	if (ret == 0 && !(ctx->conn->flags & (CO_FL_ERROR | CO_FL_SOCK_RD_SH))) {
		BIO_set_retry_read(h);
		ret = -1;
	} else if (ret == 0)
		BIO_clear_retry_flags(h);
	return ret;
}

