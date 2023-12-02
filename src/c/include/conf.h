#ifndef __CONF_H
#define __CONF_H

#include <stdinc.h>
#include <cdefs.h>
#include <list.h>
#include <apc.h>
#include <http.h>
#include <watchlist.h>
#include <watchtable.h>
#include <workspace.h>
#include <profile.h>
#include <jobs.h>
#include <stocks/stocks.h>

#define LOCALHOST       0x0100007f
#define ADMIN_PORT            7778
#define COOKIE_SIZE             15
#define MAX_USERS            10000
#define MAX_EMAIL_SIZE          63
#define MAX_USERNAME_SIZE       63

#define MODULE_SESSION_INIT      1
#define MODULE_SESSION_ALLOC     2
#define MODULE_MAIN_PRE_LOOP     3
#define MODULE_MAIN_POST_LOOP    4
#define MODULE_STOCKS_ENABLED    (1<<0)
#define MODULE_QUADVERSE_ENABLED (1<<1)
#define MAX_MODULES             10
#define MAX_WEBSOCKET_ACTIONS   64

#define TZOFF_NL            -14400   /* 4 Hours  */
#define TZOFF_AU            -54000   /* 15 Hours */
#define TRADE_MINUTES          960
#define DATE_YMD                 1
#define DATE_MDY                 2

#define CONF_TYPE_INT            1
#define CONF_TYPE_STR            2
#define CONF_TYPE_BOOL           3

#define USERNAME_EXISTS          "err 0 fail 1"
#define ILLEGAL_USERNAME         "err 0 fail 2"
#define SYSTEM_ERROR             "err 0 fail 3"

#define GIT_DB_PATH              "db"
#define GIT_DB_USERS_PATH        "db/users.db"
#define GIT_DB_REPO_PATH         "db/repo.db"
#define GIT_STOCKS_PATH          "data/stocks/STOCKS.TXT"
#define GIT_STOCKS_DAYS_PATH     "data/stocks/DAYS.TXT"
#define GIT_STOCKS_WEEKS_PATH    "data/stocks/WEEKS.TXT"
#define GIT_STOCKDB_PATH         "data/stocks/stockdb"
#define GIT_STOCKDB_CSV_PATH     "data/stocks/stockdb/csv"
#define GIT_GSPC_PATH            "data/stocks/stockdb/csv/^GSPC.csv"
#define GIT_OPTIONS_PATH         "data/stocks/stockdb/options"

#define LINUX_DB_PATH            "/usr/share/stockminer/db"
#define LINUX_DB_USERS_PATH      "/usr/share/stockminer/db/users.db"
#define LINUX_DB_REPO_PATH       "/usr/share/stockminer/db/repo.db"
#define LINUX_STOCKS_PATH        "/usr/share/stockminer/data/stocks/STOCKS.TXT"
#define LINUX_STOCKS_DAYS_PATH   "/usr/share/stockminer/data/stocks/DAYS.TXT"
#define LINUX_STOCKS_WEEKS_PATH  "/usr/share/stockminer/data/stocks/WEEKS.TXT"
#define LINUX_STOCKDB_PATH       "/usr/share/stockminer/data/stocks/stockdb"
#define LINUX_STOCKDB_CSV_PATH   "/usr/share/stockminer/data/stockss/stockdb/csv"
#define LINUX_GSPC_PATH          "/usr/share/stockminer/data/stocks/stockdb/csv/^GSPC.csv"
#define LINUX_OPTIONS_PATH       "/usr/share/stockminer/data/stocks/stockdb/options"
#define UNIX_TIMESTAMP_2016      1451642400
#define UNIX_TIMESTAMP_1990       631198800

enum { US, WW };

/* XXX: move to Server-> */

/* Options */
extern char  **LOWCAP_OPTIONS;
extern char  **HIGHCAP_OPTIONS;

extern int     verbose;

/* Time */
extern int     holiday;
extern time_t  current_timestamp;
extern time_t  current_minute;
extern time_t  QDATESTAMP[3];
extern char   *QDATE[3];
extern int     do_resume;

struct stock;
struct candle;
struct workspace;
struct board;

/*
 * HTML/JS/CSS file/"page"
 */
struct page {
	char           *html;      /* All the .html files under a mainpage||backpage */
	char           *js;        /* All the .js   files under a mainpage||backpage */
	char           *css;       /* All the .css  files under a mainpage||backpage */
	char           *json;      /* Website.json */
	char           *file;      /* the final (gzipped) html+js+css mainpage|backpage + HTTP response */
	char           *filename;  /* website  name/directory  of  a  mainpage|backpage */
	uint32_t        filesize;  /* Content-Length + HTTP Header of this page */
	uint32_t        curpage;   /* Which mainpage is the server currently serving? */
	uint64_t        etag;      /* etag of mainpage */
};

/*
 * a Website may contain multiple mainpages and/or multiple backpages
 */
struct website {
	struct page **mainpage;        /* src/websites/mainpage/                  */
	struct page **backpage;        /* src/websites/backpage/                  */
	int           nr_mainpages;    /* number of mainpages (no login/freepage) */
	int           nr_backpages;    /* number of backpages (requires login)    */
	int           curpage;         /* Currently enabled mainpage              */
};

