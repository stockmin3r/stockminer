struct ngx_event_s {
    void                 *data;
    unsigned              write:1;
    unsigned              accept:1;
    /* used to detect the stale events in kqueue and epoll */
    unsigned              instance:1;
    /* the event was passed or would be passed to a kernel in aio mode - operation was posted. */
    unsigned              active:1;
    unsigned              disabled:1;
    /* the ready event; in aio mode 0 means that no operation can be posted */
    unsigned              ready:1;
    unsigned              oneshot:1;
    /* aio operation is complete */
    unsigned              complete:1;
    unsigned              eof:1;
    unsigned              error:1;
    unsigned              timedout:1;
    unsigned              timer_set:1;
    unsigned              delayed:1;
    unsigned              deferred_accept:1;
    /* the pending eof reported by kqueue, epoll or in aio chain operation */
    unsigned              pending_eof:1;
    unsigned              posted:1;
    unsigned              closed:1;
    /* to test on worker exit */
    unsigned              channel:1;
    unsigned              resolver:1;
    unsigned              cancelable:1;
    unsigned              kq_vnode:1;
    int                   kq_errno;
    int                   available;
    ngx_event_handler_pt  handler;
    ngx_uint_t            index;
    ngx_log_t            *log;
    ngx_rbtree_node_t     timer;
    ngx_queue_t           queue; /* The posted queue */
};

struct ngx_connection_s {
    void               *data;
    ngx_event_t        *read;
    ngx_event_t        *write;
    ngx_socket_t        fd;
    ngx_recv_pt         recv;
    ngx_send_pt         send;
    ngx_recv_chain_pt   recv_chain;
    ngx_send_chain_pt   send_chain;
    ngx_listening_t    *listening;
    off_t               sent;
    ngx_log_t          *log;
    ngx_pool_t         *pool;
    int                 type;
    struct sockaddr    *sockaddr;
    socklen_t           socklen;
    ngx_str_t           addr_text;
    ngx_proxy_protocol_t *proxy_protocol;
    ngx_ssl_connection_t *ssl;
    ngx_udp_connection_t *udp;
    struct sockaddr    *local_sockaddr;
    socklen_t           local_socklen;
    ngx_buf_t          *buffer;
    ngx_queue_t         queue;
    ngx_atomic_uint_t   number;
    ngx_msec_t          start_time;
    ngx_uint_t          requests;
    unsigned            buffered:8;
    unsigned            log_error:3;     /* ngx_connection_log_error_e */
    unsigned            timedout:1;
    unsigned            error:1;
    unsigned            destroyed:1;
    unsigned            idle:1;
    unsigned            reusable:1;
    unsigned            close:1;
    unsigned            shared:1;
    unsigned            sendfile:1;
    unsigned            sndlowat:1;
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */
    unsigned            need_last_buf:1;
    unsigned            need_flush_buf:1;
    unsigned            busy_count:2;
    ngx_thread_task_t  *sendfile_task;
};

struct ngx_listening_s {
    ngx_socket_t        fd;
    struct sockaddr    *sockaddr;
    socklen_t           socklen;         /* size of sockaddr */
    size_t              addr_text_max_len;
    ngx_str_t           addr_text;
    int                 type;
    int                 backlog;
    int                 rcvbuf;
    int                 sndbuf;
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
    ngx_connection_handler_pt   handler; /* handler of accepted connection */
    void               *servers;         /* array of ngx_http_in_addr_t, for example */
    ngx_log_t           log;
    ngx_log_t          *logp;
    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    ngx_listening_t    *previous;
    ngx_connection_t   *connection;
    ngx_rbtree_t        rbtree;
    ngx_rbtree_node_t   sentinel;
    ngx_uint_t          worker;
    unsigned            open:1;
    unsigned            remain:1;
    unsigned            ignore:1;
    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;
    unsigned            wildcard:1;
    unsigned            ipv6only:1;
    unsigned            reuseport:1;
    unsigned            add_reuseport:1;
    unsigned            keepalive:2;
    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
    char               *accept_filter;
    int                 setfib;
    int                 fastopen;
};

