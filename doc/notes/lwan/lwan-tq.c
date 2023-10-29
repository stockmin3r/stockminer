/* lwan-tq.c */

struct timeout_queue {
    const struct lwan      *lwan;
    struct lwan_connection *conns;
    struct lwan_connection  head;
    struct timeout          timeout;
    unsigned int            current_time;
    unsigned int            move_to_last_bump;
};

static inline int timeout_queue_node_to_idx(struct timeout_queue *tq, struct lwan_connection *conn)
{
    return (conn == &tq->head) ? -1 : (int)(intptr_t)(conn - tq->conns);
}

static inline struct lwan_connection *timeout_queue_idx_to_node(struct timeout_queue *tq, int idx)
{
    return (idx < 0) ? &tq->head : &tq->conns[idx];
}

inline void timeout_queue_insert(struct timeout_queue *tq, struct lwan_connection *new_node)
{
    new_node->next = -1;
    new_node->prev = tq->head.prev;
    struct lwan_connection *prev = timeout_queue_idx_to_node(tq, tq->head.prev);
    tq->head.prev  = prev->next  = timeout_queue_node_to_idx(tq, new_node);
}

static inline void timeout_queue_remove(struct timeout_queue *tq, struct lwan_connection *node)
{
    struct lwan_connection *prev = timeout_queue_idx_to_node(tq, node->prev);
    struct lwan_connection *next = timeout_queue_idx_to_node(tq, node->next);

    next->prev = node->prev;
    prev->next = node->next;
}

inline bool timeout_queue_empty(struct timeout_queue *tq)
{
    return tq->head.next < 0;
}

inline void timeout_queue_move_to_last(struct timeout_queue *tq, struct lwan_connection *conn)
{
    /* CONN_IS_KEEP_ALIVE isn't checked here because non-keep-alive connections
     * are closed in the request processing coroutine after they have been
     * served.  In practice, if this is called, it's a keep-alive connection. */
    conn->time_to_expire = tq->current_time + tq->move_to_last_bump;

    timeout_queue_remove(tq, conn);
    timeout_queue_insert(tq, conn);
}

void timeout_queue_init(struct timeout_queue *tq, const struct lwan *lwan)
{
    *tq = (struct timeout_queue){
        .lwan         = lwan,
        .conns        = lwan->conns,
        .current_time = 0,
        .move_to_last_bump = lwan->config.keep_alive_timeout,
        .head.next    = -1,
        .head.prev    = -1,
        .timeout      = (struct timeout){},
    };
}

void timeout_queue_expire(struct timeout_queue *tq,
                          struct lwan_connection *conn)
{
    timeout_queue_remove(tq, conn);

    if (LIKELY(conn->coro)) {
        coro_free(conn->coro);
        conn->coro = NULL;
    }
    close(lwan_connection_get_fd(tq->lwan, conn));
}

void timeout_queue_expire_waiting(struct timeout_queue *tq)
{
    tq->current_time++;

    while (!timeout_queue_empty(tq)) {
        struct lwan_connection *conn = timeout_queue_idx_to_node(tq, tq->head.next);

        if (conn->time_to_expire > tq->current_time)
            return;
        timeout_queue_expire(tq, conn);
    }
    /* Timeout queue exhausted: reset epoch */
    tq->current_time = 0;
}

void timeout_queue_expire_all(struct timeout_queue *tq)
{
    while (!timeout_queue_empty(tq)) {
        struct lwan_connection *conn = timeout_queue_idx_to_node(tq, tq->head.next);
        timeout_queue_expire(tq, conn);
    }
}