/*
 * hooks for significant "blocks" of the website ("modules")
 * currently there are 2 modules: stocks and quadverse
 */
struct module {
	const char  *name;
	void       (*module_init)         (void *args);
	void       (*module_fini)         (struct module  *module);
	void       (*session_init_hook)   (struct session *session);
	void       (*session_alloc_hook)  (struct session *session);
	void       (*main_pre_loop_hook)  (struct server  *server);
	void       (*main_post_loop_hook) (struct server  *server);
	bool         enabled;
};

typedef void *(*task_handler_t)(void *args);

struct task {
	char          *name;
	int            type;
	int            priority;     // once all task nr_miners reach their nr_deal, pick task based on pririty
	task_handler_t handler;      // task handler
	int            start_day;    // run task every day from this day
	int            end_day;      // run task until and including this day
	int            start_hour;   // run the task starting at this hour (UTC)
	int            start_min;    // run the task starting at hour:min
	int            nr_preferred; // minimum number of miners preferred for this task
	int            nr_miners;    // number of servers/desktops/devices currently executing this task
};

struct miner {
	struct task       **tasks;
	struct connection  *connection;
	int                 nr_tasks;
	int                 work;
};

#define MAXEVENTS                       512
#define MAX_PACKET_SIZE               64 KB

#define NETWORK_STATE_CONNECTING          0
#define NETWORK_STATE_ACCEPTING           1
#define NETWORK_STATE_CONNECTED           2
#define NETWORK_STATE_TLS                 4
#define NETWORK_STATE_TLS_HANDSHAKE_DONE  8
#define NETWORK_STATE_WEBSOCKET          16

#define TLS_SERVER    1
#define TLS_CLIENT    2
#define TLS_WEBSOCKET 4

#define URL_ROUTE_ROOT           0  /* / root mainpage */
#define URL_ROUTE_STOCKS         1  /* /stocks/ticker  - loads the stockpage for a ticker */
#define URL_ROUTE_OPTIONS        2  /* /options/ticker - loads the options QuadVerse */
#define URL_ROUTE_USER           3  /* a User route eg: /bob (bob's squeak feed) or /bob/crypto (bob's crypto QuadVerse) */
#define MAX_URL_SEGMENTS         2  /* maximum URL segments */

struct url {
	char                *segment[MAX_URL_SEGMENTS];
	int                  nr_segments;
	int                  action;
	int                  route;
};

struct event {
	event_fd_t           event_fd;
};

typedef int   (*io_callback_t) (struct connection *connection);
typedef void  (*db_callback_t) (struct connection *connection, int result);
typedef void  (*ssl_callback_t)(void *args);
typedef void *(*www_callback_t)(void *args);

struct connection        {
	io_callback_t        recv;
	io_callback_t        send;
	struct session      *session;         /* points to the session of this connection */
	char                *packet;          /* buffer for sending responses */
	packet_size_t        packet_size;     /* size of response packet */
	packet_size_t        packet_size_max; /* current maximum size of the packet, can be expanded */
	uint8_t              nonce[32];       /* libhydrogen nonce size (32 bytes) */
	SSL                 *ssl;
	SSL_CTX             *ctx;
	int                  fd;
	struct event         event;
	int                  websocket_id;
	int                  state;
	int                  protocol;
	unsigned int         events;
	int                  has_cookie;
};

struct user {
	struct session      *session;
	char                 cookies[MAX_WEBSOCKETS][COOKIE_SIZE];
	char                 uname[MAX_USERNAME_SIZE+1];
	char                 pwhash[crypto_pwhash_STRBYTES];
	char                 pubkey[44];
	unsigned int         logged_in;
	uid_t                uid;
	uint64_t             config;
	time_t               last_login;
	time_t               join_date;
	char                 email[64];
	char                 location[48];
	char                 realname[64];
	char                 url[96];
	char                 desc[256];
	char                 img_url[16];
	int                  nr_followers;
	int                  nr_squeaks;
	int                  max_squeaks;
	unsigned short       nr_following;
	unsigned short       nr_friends;
} __attribute__((packed));

struct session {
	struct list_head     list;
	struct user         *user;
	struct profile      *profile;
	struct squeak      **sqdb;
	struct quadverse    *quadverse[MAX_QUADVERSES];
	struct watchlist    *watchlists[MAX_WATCHLISTS+1];
	struct wtab         *watchtable_presets[MAX_WTAB_PRESETS];
	struct preset       *presets[MAX_WTAB_PRESETS];
	struct qpage       **qpages;
	struct object       *objects;
	yyjson_mut_doc      *qcache;
	struct chart        *main_chart;
	struct stock        *crypto_stock;
	struct opstock      *option_contract;
	struct watchlist    *morphtab;
	struct watchlist    *ufo_watchlist;
	struct style        *styles;
	struct object       *avatar;
	struct connection   *websockets[MAX_WEBSOCKETS];
	time_t               timestamp;
	mutex_t              session_lock;
	mutex_t              watchlist_lock;
	int                  ufo_tables;
	unsigned char        current_quadverse[MAX_WEBSOCKETS];
	unsigned char        page;
	unsigned char        nr_presets;
	unsigned char        nr_watchlists;
	unsigned char        nr_watchtable_presets;
	unsigned char        nr_quadverses;
	unsigned char        nr_websockets;
	unsigned char        nr_qpages;
	unsigned char        nr_table_css;
	UT_hash_handle       hh;
};