struct ngx_listening_s {
    ngx_socket_t        fd;
    struct sockaddr    *sockaddr;
    socklen_t           socklen;         /* size of sockaddr */
    size_t              addr_text_max_len;
    ngx_str_t           addr_text;
    int                 type;
    int                 backlog;
    int                 rcvbuf;
    int                 sndbuf;
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
    ngx_connection_handler_pt   handler; /* handler of accepted connection */
    void               *servers;         /* array of ngx_http_in_addr_t, for example */
    ngx_log_t           log;
    ngx_log_t          *logp;
    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    ngx_listening_t    *previous;
    ngx_connection_t   *connection;
    ngx_rbtree_t        rbtree;
    ngx_rbtree_node_t   sentinel;
    ngx_uint_t          worker;
    unsigned            open:1;
    unsigned            remain:1;
    unsigned            ignore:1;
    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;
    unsigned            wildcard:1;
    unsigned            ipv6only:1;
    unsigned            reuseport:1;
    unsigned            add_reuseport:1;
    unsigned            keepalive:2;
    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
    char               *accept_filter;
    int                 setfib;
    int                 fastopen;
};


struct ngx_ssl_connection_s {
    SSL                        *connection;
    SSL_CTX                    *session_ctx;
    ngx_int_t                   last;
    ngx_buf_t                  *buf;
    size_t                      buffer_size;
    ngx_connection_handler_pt   handler;
    ngx_ssl_session_t          *session;
    ngx_connection_handler_pt   save_session;
    ngx_event_handler_pt        saved_read_handler;
    ngx_event_handler_pt        saved_write_handler;
    ngx_ssl_ocsp_t             *ocsp;
    u_char                      early_buf;
    unsigned                    handshaked:1;
    unsigned                    handshake_rejected:1;
    unsigned                    renegotiation:1;
    unsigned                    buffer:1;
    unsigned                    sendfile:1;
    unsigned                    no_wait_shutdown:1;
    unsigned                    no_send_shutdown:1;
    unsigned                    shutdown_without_free:1;
    unsigned                    handshake_buffer_set:1;
    unsigned                    try_early_data:1;
    unsigned                    in_early:1;
    unsigned                    in_ocsp:1;
    unsigned                    early_preread:1;
    unsigned                    write_blocked:1;
}

typedef struct {
    ngx_int_t  (*add)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
    ngx_int_t  (*del)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
    ngx_int_t  (*enable)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
    ngx_int_t  (*disable)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
    ngx_int_t  (*add_conn)(ngx_connection_t *c);
    ngx_int_t  (*del_conn)(ngx_connection_t *c, ngx_uint_t flags);
    ngx_int_t  (*notify)(ngx_event_handler_pt handler);
    ngx_int_t  (*process_events)(ngx_cycle_t *cycle, ngx_msec_t timer, ngx_uint_t flags);
    ngx_int_t  (*init)(ngx_cycle_t *cycle, ngx_msec_t timer);
    void       (*done)(ngx_cycle_t *cycle);
} ngx_event_actions_t;

struct ngx_ssl_sess_id_s {
    ngx_rbtree_node_t           node;
    u_char                     *id;
    size_t                      len;
    u_char                     *session;
    ngx_queue_t                 queue;
    time_t                      expire;
    void                       *stub;
    u_char                      sess_id[32];
};

typedef struct {
    ngx_rbtree_t                session_rbtree;
    ngx_rbtree_node_t           sentinel;
    ngx_queue_t                 expire_queue;
} ngx_ssl_session_cache_t;

/*
 * for (;;)
 *		ngx_process_events_and_timers(cycle);
 *			- ngx_process_events(cycle, timer, flags) {ngx_event.c} -> ngx_epoll_process_events()
 *          - ngx_event_process_posted()
 *
 */
