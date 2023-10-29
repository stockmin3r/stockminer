/* The abstract base class of all handles. */
struct uv_handle_s {
  UV_HANDLE_FIELDS
};

/*
 * uv_stream_t is a   subclass of uv_handle_t (uv_handle_s)
 * uv_stream   is an  abstract class.
 * uv_stream_t is the parent   class of uv_tcp_t, uv_pipe_t and uv_tty_t.
 */
struct uv_stream_s {
  UV_HANDLE_FIELDS
  UV_STREAM_FIELDS
};

/* Linux uv_stream_s */
struct uv_stream_s {
	/* UV_HANDLE_FIELDS */
	void              *data;
	uv_loop_t         *loop;
	uv_handle_type     type;
	uv_close_cb        close_cb;
	struct uv__queue   handle_queue;
	union {
		int            fd;
		void          *reserved[4];
	} u;
	/* UV_LOOP_PRIVATE_FIELDS */
	unsigned long      flags;
	int                backend_fd;
	struct uv__queue   pending_queue;
	struct uv__queue   watcher_queue;
	uv__io_t         **watchers;
	unsigned int       nwatchers;
	unsigned int       nfds;
	struct uv__queue   wq;
	uv_mutex_t         wq_mutex;
	uv_async_t         wq_async;
	uv_rwlock_t        cloexec_lock;
	uv_handle_t       *closing_handles;
	struct uv__queue   process_handles;
	struct uv__queue   prepare_handles;
	struct uv__queue   check_handles;
	struct uv__queue   idle_handles;
	struct uv__queue   async_handles;
	void             (*async_unused)(void);
	uv__io_t           async_io_watcher;
	int                async_wfd;
	struct {
		void          *min;
		unsigned int   nelts;
	} timer_heap;
	uint64_t           timer_counter;
	uint64_t           time;
	int                signal_pipefd[2];
	uv__io_t           signal_io_watcher;
	uv_signal_t        child_watcher;
	int                emfile_fd;
	/* UV_PLATFORM_LOOP_FIELDS */
	uv__io_t           inotify_read_watcher;
	void              *inotify_watchers;
 	int                inotify_fd;
	/* UV_STREAM_FIELDS */
	size_t             write_queue_size; /* number of bytes queued for writing */
	uv_alloc_cb        alloc_cb;
	uv_read_cb         read_cb;
	/* UV_STREAM_PRIVATE_FIELDS */
	uv_connect_t      *connect_req;
	uv_shutdown_t     *shutdown_req;
	uv__io_t           io_watcher;
	struct uv__queue   write_queue;
	struct uv__queue   write_completed_queue;
	uv_connection_cb   connection_cb;
	int                delayed_error;
	int                accepted_fd;
	void              *queued_fds;
};

/* uv_tcp_s is a subclass of uv_stream_s */
struct uv_tcp_s {
  UV_HANDLE_FIELDS
  UV_STREAM_FIELDS
  UV_TCP_PRIVATE_FIELDS
};


/* uv_connect_t is a subclass of uv_req_t. */
struct uv_connect_s {
	UV_REQ_FIELDS
	uv_connect_cb  cb;
	uv_stream_t   *handle;
	UV_CONNECT_PRIVATE_FIELDS
};


struct uv_async_s {
  UV_HANDLE_FIELDS
  UV_ASYNC_PRIVATE_FIELDS
};

struct uv_async_t {
  /* UV_HANDLE_FIELDS */
  uv_async_cb      async_cb;
  struct uv__queue queue;
  int              pending;
};