struct uhash {
	struct session      *session;
	char                 username[MAX_USERNAME_SIZE+1];
	UT_hash_handle       hh;
};

struct chash {
	struct session      *session;
	char                *cookie;
	UT_hash_handle       hh;
};

struct ihash {
	struct session      *session;
	unsigned int         uid;
	UT_hash_handle       hh;
};

struct rhash {
	struct request      *request;
	char                *URL;
	UT_hash_handle       hh;
};

struct column_hash {
	int                  column_id;
	struct column       *column;
	UT_hash_handle       hh;
};

struct ssl_server {
	SSL_CTX             *ssl_ctx;
	char                *ssl_cert;
	char                *ssl_key;
};

typedef struct ssl_server ssl_server_t;

struct config {
	char                *key;
	char                *val_string_t;
	uint64_t             val_uint64_t;
	char               **argv;
	int                  argc;
	int                  valtype;
	UT_hash_handle       hh;
};

struct server {
	struct XLS          *XLS;
	struct watchlist    *Watchlists[256];
	char                *css_themes[256];
	char                 domain[256];
	mutex_t              watchlist_lock;
	mutex_t              stock_lock;
	unsigned short       stock_boot;
	unsigned short       current_day;
	/* OS */
	char                *python_path;
	int                  nr_vcpu;
	bool                 async;
	bool                 production;
	bool                 daemon;
	uint64_t               http_port;
	port_t               https_port;
	/* Globals */
	int                  nr_global_watchlists;
	int                  nr_public_css_themes;
	int                  nr_working_stocks;
	int                  nr_failed_stocks;
	int                  modules_enabled;
	int                  candles_enabled;
	int                  UTCOFF;
	int                  TIMEZONE;
	int                  TIMEZONE_HOURS;
	int                  DEBUG_MODE;
	int                  DEBUG_THREAD;
	char                *DEBUG_STOCK;
	struct config       *CONFIG_HASHTABLE;
	char                *modules[MAX_MODULES];
	/* Data source IP Addresses */
	unsigned int         WSJ_ADDR;
	unsigned int         YAHOO_ADDR;
	unsigned int         CBOE_ADDR;
};

/* Websocket Frame */
struct frame {
	unsigned char       *data;
	unsigned int         data_length;
	unsigned char        mask[4];
};

struct object {
	char                *blob;
	char                 key[16];
	int                  uid;
	int                  status;
	int64_t              blobsize;
	UT_hash_handle       hh;
};

struct colmap {
	unsigned short       min;
	unsigned short       max;
};

struct row {
	char               **cells;
};

/* HTML Table */
struct table {
	struct table       **table;
	char                *thead;
	char                *tbody;
	char                *caption;
	char                *Class;
	char                *id;
	char                *html;
	char               **columns;
	struct colmap       *colmap;
	struct row          *rows;
	unsigned short       nr_columns;
	unsigned short       nr_rows;
	unsigned int         table_size;
};

/*
 * All the "resources" cached at a URL, such as:
 *  - tables
 *  - pdfs, other objects, eventually
 */
struct www {
	char                *URL;
	struct table       **tables;
	unsigned short       nr_tables;
	UT_hash_handle       hh;
};

/* Ancient struct to be replaced by a struct watchtable */
struct wtab {
	char                 dict[MAX_WTAB_SIZE];
	char                 name[64];
	unsigned short       colmap[MAX_WTAB_COLUMNS];
	unsigned int         table_type;
	unsigned short       col_type;
	unsigned char        nr_columns;
	unsigned char        id;
};

struct style {
	char               **table_css;
	char               **css_name;
	uint64_t             nr_table_css;
};

#define ARGS_TYPE_ARGV 1 // arguments separated by a space (will be used by cstring_split() to parse into a char *argv[] array for the RPC handler)
#define ARGS_TYPE_JSON 2 // arguments in JSON format (will be used by yyjson to parse)

struct rpc {
	struct session      *session;
	struct connection   *connection;
	char                *packet;
	char               **argv;
	int                  argc;
	bool                 internal;
	packet_size_t        packet_size;
	struct session     **free_session; // preauth session pointer: for rpc_user_login() 
};

typedef void (*rpc_func)(struct rpc *rpc);

struct request {
	char                *URL;
	rpc_func             rpc_handler;
	int                  argc_min;
	int                  argc_max;
	int                  argtype;
	int                  internal;    // some rpc's are called internally: eg rpc_chart()
};

/* RPC */
bool              rpc_boot                (struct rpc *rpc); // workspace.c
void              rpc_chart               (struct rpc *rpc); // workspace.c
void              rpc_mini_charts         (struct rpc *rpc); // workspace.c
void              rpc_stockpage           (struct rpc *rpc); // workspace.c
void              rpc_quadverse_export    (struct rpc *rpc); // workspace.c
void              rpc_quadverse_update    (struct rpc *rpc); // workspace.c
void              rpc_qswitch             (struct rpc *rpc); // workspace.c
void              rpc_session_finish      (struct rpc *rpc); // www.c
void              rpc_wget_table          (struct rpc *rpc); // www.c
void              rpc_stockpage_earnings  (struct rpc *rpc); // earnings.c
void              rpc_stockpage_anyday    (struct rpc *rpc); // algo.c
void              rpc_send_highcaps       (struct rpc *rpc); // ufo.c
void              rpc_ufo                 (struct rpc *rpc); // ufo.c
void              rpc_ufo_megachart       (struct rpc *rpc); // ufo.c
void              rpc_stockpage_indicators(struct rpc *rpc); // ufo.c
void              rpc_stockpage_signals   (struct rpc *rpc); // monster.c
void              rpc_stock_scatter       (struct rpc *rpc); // monster.c
void              rpc_send_monster        (struct rpc *rpc); // monster.c
void              rpc_airstocks_fork      (struct rpc *rpc); // fork.c
void              rpc_airstocks_sigstat   (struct rpc *rpc); // fork.c
void              rpc_airstocks_portfolio (struct rpc *rpc); // fork.c
void              rpc_set_max_ranks       (struct rpc *rpc); // ranks.c
void              rpc_profile_follow      (struct rpc *rpc); // profile.c
void              rpc_profile_squeak      (struct rpc *rpc); // profile.c
void              rpc_profile_set         (struct rpc *rpc); // profile.c
void              rpc_profile_get         (struct rpc *rpc); // profile.c
void              rpc_profile_set_image   (struct rpc *rpc); // profile.c
void              rpc_user_chart          (struct rpc *rpc); // profile.c
void              rpc_watchlist_delstock  (struct rpc *rpc); // watchlib.c
void              rpc_watchlist_save      (struct rpc *rpc); // watchlib.c
void              rpc_watchlist_remove    (struct rpc *rpc); // watchlib.c
void              rpc_watchlist_clear     (struct rpc *rpc); // watchlist.c
void              rpc_watchlist_load      (struct rpc *rpc); // watchlist.c
void              rpc_watchlist_alert     (struct rpc *rpc); // watchlist.c
void              rpc_webscript           (struct rpc *rpc); // watchlist.c
void              rpc_watchtable_addstocks(struct rpc *rpc); // watchlist.c
void              rpc_watchtable_sort     (struct rpc *rpc); // watchtable.c
void              rpc_watchtable_load     (struct rpc *rpc); // watchtable.c
void              rpc_watchtable_bomb     (struct rpc *rpc); // watchtable.c
void              rpc_watchtable_columns  (struct rpc *rpc); // watchtable.c
void              rpc_css                 (struct rpc *rpc); // watchtable.c
void              rpc_define_table        (struct rpc *rpc); // watchtable.c
void              rpc_remove_alert        (struct rpc *rpc); // watchcond.c
void              rpc_search              (struct rpc *rpc); // stocks.c
void              rpc_indicator_save      (struct rpc *rpc); // stocks.c
void              rpc_indicator_edit      (struct rpc *rpc); // stocks.c
void              rpc_candle_stock        (struct rpc *rpc); // candles.c
void              rpc_candle_zoom         (struct rpc *rpc); // candles.c
void              rpc_csp                 (struct rpc *rpc); // candles.c
void              rpc_csr                 (struct rpc *rpc); // candles.c
void              rpc_stockpage_candle    (struct rpc *rpc); // candles.c
void              rpc_option_chart        (struct rpc *rpc); // options.c
void              rpc_option_covered_calls(struct rpc *rpc); // options.c
void              rpc_option_chain        (struct rpc *rpc); // options.c
void              rpc_option_page         (struct rpc *rpc); // options.c
void              rpc_user_register       (struct rpc *rpc); // user.c
void              rpc_user_login          (struct rpc *rpc); // user.c
void              rpc_xls                 (struct rpc *rpc); // xls.c

/* workspace.c */
__MODULE_INIT     init_quadverse_module   (void);
__MODULE_HOOK     quadverse_session_init  (struct session *session);
__MODULE_HOOK     quadverse_session_init  (struct session *session);
__MODULE_HOOK     quadverse_session_alloc (struct session *session);
int               process_workspace       (struct session *session, char *packet, struct workspace *workspace, int PID, int QSID, int QID, int WSID);
int               workspace_name          (struct session *session, char *packet, struct qpage *qpage, char *title, int JQVID, int QVID, int QSID, int QID, int WSID);
int               workspace_chart_json    (struct stock *stock, char *packet, char *div, char *cfunc, char *className);
int               qpage_packet            (struct session *session, char *packet);
int               addPoint_page           (struct session *session, char *packet);
void              anyday_workspace        (struct session *session, char *packet, char *ticker);
void              signals_workspace       (struct session *session, char *packet, char *ticker);
void              algo_stock_page         (struct session *session, char *packet, char *ticker, int QSID);
void              quadverse_subscribe     (struct session *session, struct connection *connection, struct qpage *qpage, int QVID);
void              workspace_broadcast     (struct session *session, struct connection *connection, char *packet, int packet_len);

void              update_page             (struct session *session, struct connection *connection, char *packet);
void              stock_indicators        (struct session *session, struct connection *connection, char *packet, char *table);
struct workspace *get_workspace           (struct session *session, struct wsid *wsid, int *create_workspace);
int               workspace_id            (struct session *session, char *id, struct wsid *wsid);
int               remove_chart            (struct session *session, char *ticker, int QVID, int QSID, int QID, int WSID);
struct chart     *search_chart            (struct session *session, char *ticker, int QVID, int QSID, int QID, int WSID);
void              qcache_remove_object    (struct session *session, struct qpage *qpage, char *obj_type, char *obj_name, int QVID, int QSID, int QID, int WSID);
void	          quadverse_unsubscribe   (struct session *session);
int               pack_workspace          (struct stock *stock, char *packet, char *div, struct wsid *wsid, char *cfunc);
int               pack_stockpage          (struct stock *stock, char *packet, char *div, struct wsid *wsid, char *cfunc);
void              qpage_send_subscribers  (struct qpage *qpage, char *packet, int packet_len);
int               pack_mini               (struct stock *stock, char *packet, struct chart *chart, struct wsid *wsid, int limit, char *moveTo, int height, int interval);
void              workspace_set_watchtable(struct session *session, struct watchlist *watchtable, char *QDIV);
void              load_quadverse_pages    (void);

/* session.c */
struct session   *session_alloc           (struct connection *connection);
struct session   *session_get             (struct connection *connection, char *request);
void              session_set_config      (struct connection *connection);
struct session   *session_by_cookie       (char *cookie);
struct session   *session_by_username     (char *username);
struct session   *session_by_uid          (unsigned int uid);
struct list_head *get_session_list        (void);
void              apc_sessions            (struct connection *connection, char **argv);
void              session_add             (struct session *session);
void              session_destroy         (struct session *session);
void              session_load_quadverse  (struct session *);
void              sessions_update_XLS     (struct XLS *XLS);
void              sessions_checkpoint     (void);
void              cmd_session_list        (int http_fd);
void              session_upload          (struct session *session, struct connection *connection, int type, char *args);
void              SESSION_LOCK            (void);
void              SESSION_UNLOCK          (void);

/* module.c */
void              init_modules            (struct server *config);
void              module_session_hook     (struct session *session, int module_hook);
void              register_module         (struct module *module);
void              module_hook             (void *args, int hook);

/* db.c */
void              init_db                 (struct server *server);
void             *db_thread               (void *args);
void              db_user_add             (struct user *user, struct connection *connection);
void              db_user_update_cb       (struct user *user);
void              db_user_update          (struct user *user);
void              db_user_list            (void);
void             *db_submit_tasks         (void *args);

/* fdb.c */
void              fdb_user_list           (void);
uid_t             fdb_user_uid            (char *username);

/* price.c */
void              apc_update_EOD          (struct connection *connection, char **argv);
void              wsj_update_stock        (char *ticker);
void              wsj_get_EOD             (struct stock *stock, char *page);
int               wsj_query               (struct stock *stock, char *url, int url_size, char *page, void (*query)(struct stock *stock, char *page));
void              update_current_price    (struct stock *stock);
int               update_allday_price     (struct XLS *XLS, struct stock *stock);
int               load_ohlc               (struct stock *stock);
double            price_by_date           (struct stock *stock, char *date);
void              argv_wsj_allday         (char *ticker);
void              init_ip                 (void);

/* volume.c */
char             *volume                  (uint64_t vol, char *buf);

/* watchtable.c */
void              init_watchtable(void);

double stock_intraday_low (struct stock *stock);
size_t curl_get_data      (char *buf, size_t size, size_t count, void *data);

/* algo.c */
void   algorithm_mag1(struct stock *stock, struct mag *mag);
void   init_algo     (struct XLS *XLS, struct stock *stock);
void   init_ctable   (struct XLS *XLS);
void   init_anyday   (struct XLS *XLS);
void   init_BIX      (struct XLS *XLS);

/* mag.c */
void   load_mag2     (struct stock *stock);
void   load_mag3     (struct stock *stock);
void   load_mag4     (struct stock *stock);
void   build_mag     (char *ticker, struct XLS *XLS);
void   process_mag3  (struct XLS *XLS);

/* blob.c */
int  websocket_upload_object         (struct session *session, struct connection *connection, char *req);
int  remove_object                   (struct session *session, char *URL);
int  HTTP_BLOB                       (char *key, struct connection *connection);
bool WWW_GET_IMAGE                   (char *req, struct connection *connection);
void websocket_send_images           (struct connection *connection);
void blob_load_images                (char *directory);
void websocket_image                 (int websocket_fd, char *image);
void load_objects                    (struct session *session, unsigned int uid);

/* qcache.c */
yyjson_mut_doc *qcache_new_quadspace (struct session *session, struct qpage *qpage, int QVID,       int QSID, int *broadcast, char *title, int stockpage);
yyjson_mut_val *qcache_new_workspace (struct session *session, struct qpage *qpage, char *title,    int QVID, int QSID, int QID, int WSID);
yyjson_mut_doc *qcache_create        (struct session *session, int QVID, int QSID,  char *title,    int stockpage);
void qcache_replace_chart            (struct session *session, struct qpage *qpage, char *old_ticker, char *new_ticker, int QVID, int QSID, int QID, int WSID);
void qcache_remove_chart             (struct session *session, struct qpage *qpage, char *ticker,   int QVID, int QSID, int QID, int WSID);
void qcache_remove_object            (struct session *session, struct qpage *qpage, char *obj_type, char *obj_name, int QVID, int QSID, int QID, int WSID);
void qcache_remove_wstab             (struct session *session, struct qpage *qpage, char *obj_type, char *obj_name, int QVID, int QSID, int QID, int WSID);
int  qcache_chart_indicator_add      (struct session *session, struct qpage *qpage, char *ticker,   char *indicator, int QVID, int QSID, int QID, int WSID);
int  qcache_chart_indicator_remove   (struct session *session, struct qpage *qpage, char *ticker,   char *indicator, int QVID, int QSID, int QID, int WSID);
void qcache_set_position             (struct session *session, struct qpage *qpage, char *blob_url, char *blob_type, int QVID, int QSID, int QID, int WSID);
void qcache_addchart                 (struct session *session, struct qpage *qpage, char *ticker,   int QVID, int QSID, int QID, int WSID);
void qcache_add_object2              (struct session *session, struct qpage *qpage, char *KEY,      char *VAL, int QVID, int QSID, int QID, int WSID);
void qcache_add_object               (struct session *session, struct qpage *qpage, char *objname,  char *objval, int QVID, int QSID, int QID, int WSID);
void qcache_setgrid                  (struct session *session, struct qpage *qpage, char *grid,     int QVID, int QSID, int QID, int WSID);
void qcache_setname                  (struct session *session, struct qpage *qpage, char *title,    int QVID, int QSID, int QID, int WSID);
void qcache_remove_quadspace         (struct session *session, struct qpage *qpage, int QVID,       int QSID);
void qcache_remove_workspace         (struct session *session, struct qpage *qpage, int QVID,       int QSID, int QID, int WSID);
void qcache_save                     (struct session *session, struct qpage *qpage, int QVID,       int QSID);
void session_load_qcache             (struct session *session);

/* www.c */
void          www_send_backpage      (struct session *session, struct connection *connection, int send_qconf);
int           www_new_websocket      (struct session *session, struct connection *connection);
void         *init_www               (void *args);
void          www_load_website       (struct server *server);
void          apc_reload_website     (struct connection *connection, char **argv);

/* auth.c */
void          admin_client_auth      (char *command);
void          apc_server_auth        (struct connection *connection, char **argv);
void          lpc_adduser            (struct connection *connection, char **argv);

/* profile.c */
void              session_load_profile      (struct session *session);
int               packet_profile            (struct session *session, char *packet);

/* watchcond.c */
void              watchcond_update          (struct session *session, struct watchlist *watchlist, struct watchcond *watchcond);
void              watchcond_create          (struct session *session, struct watchlist *watchlist, struct watchcond *watchcond);
void              watchcond_del             (struct watchlist *watchlist, int idx);
struct watchcond *session_watchcond         (struct watchlist *watchlist, uint64_t id);
struct watchcond *get_watchcond             (struct watchlist *watchlist, struct stock *stock);

/* watchlist.c */
void               watchtable_clear         (struct watchlist *watchtable);
void               watchlist_path           (struct session   *session,   char *path);
struct watchlist  *map_watchlist            (struct session   *session,   char *wpath, char *watchlist_name, char **map, struct filemap *filemap);
void               watchlist_delstock       (struct session   *session,   struct watchlist *watchlist, char *watchlist_name, struct stock *stock);
struct watchstock *get_watchstock           (struct watchlist *watchlist, struct stock *stock);
void               list_watchlists          (int http_fd);

double             stock_price_target       (struct session   *session,   struct watchlist *watchlist, struct stock *stock);
double             get_stock_price_target   (struct session   *session,   struct watchlist *watchlist, struct stock *stock);
void               session_load_watchlists  (struct session   *session);
struct watchlist  *search_watchlist         (struct session   *session,   char *watchlist_name);
struct watchlist  *watchtable_create        (struct session   *session,   char *watchtable_name, char *watchlist_name);
struct watchlist  *watchlist_add            (struct session   *session,   struct watchlist *watchlist,  struct stock *stock);
struct watchlist  *watchtable_add           (struct session   *session,   struct watchlist *watchtable, struct stock *stock);
struct watchcond *get_watchcond             (struct watchlist *watchlist, struct stock *stock);
void               memwatch_addstock        (struct watchlist *watchlist, struct stock *stock);

/* notify.c */
void               watchlist_notifications  (struct session *session, struct connection *connection);

/* upload.c */
void               load_images              (void);
void               load_image               (int idx, char *cmd, char *img, int img_len);
int                HTTP_GET_IMAGE           (char *req, struct connection *connection);

/* os.c */
void               init_os                  (struct server *server);
void               thread_create            (void *(*thread)(void *), void *args);

/* threads.c */
void *market_update_thread  (void *args);
void *market_end            (void *args);
void  create_stock_threads  (struct XLS *XLS);
void  cmd_threadstat        (int);

/* earnings.c */
void  update_earnings       (struct XLS *XLS);
void  build_stock_earnings  (void);
int   build_earnings        (struct stock *stock);
void  get_earnings_dates    (char *ticker, struct XLS *XLS);
void  print_earnings        (char *ticker);
char *MINI_TABLE            (int *table_size, int table_id);
void  cmd_dump_earnings     (int stockfd);

/* conf.c */
void  init_config           (struct server *server);
struct config *config_get   (const char *key, int var_type, char **out_cstr, uint64_t *out_uint64);
bool config_enabled         (const char *key);
int  conf_private_alerts    (struct session *session, char *packet);
int  conf_private_watchlists(struct session *session, char *packet);
int  conf_styles            (struct session *session, char *packet);
int  conf_public_alerts     (char *packet);
int  conf_public_watchlists (char *packet);

/* ranks.c */
void init_ranks             (struct XLS *XLS);
int  date_to_rank           (struct stock *stock, char *date);

/* json.c */
void *json_thread          (void *args);
void update_highcaps       (struct board *board);
void update_lowcaps        (struct board *board);
void update_lowcaps_double (struct board *board);
void update_highcaps_double(struct board *board);
void update_highcaps_volume(struct board *board);
void update_lowcaps_volume (struct board *board);
void update_indicators     (struct board *board);

/* net.c */
void *netctl_thread(void *args);

/* gzip.c */
int      zip_compress   (char *page, char *page_gz, int gz_len);
int      zip_decompress (char *page_gz, char *page, int gz_len);
int      zip_deflate    (char *page, char *page_gz, int page_len);
int      zip_deflate2   (char *page, char *page_gz, int page_len);
uint64_t zip_decompress2(unsigned char *page_gz, unsigned char *page, int compressed_size, int uncompressed_size);

/* time.c */
int      timezone_offset    (const char *tzname);
int      weekday_offset     (const char *day);
int      splitdate_MDY      (char *date, int *year, int *month, int *day);
int      splitdate_YMD      (char *date, int *year, int *month, int *day);
char    *MDY2YMD            (char *from, char *to);
time_t   str2utc            (char *timestr);
time_t   str2unix           (char *timestr);
time_t   str2unix2          (char *timestr);
time_t   get_ny_time        (char *timestr);
time_t   get_current_time   (char *timestr);
char    *ny_date            (char *buf);
time_t   ny_time            (char *timestr);
time_t   get_timestamp      (void);
char    *unix2str           (time_t timestamp, char *timestr);
char    *unix2str2          (time_t timestamp, char *timestr);
char    *unix2str3          (time_t timestamp, char *timestr);
int      get_time           (void);
void     init_time          (struct server *server);
void     time_EOD           (void);
void     set_current_minute (void);

/* task.c */
void task_benchmark(char *args);
void task_schedule(void);
void init_tasks(void);

/* lib.c */
int64_t        fs_read                 (int, char *, int64_t);
int            fs_readline             (char *s, int fd);
int64_t        fs_readfile             (char *path, char *buf, int64_t max);
int64_t        fs_readfile_str         (char *path, char *buf, int64_t max);
int64_t        fs_write                (int fd, void *buf, int64_t count);
char          *fs_mallocfile           (char *path, int64_t *filesize);
char          *fs_mallocfile_str       (char *path, int64_t *filesize);
void           fs_writefile            (char *path, char *file, int64_t len);
void           fs_newfile              (char *path, void *file, int64_t len);
void           fs_appendfile           (char *path, void *file, int64_t filesize);
void           fs_appendfile_nl        (char *path, char *file, int64_t filesize);
int            fs_line_count           (char *filename);
void           fs_copy_file            (char *src, char *dst);
void           fs_copy_big_file        (char *src, char *dst);
void           fs_printfile            (char *path);
char         **fs_list_directory       (char *directory, int *nr_files_out);
int64_t        net_send                (int fd, void *buf, int64_t count);
int64_t        net_recv                (int sockfd, char *buf, int64_t size, int timeout);
socket_t       net_tcp_bind            (unsigned int bind_addr, unsigned short port);
int            net_tcp_connect         (const char *dst_addr,  unsigned short dst_port);
int            net_tcp_connect2        (unsigned int dst_addr, unsigned short dst_port);
int            net_tcp_connect_host    (const char *hostname, int port);
int            net_tcp_connect_timeout (unsigned int ip, int port, int seconds);
int            net_select_waitfd       (int, int);
in_addr_t      net_get_hostname        (char *hostname);
char          *cstring_inject          (char *target, char *payload, char *pattern, int *output_size);
void           cstring_excise          (char *str, char *pattern);
int            cstring_itoa            (char *s, uint64_t n);
int64_t        cstring_remove_line     (char *text, int lineno, int64_t textsize);
int64_t        cstring_count_chars     (char *str, char c);
int64_t        cstring_line_count      (char *);
int            cstring_split           (char *msg, char *argv[], int max, char c);
void           cstring_strip_char      (char *ptr, char c);
char          *memdup                  (char *mem, int64_t size);
void          *zmalloc                 (int64_t size);
unsigned short random_short            (void);
uint32_t       random_int              (void);
uint64_t       random_long             (void);
double         random_double           (void);
void           random_string_len       (char *str, int64_t len);
void           random_string           (char *str);
void           random_string_int       (char *str);
void           random_cookie           (char *cookie);
void           INSERTION_SORT          (int array[], int n);
void           DOUBLE_SORT_LOHI        (double array[], int n);
void           DOUBLE_SORT_HILO        (double array[], int n);

/* openssl.c */
void               openssl_init               (void);
struct ssl_server *openssl_server             (const char *cert, const char *key);
void               openssl_server_sync        (www_callback_t callback, in_addr_t ipaddr, port_t port, bool create_thread);
bool               openssl_connect_sync       (struct connection *connection, unsigned int ipaddr, unsigned short port);
packet_size_t      openssl_read_sync          (struct connection *connection, char *buf, packet_size_t packet_size);
packet_size_t      openssl_read_sync2         (struct connection *connection, char *buf, packet_size_t max_packet_size);
packet_size_t      openssl_read_http          (struct connection *connection, char *buf, packet_size_t max_packet_size);
packet_size_t      openssl_write_sync         (struct connection *connection, char *packet, packet_size_t packet_size);
void               openssl_destroy            (struct connection *connection);
int                connection_ssl_handshake   (struct connection *connection);
struct connection *openssl_connect_async      (void);
struct connection *openssl_accept_async       (ssl_server_t *ssl_server, int fd);
int                connection_ssl_send        (struct connection *connection);
int                connection_ssl_recv        (struct connection *connection);

/* websocket.c */
int  websocket_recv      (char *data, uint64_t data_length, struct frame *frames, char *msg, int mask);
int  websocket_send      (struct connection *connection, char *data, uint64_t data_length);
int  websocket_send2     (struct connection *connection, char *data, uint64_t data_length);
int  websocket_send4     (struct connection *connection, char *data, uint64_t data_length);
int  websocket_send_img  (struct connection *connection, char *data, uint64_t data_length);
int  websocket_send_huge (struct connection *connection, char *data, uint64_t data_length);
void websocket_send_gzip (struct connection *connection, char *data, uint64_t data_length);
bool websocket_handshake (struct connection *connection, char *data);
void websocket_sendx     (struct connection *connection, char *packet, int packet_len);
void websockets_sendall  (struct session *session, char *packet, int packet_len);
void websockets_sendall_except(struct session *session, struct connection *this_connection, char *packet, int packet_len);

/* xls.c */
char *stock_xls     (char *ticker, char *div, int *xls_len);
void xls_mag2_action(char *ticker, char *div, int action, int sockfd);
void xls_mag2       (struct connection *connection, char *table, char *div, char *ticker, int nr_entries, int via);
void HTTP_XLS       (char *req, struct connection *connection);

/* html_tables.c */
struct table **LIBXLS_wget_table     (char *URL, int *ntables);
int            LIBXLS_deftab         (char *packet, char *URL, char *QGID, char *table_names);
void           LIBXLS_rpc_excel2table(struct session *session, struct connection *connection, char *excel_file, char *excel_filepath, int excel_size, char **subargv, void *(*completion)(struct job *job));

/* user.c */
struct user *search_user        (char *username);
uid_t        uid_by_username    (char *username);
void         user_update        (struct user *user);
void         init_users         (void);
void         db_user_register_cb(struct connection *connnection, int result);

/* md5.c */
unsigned char *md5_checksum      (char *input, unsigned char *checksum, int size);
unsigned char *md5_checksum_file (char *filename, unsigned char *checksum);
char          *md5_string        (unsigned char *checksum, char *output);
int            md5_file_diff     (unsigned char *checksum, char *filename);
int            md5_compare       (unsigned char *checksum1, unsigned char *checksum2);

/* build.c */
struct website *build_website    (struct server *server);

/* unix.c/windows.c */
char *MAP_FILEMAP_RW             (struct filemap *filemap);
char *MAP_FILE_RO                (char *path, struct filemap *filemap);
char *MAP_FILE_RW                (char *path, struct filemap *filemap);
char *MAP_FILE_CRW               (char *path, struct filemap *filemap);
void  UNMAP_FILE                 (char *map,  struct filemap *filemap);
void  net_socket_block           (socket_t sockfd);
void  net_socket_nonblock        (socket_t sockfd);
void  os_exec_argv               (char *argv[]);
void  os_exec                    (char *command);
void  event_del                  (struct connection *connection);
void  event_mod                  (struct connection *connection);

/* external/base64.c */
size_t base64_encode             (unsigned char *src, size_t len, unsigned char *out);
size_t base64_decode             (unsigned char *src, size_t len, unsigned char *out);
bool   base64_validate           (unsigned char *src, size_t len);

/* apc.c */
void   admin_client_loop         (void);
void   apc_send_command          (const char *command);
void   apc_connect_command       (const char *command);
void   apc_send_result           (struct connection *connnection, char *result);
void   apc_exec                  (char *command);

/* term.c */
void terminal_loop(void);


struct curldata {
    char         *memory;
    size_t        size;
    struct stock *stock;
};

char *convert_urlencoded(char *);
char *curl_get          (char *url, char *page);
int   http_get_boundary (char *req, char *cookie, char **bptr);

#endif
