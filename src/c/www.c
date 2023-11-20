#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

#define INIT 0x74696e69

struct website    *WEBSITE;
struct ssl_server *WWW_SERVER;
char              *auth_wasm_header;
char              *stock_data_gz;
uint64_t           stock_data_size;
int                auth_wasm_size;
struct rhash      *RPC_HASHTABLE;

static __inline__ int www_reload   (struct connection *connection);
static void           www_set_route(struct connection *connection, struct url *url);
static bool           www_get_route(char *request, struct url *url);

/****************************
*struct request {
*    char      *URL;
*    rpc_func   rpc_handler;  // typedef void (*rpc_func)(struct rpc *rpc);
*    int        argc_min;
*    int        argc_max;
*    int        argtype;
*};*/
struct request rpc_requests[] = {
	/* Charts */
	{ "chart",               rpc_chart,                  1, 4, ARGS_TYPE_ARGV},
	{ "mini",                rpc_mini_charts,            1, 1, ARGS_TYPE_ARGV},
	{ "indi_preset_save",    rpc_indicator_save,         2, 2, ARGS_TYPE_ARGV},
	{ "indi_preset_edit",    rpc_indicator_edit,         2, 2, ARGS_TYPE_ARGV},

	/* QuadVerse */
	{ "qswitch",             rpc_qswitch,                3, 4, ARGS_TYPE_ARGV},
	{ "qupdate",             rpc_quadverse_update,       6, 6, ARGS_TYPE_ARGV},
	{ "qexport",             rpc_quadverse_export,       6, 6, ARGS_TYPE_ARGV},
	{ "stage",               rpc_webscript,              3, 3, ARGS_TYPE_ARGV},

	/* Stockpage */
	{ "stockpage",           rpc_stockpage,              3, 3, ARGS_TYPE_ARGV},
	{ "sp-anyday",           rpc_stockpage_anyday,       2, 2, ARGS_TYPE_ARGV},
	{ "sp-etab",             rpc_stockpage_earnings,     2, 2, ARGS_TYPE_ARGV},
	{ "sp-candle",           rpc_stockpage_candle,       3, 3, ARGS_TYPE_ARGV},
	{ "sp-indicator",        rpc_stockpage_indicators,   2, 2, ARGS_TYPE_ARGV},
	{ "sigmon",              rpc_stockpage_signals,      2, 2, ARGS_TYPE_ARGV},
	{ "search",              rpc_search,                 2, 2, ARGS_TYPE_ARGV},
	{ "ranks",               rpc_set_max_ranks,          2, 2, ARGS_TYPE_ARGV},

	/* Airstocks QuadVerse */
	{ "sigstat",             rpc_airstocks_sigstat,      1, 1, ARGS_TYPE_ARGV},
	{ "airports",            rpc_airstocks_portfolio,    1, 1, ARGS_TYPE_ARGV},
	{ "fork",                rpc_airstocks_fork,         1, 1, ARGS_TYPE_ARGV},

	/* Profile */
	{ "profile_get",         rpc_profile_get,            1, 1, ARGS_TYPE_ARGV},
	{ "profile_set",         rpc_profile_set,            0, 0, ARGS_TYPE_JSON},
	{ "profile_img",         rpc_profile_set_image,      3, 3, ARGS_TYPE_JSON},
	{ "squeak",              rpc_profile_squeak,         2, 2, ARGS_TYPE_JSON},
	{ "follow",              rpc_profile_follow,         2, 2, ARGS_TYPE_ARGV},
	{ "uchart",              rpc_user_chart,             2, 2, ARGS_TYPE_JSON},

	/* UFOS */
	{ "ufoinit",             rpc_ufo,                    3, 3, ARGS_TYPE_ARGV},
	{ "ufomega",             rpc_ufo_megachart,          2, 2, ARGS_TYPE_ARGV},

	/* Watchlists & Watchtables */
	{ "watchtable",          rpc_watchtable_columns,     5, 5, ARGS_TYPE_ARGV},
	{ "watchlist_addstocks", rpc_watchtable_addstocks,   2, 3, ARGS_TYPE_ARGV},
	{ "watchlist_save",      rpc_watchlist_save,         2, 2, ARGS_TYPE_ARGV},
	{ "watchlist_delstock",  rpc_watchlist_delstock,     2, 2, ARGS_TYPE_ARGV},
	{ "watchlist_load",      rpc_watchlist_load,         3, 3, ARGS_TYPE_ARGV},
	{ "watchlist_clear",     rpc_watchlist_clear,        2, 2, ARGS_TYPE_ARGV},
	{ "watchlist_remove",    rpc_watchlist_remove,       2, 2, ARGS_TYPE_ARGV},
	{ "watchlist_alert",     rpc_watchlist_alert,        2, 2, ARGS_TYPE_ARGV},
	{ "watchtable_sort",     rpc_watchtable_sort,        2, 2, ARGS_TYPE_ARGV},

	/* Columns */
	{ "TPLoad",              rpc_watchtable_load,        2, 2, ARGS_TYPE_ARGV},
	{ "wbomb",               rpc_watchtable_bomb,        2, 2, ARGS_TYPE_ARGV},
	{ "xls",                 rpc_xls,                    8, 8, ARGS_TYPE_ARGV},

	/* Alerts */
	{ "alerts",              rpc_watchlist_alert,        2, 2, ARGS_TYPE_ARGV},
	{ "alerts_del",          rpc_remove_alert,           2, 2, ARGS_TYPE_ARGV},

	/* Tables, Styles */
	{ "css",                 rpc_css,                    2, 2, ARGS_TYPE_JSON},
	{ "deftab",              rpc_define_table,           2, 2, ARGS_TYPE_JSON},
	{ "wget",                rpc_wget_table,             3, 3, ARGS_TYPE_ARGV},

	/* CandleSticks */
	{ "candle",              rpc_candle_stock,           2, 2, ARGS_TYPE_ARGV},
	{ "csp",                 rpc_csp,                    3, 3, ARGS_TYPE_ARGV},
	{ "csr",                 rpc_csr,                    3, 3, ARGS_TYPE_ARGV},
	{ "czoom",               rpc_candle_zoom,            3, 3, ARGS_TYPE_ARGV},

	/* Options */
	{ "oppage",              rpc_option_page,            1, 1, ARGS_TYPE_ARGV},
	{ "opcc",                rpc_option_covered_calls,   2, 2, ARGS_TYPE_ARGV},
	{ "opchart",             rpc_option_chart,           2, 2, ARGS_TYPE_ARGV},
	{ "opchain",             rpc_option_chain,           2, 2, ARGS_TYPE_ARGV},

	/* Session */
	{ "fini",                rpc_session_finish,         1, 1, ARGS_TYPE_ARGV},
	{ "login",               rpc_user_login,             3, 3, ARGS_TYPE_ARGV},
	{ "register",            rpc_user_register,          3, 3, ARGS_TYPE_ARGV}
};

int NR_RPC_REQUESTS  = sizeof(rpc_requests)/sizeof(struct request);

void www_init_requests()
{
	// Websocket RPC (Remote Procedure Calls)
	for (int x=0; x<NR_RPC_REQUESTS; x++) {
		struct request *request = &rpc_requests[x];
		struct rhash   *rhash   = (struct rhash *)zmalloc(sizeof(*rhash));
        rhash->request          = request;
		rhash->URL              = request->URL;
        HASH_ADD_STR(RPC_HASHTABLE, URL, rhash);
	}
}

static struct request *www_request_hash(char *str)
{
	struct rhash *rhash = NULL;
	char *p = strchr(str, ' ');

	if (p)
		*p = 0;

	HASH_FIND_STR(RPC_HASHTABLE, str, rhash);
	if (p)
		*p = ' ';
	if (rhash)
		return rhash->request;
	return NULL;
}

#define MAX_RPC_ARGV 16

static bool
websocket_process_request(struct connection *connection)
{
	struct frame   *frame;
	struct frame    frames[6] = {0};
	struct rpc      rpc;
	struct request *request;
	char           *msg;
	char           *argv[MAX_RPC_ARGV];
	char            msgbuf[2048];
	int             nr_frames, argc, argc_max;

	connection->packet_size = openssl_read_sync2(connection, connection->packet, sizeof(msgbuf)-10);
	if (connection->packet_size <= 0)
		return false;
	nr_frames = websocket_recv(connection->packet, connection->packet_size, &frames[0], msgbuf, 1);
	if (nr_frames <= 0) {
		printf(BOLDCYAN "CRITICAL: nr_frames: %d" RESET "\n", nr_frames);
		return false;
	}
	for (int x=0; x<nr_frames; x++) {
		frame = &frames[x];
		msg   = (char *)frame->data;
		printf(BOLDCYAN "%s [%d] {%d}" RESET "\n", msg, connection->fd, nr_frames);
		if ((request=www_request_hash(msg))) {
			argc_max         = (request->argc_max==0) ? MAX_RPC_ARGV : (request->argc_max);
			rpc.session      = connection->session;
			rpc.connection   = connection;
			rpc.packet       = connection->packet;
			rpc.argc         = argc = cstring_split(msg, argv, argc_max, ' ');
			rpc.argv         = argv;
			printf("argc: %d min: %d max: %d acmax: %d\n", argc, request->argc_min, request->argc_max, argc_max);
			if (request->argc_min && (argc > request->argc_max || argc < request->argc_min)) {
				printf(BOLDRED "command error: %s" RESET "\n", argv[0]);
				return false;
			}
			request->rpc_handler(&rpc);
		}
	}
	return true;
}

/* sends the "backpage" AFTER a successful login command */
void www_send_backpage(struct session *session, struct connection *connection, int send_qconf)
{
	struct website  *website  = WEBSITE;
	struct page     *backpage = website->backpage[0]; // still thinking about how to make this possible for multiple backpages
	char             packet[512 KB];
	char            *qcache;
	int              buffer, pagesize, qcache_size, packet_len = 0;

	websocket_send(connection, backpage->file, backpage->filesize);
	if (!send_qconf)
		return;
	if (session->qcache && (qcache=yyjson_mut_write(session->qcache, 0, NULL))) {
		qcache_size = strlen(qcache);
		if (qcache_size >= sizeof(packet))
			return;
		strcpy(packet+packet_len, "qcache ");
		packet_len  = 7;
		memcpy(packet+packet_len, qcache, qcache_size);
		packet_len += qcache_size;
		packet[packet_len++] = ' ';
		packet[packet_len++] = '1';
		packet[packet_len++] = '@';
	}
	packet[packet_len] = 0;
	websocket_send(connection, packet, packet_len);
}

void www_send_mainpage(struct connection *connection)
{
	struct website *website  = WEBSITE;
	struct page    *mainpage = website->mainpage[website->curpage];

	openssl_write_sync(connection, mainpage->file, mainpage->filesize);
}

void www_load_wasm()
{
	struct filemap filemap;
	char *auth_wasm = MAP_FILE_RO("www/hydrogen.wasm", &filemap);

	if (!auth_wasm)
		return;
	auth_wasm_header = (char *)malloc(filemap.filesize + sizeof(HTTP_WASM)+64);
	auth_wasm_size   = snprintf(auth_wasm_header, 256, HTTP_WASM, (int)filemap.filesize);
	memcpy(auth_wasm_header+auth_wasm_size, auth_wasm, filemap.filesize);
	auth_wasm_size  += filemap.filesize;

/*
	if (!hydro_wasm)
		return;
	libhydrogen_wasm_header = (char *)malloc(filemap.filesize + sizeof(HTTP_WASM)+64);
	libhydrogen_wasm_size   = snprintf(libhydrogen_wasm_header, 256, HTTP_WASM, (int)filemap.filesize);
	memcpy(libhydrogen_wasm_header+libhydrogen_wasm_size, libhydrogen_wasm_header, filemap.filesize);
	libhydrogen_wasm_size  += filemap.filesize;*/
}

void www_send_wasm(struct connection *connection)
{
	if (auth_wasm_header)
		openssl_write_sync(connection, auth_wasm_header, auth_wasm_size);
}

void www_send_stockdata(struct connection *connection)
{
	SSL_write(connection->ssl, stock_data_gz, stock_data_size);
}

void www_load_stockdata()
{
	struct filemap filemap;
	char          *stock_data = MAP_FILE_RO("www/stockdata.tar.gz", &filemap);

	if (!stock_data)
		return;
	stock_data_gz    = (char *)malloc(filemap.filesize + sizeof(HTTP_GZIP)+64);
	stock_data_size  = snprintf(stock_data_gz, 256, HTTP_GZIP2, filemap.filesize);
	memcpy(stock_data_gz+stock_data_size, stock_data, filemap.filesize);
	stock_data_size += filemap.filesize;	
}

void www_load_website(struct server *server)
{
	WEBSITE = build_website(server);
	if (!WEBSITE)
		return;
	www_load_wasm();
}

void apc_reload_website(struct connection *connection, char **argv)
{
	www_load_website(&Server);
	printf(BOLDGREEN "reloaded website" RESET "\n");
}

static void
www_get_request(char *request, struct connection *connection)
{
	struct website  *website  = WEBSITE;
	struct page     *mainpage = website->mainpage[website->curpage];
	char            *p        = strstr(request, "If-None");
	uint64_t         etag;

	if (unlikely(!p)) {
		printf(BOLDCYAN "NO etag (%s)\n", (char *)&mainpage->etag);
		goto out;
	}
	etag = *(uint64_t *)(p+16);
	if (etag == mainpage->etag) {
		printf(BOLDGREEN "same etag: %.8s vs %.8s" RESET "\n", (char *)&etag, (char *)&mainpage->etag);
		openssl_write_sync(connection, HTTP_304, sizeof(HTTP_304)-1);
		return;
	}
	printf(BOLDRED "stale etag: %.8s vs %.8s" RESET "\n", (char *)&etag, (char *)&mainpage->etag);
out:
	www_send_mainpage(connection);
}

void rpc_session_finish(struct rpc *rpc)
{
	if (!rpc->session->user || !rpc->session->user->logged_in)
		return;
	www_send_backpage(rpc->session, rpc->connection, 0);
}

static __inline__ int get_free_QVID(struct session *session, struct qpage *qpage)
{
	for (int x=0; x<MAX_QUADVERSES; x++) {
		struct quadverse *quadverse = session->quadverse[x];
		if (quadverse && qpage && (qpage == quadverse->qpage)) {
			printf("quadverse: %p qpage: %p\n", quadverse, qpage);
			return x;
		} else if (!qpage && !quadverse)
			return x;
	}
	return -1;
}

static __inline__ int get_profile_QVID(struct session *session, char *username)
{
	int free_QVID = 0;

	for (int x=0; x<MAX_QUADVERSES; x++) {
		struct quadverse *quadverse = session->quadverse[x];
		if (!quadverse) {
			if (!free_QVID)
				free_QVID = x;
			continue;
		}
		if (quadverse && quadverse->qpage && quadverse->flags & QUADVERSE_PROFILE) {
			if (!strcasecmp(username, quadverse->qpage->url))
				return x;
		}
	}
	if (free_QVID)
		return (free_QVID);
	return -1;
}


/* 1) User has no Cookie and has NOT logged in
 * 2) User has Cookie but has NOT logged in
 * 3) User has no cookie and has logged in      (must login and get configuration + backpage)
 * 4) User has Cookie and has logged in         (load the user's default backpage upon websocket creation)
 */
int www_websocket_async(char *request, struct connection *connection)
{
	struct session *session;
	struct url      url = {0};

	printf(BOLDWHITE "www_websocket: %.60s" RESET "\n", request);

	/* extract the cookie (if any) and get its session or malloc a new session */
	if (!(session=session_get(connection, request)))
		return 0;

	/* Allocate new websocket in a bounded circular array buffer of type socket_t (int) */
	connection->websocket_id = www_new_websocket(session, connection);

	/* extract the URL segments */
	if (!www_get_route(request+8, &url))
		return 0;

	/* Send websocket handshake response */
	if (!websocket_handshake(connection, request))
		return 0;

	/* Website Reload */
	if (url.action == ACTION_RELOAD)
		return www_reload(connection);

	/* User is uploading an object */
	if (url.action >= ACTION_UPLOAD_MIN && url.action <= ACTION_UPLOAD_MAX)
		return websocket_upload_object(session, connection, request+8);

	/* The mainpage's website.json boot RPC */
	if (url.action == ACTION_BOOT)
		if (!rpc_boot(request+8, connection))
			return 0;

	/* pack the user's saved website customization confs into connection->packet */
	session_set_config(connection);

	/* QPAGE || mainpage */
	if (url.action == ACTION_QPAGE) {
		www_set_route(connection, &url); // init qpage (builtin route (eg: /stocks), a user profile or a user quadverse, etc)
	} else {
		*(unsigned int *)(connection->packet+connection->packet_size) = INIT; // the string "init" in hex (4 bytes)
		connection->packet_size += 4;
	}

	// Send Initial Packet with user settings & init command
	if (connection->packet_size)
		websocket_send(connection, connection->packet, connection->packet_size);
}

/* HTTP GET /user           [profile endpoint] */
/* HTTP GET /user/quadverse [qpage endpoint]   */
static bool
www_route_user(struct connection *connection, struct url *url)
{
	struct qpage   *qpage;
	struct user    *user;
	struct session *session = connection->session;
	char           *packet  = connection->packet+connection->packet_size;
	struct session *usession;
	int             QVID, packet_size, profile_size;

	printf("www_route_user(): %s\n", url->segment[0]);
	user = search_user(url->segment[0]);
	if (!user || !(usession=user->session))
		return (false);
	printf("usession: %p nr_segments: %d\n", usession, url->nr_segments);

	// GET /user
	if (url->nr_segments == 1) {
		/* init profile qpage_json, profile_username QVID profile_json (profile_json: empty profile JSON) */
		strcpy(packet, "init profile [[null]] ");
		packet_size = 22;

		/* QVID */
		if (session != usession) {
			QVID = get_profile_QVID(session, url->segment[0]);
			if (QVID < 0)
				return (false);
		} else {
			QVID = QUADVERSE_PROFILE;
		}
		packet_size += snprintf(packet+packet_size, 72, "%s %d ", usession->user->uname, QVID);

		/* PROFILE JSON */
		profile_size = packet_profile(usession, packet+packet_size);
		if (!profile_size)
			return false;
		if (session != usession)
			quadverse_subscribe(session, connection, usession->quadverse[QUADVERSE_PROFILE]->qpage, QVID);
		packet_size             += profile_size;
		connection->packet_size += packet_size;
		packet[packet_size]      = 0; // null terminate for debug printing
		session->quadverse[QVID] = usession->quadverse[QUADVERSE_PROFILE];
		printf("init user packet: %s --- QVID: %d\n", packet, QVID);
		return (true);
	}

	/* GET /user/quadverse - Search for User's QuadVerse */
	for (int x = 0; x<usession->nr_qpages; x++) {
		/* init qpage qpage_json URL QVID */
		qpage = usession->qpages[x];
		if (!strcasecmp(url->segment[1], qpage->url)) {
			printf("loading qpage: %s\n", url->segment[0]);
			strcpy(packet, "init qpage ");
			packet_size = 11;
			/* QPAGE JSON */
			memcpy(packet+packet_size, qpage->qcache_json, qpage->qsize);
			packet_size += qpage->qsize;
			packet[packet_size++] = ' ';
			/* URL */
			memcpy(packet+packet_size, qpage->url, qpage->usize);
			packet_size += qpage->usize;
			/* QVID */
			QVID = get_free_QVID(session, qpage);
			if (QVID < 0)
				return false;
			printf("allocating QVID: %d for qpage quadverse\n", QVID);
			quadverse_subscribe(usession, connection, qpage, QVID);
			packet_size += snprintf(packet+packet_size, 8, " %d", QVID);
			connection->packet_size += packet_size;
			return (true);
		}
	}
	return (false);
}

static void
www_set_route(struct connection *connection, struct url *url)
{
	struct session *session     = connection->session;
	char           *packet      = connection->packet+connection->packet_size;
	packet_size_t   packet_size = 0;

	switch (url->route) {
		case URL_ROUTE_STOCKS:
			if (url->nr_segments != 2)
				goto out_init;
			connection->packet_size += snprintf(packet, 24, "init stocks %s@", url->segment[1]);
			break;
		case URL_ROUTE_OPTIONS:
			if (url->nr_segments != 2) {
				strcpy(packet, "init options@");
				packet_size = 13;
				break;
			}
			connection->packet_size += snprintf(connection->packet, 48, "init options %s@", url->segment[1]);
			break;
		case URL_ROUTE_USER:
			if (!www_route_user(connection, url))
				goto out_init;
			break;
	}
	return;
out_init:
	*(unsigned int *)(packet) = INIT;
}

static bool
www_get_route(char *request, struct url *url)
{
	char tmpbuf[128], *p;

	if (!(p=strchr(request, '\r')))
		return false;
	memcpy(tmpbuf, request, p-request);
	tmpbuf[p-request] = 0;

	request = tmpbuf;
	if (!(p=strchr(request, ' ')))
		return false;

	url->action = atoi(request);
	if (url->action >= MAX_WEBSOCKET_ACTIONS)
		return false;

	// GET /ws/ACTION_QPAGE/stocks/AAPL HTTP/1.1\r\n                    - load the AAPL stockpage
	// GET /ws/ACTION_QPAGE/username HTTP/1.1\r\n                       - load user's "squeak feed"
	// GET /ws/ACTION_QPAGE/username/quadverse25519 HTTP/1.1.\r\n       - load a user's QuadVerse
	// GET /ws/action/objtype/filetype/filesize/filename/arg1/arg2\r\n  - for uploading
	p                = request;
	url->segment[0]  = request;
	url->nr_segments = 1;
	for (int x=1; x<MAX_URL_SEGMENTS; x++) {
		while (*p != '/')
			p++;
		*p++ = 0;
		if (*p == '\0')
			break;
		url->segment[x] = strdup(p);
		url->nr_segments++;
	}

	if (url->segment[1]) {
		if (!strncasecmp(url->segment[1], "stocks/", 6))
			url->route = URL_ROUTE_STOCKS;
		else if (!strncasecmp(url->segment[1], "options/", 8))
			url->route = URL_ROUTE_OPTIONS;
		else
			url->route = URL_ROUTE_USER;
	} else {
		url->route = URL_ROUTE_ROOT;
	}
	return true;
}

int www_websocket_sync(char *req, struct connection *connection)
{
	struct session   *session;
	struct request   *request;
	struct rhash     *rhash;
	struct timeval    timeout;
	struct frame     *frame;
	struct frame      frames[6] = {0};
	struct rpc        rpc;
	char             *msg, *packet;
	char              msgbuf[2048];
	char             *argv[104];
	fd_set            rdset;
	struct url        url = {0};
	int               http_fd = connection->fd;
	int               packet_size, nbytes, argc, nr_frames;

	if (!(session=connection->session=session_get(connection, req)))
		return 0;

	printf(BOLDWHITE "www_websocket(): %.60s\n" RESET "session: %p\n", req, session);

	/* extract the URL segments */
	if (!www_get_route(req+8, &url))
		return 0;
	printf("url seg: %s url2: %s\n", url.segment[0], url.segment[1]);
	/* Send websocket handshake response */
	if (!websocket_handshake(connection, req))
		return 0;

	/* Website Reload */
	if (url.action == ACTION_RELOAD)
		return www_reload(connection);

	/* User is uploading an object */
	if (url.action >= ACTION_UPLOAD_MIN && url.action <= ACTION_UPLOAD_MAX)
		return websocket_upload_object(session, connection, req+8);

	/* The mainpage's website.json boot RPC */
	if (url.action == ACTION_BOOT)
		if (!rpc_boot(req+8, connection))
			return 0;

	/* Allocate new websocket in a bounded circular array buffer of type socket_t */
	connection->websocket_id = www_new_websocket(session, connection);

	/* pack the user's saved website customization confs into connection->packet */
	session_set_config(connection);

	/* QPAGE || mainpage */
	if (url.action == ACTION_QPAGE) {
		www_set_route(connection, &url); // init qpage (builtin route (eg: /stocks), a user profile or a user quadverse, etc)
	} else {
		*(unsigned int *)(connection->packet+connection->packet_size) = INIT; // the string "init" in hex (4 bytes)
		connection->packet_size += 4;
	}

	// [9] Send Config
	if (connection->packet_size) {
		connection->packet[connection->packet_size] = 0;
		websocket_send(connection, connection->packet, connection->packet_size);
	}
	packet = connection->packet;
//	printf(BOLDCYAN "%s" RESET "\n", packet);
	net_socket_nonblock(http_fd);
	while (1) {
		FD_ZERO(&rdset);
		FD_SET(http_fd, &rdset);
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;
		select(http_fd+1, &rdset, NULL, NULL, &timeout);
		if (FD_ISSET(http_fd, &rdset)) {
			memset(packet,   0, 128);
			memset(msgbuf,   0, sizeof(msgbuf));
			memset(&argv[0], 0, sizeof(void *)*8);
			packet_size = openssl_read_sync2(connection, packet, sizeof(msgbuf)-10);
			if (packet_size <= 0)
				break;
			nr_frames = websocket_recv(packet, packet_size, &frames[0], msgbuf, 1);
			if (nr_frames <= 0) {
				printf(BOLDCYAN "CRITICAL: nr_frames: %d" RESET "\n", nr_frames);
				break;
			}
			mutex_lock(&session->session_lock);
			for (int x=0; x<nr_frames; x++) {
				frame = &frames[x];
				msg   = (char *)frame->data;
				printf(BOLDCYAN "%s [%d] {%d}" RESET "\n", msg, http_fd, nr_frames);

				if ((request=www_request_hash(msg))) {
					rpc.session      = session;
					rpc.connection   = connection;
					rpc.packet       = packet;
					rpc.argc         = argc = cstring_split(msg, argv, request->argc_max, ' ');
					rpc.argv         = argv;
					rpc.internal     = 0;
					printf("argc: %d min: %d max: %d\n", argc, request->argc_min, request->argc_max);
					if (request->argc_min && (argc > request->argc_max || argc < request->argc_min)) {
						printf(BOLDRED "command error: %s" RESET "\n", argv[0]);
						break;
					}
					request->rpc_handler(&rpc);
				}
			}
			mutex_unlock(&session->session_lock);
		} else if (market != NO_MARKET) {
			update_page(session, connection, packet);
			watchlist_notifications(session, connection);
		}
	}
	quadverse_unsubscribe(session);
	printf(BOLDRED "CLOSED CONNECTION: %.8s client_id: %d fd: %d" RESET "\n", (char *)&session->user->cookies[connection->websocket_id], connection->websocket_id, session->websockets[connection->websocket_id]->fd);
	session->websockets[connection->websocket_id]->fd = -1;
	return 0;
}

/*
 * example: GET /api/stock/TSLA/tick
 */
void www_api(struct connection *connection, char *api)
{
	char *argv[8];
	char *p;
	int argc;

	p = strstr(api, " HTTP/1.1");
	if (!p)
		return;
	*p++ = '/';
	*p   = 0;

	argc = cstring_split(api, argv, 5, '/');
	if (argc < 1 || argc >= 5)
		return;
	if (!strcmp(argv[0], "stock")) {
		http_stock_api(connection, argv[1], argv[2], &argv[3]);
	}
}

int www_new_websocket(struct session *session, struct connection *connection)
{
	int websocket_id = 0, found_empty = 0;

	if (session->nr_websockets >= MAX_WEBSOCKETS) {
		for (int x=0; x<session->nr_websockets; x++) {
			if (session->websockets[x] && session->websockets[x]->fd == -1) {
				websocket_id = x;
				found_empty  = 1;
				break;
			}
		}
		if (!found_empty)
			close(session->websockets[0]->fd);
	} else
		websocket_id = session->nr_websockets++;
	session->websockets[websocket_id] = connection;
	return (websocket_id);
}

static __inline__ int www_reload(struct connection *connection)
{
	return websocket_send(connection, "qreload", 7);
}

void *www_process_sync(void *args)
{
	char            req[2048];
	struct connection *connection = (struct connection *)args;
	int             http_fd = connection->fd;
	unsigned int    r1, r2;
	uint64_t        r0, r8;

	while (1) {
		memset(req, 0, sizeof(req));
		if (!openssl_read_sync2(connection, req, 1024))
			goto out;
		printf(BOLDYELLOW "%.25s" RESET "\n", req);
		r0 = *(uint64_t *)req;
		r1 = *(uint32_t *)req;
		r2 = *(uint32_t *)(req+4);
		r8 = *(uint64_t *)(req+4);

		/* GET REQUESTS */
		if (r1 == 0x20544547) {     /* GET / */
			if (r0 == 0x5448202f20544547) {
				www_get_request(req, connection);
				continue;
			}
			switch (r2) {
				/* GET /ws/ */
				case 0x2f73772f:
					www_websocket_sync(req, connection);
					goto out;
				/* GET /favicon.ico */
				case 0x7661662f:
					if (WWW_GET_IMAGE(req+5, connection))
						continue;
					break;
				/* GET /img */
				case 0x676d692f:
					if (WWW_GET_IMAGE(req+9, connection))
						continue;
					goto out;
				/* blob */
				case 0x6f6c622f:
					if (HTTP_BLOB(req+10, connection))
						continue;
					goto out;
				/* GET /gztab */
				case 0x747a672f:
					http_send_monster(connection);
					goto out;
				/* GET /api/ */
				case 0x6970612f:
					www_api(connection, req+9);
					goto out;
				/* GET /ROBO */
				case 0x626f722f:
					openssl_write_sync(connection, ROBOTS_TXT, sizeof(ROBOTS_TXT)-1);
					break;
				/* GET /wasm */
				case 0x7361772f:
					www_send_wasm(connection);
					break;
				/* GET /data */
				case 0x7461642f:
					www_send_stockdata(connection);
					break;
				default:
					www_get_request(req, connection);
					continue;
			} // switch (r2)
			break; // break out of while(1)
		} // if (GET REQUEST)

		// HEAD request
		if (r1 == 0x44414548)
			openssl_write_sync(connection, REDIRECT_HTTPS, sizeof(REDIRECT_HTTPS)-1);
	} // while(1)
out:
	shutdown(http_fd, SHUT_WR);
	if (http_fd <= 2)
		return NULL;
	close(http_fd);
	free(connection);
	return (NULL);
}

void *www_http_server_sync(void *args)
{
	struct sockaddr_in srv, cli;
	char               buf[1024];
	socklen_t          slen   = 16;
	int                val    = 1, sockfd, client_fd;
	port_t             port;
	in_addr_t          ipaddr;

	if (Server.production) {
		port   = 80;
		ipaddr = INADDR_ANY;
	} else {
		port = Server.http_port;
		ipaddr = LOCALHOST;
	}
		
	sockfd = net_tcp_bind(INADDR_ANY, 80);
	if (sockfd == -1) {
		printf(BOLDRED "[-] Failed to bind() to port: %d" RESET "\n", port);
		return NULL;
	}
	for (;;) {
		client_fd = accept4(sockfd, (struct sockaddr *)&cli, &slen, SOCK_CLOEXEC);
		if (client_fd < 0)
			continue;
		recv(client_fd, buf, 1024, 0);
		net_send(client_fd, (char *)REDIRECT_HTTPS,  sizeof(REDIRECT_HTTPS)-1);
		close(client_fd);
	}
}

/*
 * Sync www https server: openssl_server_sync() will
 * create a thread that will call www_process_sync()
 * when it completes the TLS Handshake
 */
void *www_https_server_sync(void *args)
{
	in_addr_t ipaddr = LOCALHOST;
	port_t    port;

	if (Server.production) {
		ipaddr = INADDR_ANY;
		port   = 443;
	} else {
		port   = Server.https_port;
	}
	openssl_server_sync(www_process_sync, ipaddr, port, 1);
}

/*
 * Async
 */
void www_process_request(struct connection *connection)
{
	char         *request = connection->packet;
	unsigned int  r1, r2;
	uint64_t      r0, r8;

	printf(BOLDYELLOW "www_process_request(): %.20s" RESET "\n", request);

	r0 = *(uint64_t *)request;
	r1 = *(uint32_t *)request;
	r2 = *(uint32_t *)(request+4);
	r8 = *(uint64_t *)(request+4);

	/* GET REQUESTS */
	if (r1 == 0x20544547) {     /* GET / */
		if (r0 == 0x5448202f20544547) {
			www_get_request(request, connection);
			return;
		}
		switch (r2) {
			/* GET /ws/ */
			case 0x2f73772f:
				www_websocket_async(request, connection);
				connection->state |= NETWORK_STATE_WEBSOCKET;
				return;
		}
	} else if (r1 == 0x44414548) { // HEAD request
		openssl_write_sync(connection, REDIRECT_HTTPS, sizeof(REDIRECT_HTTPS)-1);
	}
}

int www_process_async(struct connection *connection)
{
	int nbytes;

	if ((!(connection->state & NETWORK_STATE_TLS_HANDSHAKE_DONE)) && ((connection_ssl_handshake(connection)<0))) {
		printf("www_process_requests: error\n");
		return -1;
	}

	if (connection->state & NETWORK_STATE_WEBSOCKET) {
		printf("www_process_async(): NETWOK_STATE_WEBSOCKET\n");
		return websocket_process_request(connection);
	}
	if (!(connection->packet = zmalloc(MAX_PACKET_SIZE)))
		return 0;
	connection->packet_size_max = MAX_PACKET_SIZE;

	/* now that the SSL handshake is complete, connection->recv() points to connection_ssl_recv() */
	printf("www_process_async(): calling recv(): %p max_size: %lld\n", connection->recv, connection->packet_size_max);
	nbytes = connection->recv(connection);
	if (nbytes <= -1) {
		return -1;
	}
	if (nbytes == 0) {
		printf("ssl died\n");
		return 0;
	}
	connection->packet[nbytes] = 0;
	printf(BOLDGREEN "processing request" RESET "\n");
	www_process_request(connection);
	return 1;
}

/*
 * Main event loop thread (one per vCPU)
 */
void *www_server_async(void *args)
{
	struct connection  *connection;
	struct epoll_event  https_server_events;
	struct epoll_event *events, *event;
	struct sockaddr_in  client_addr;
	ssl_server_t       *ssl_server;
	port_t              port;
	in_addr_t           ipaddr;
	int                 val, nready, epoll_fd, fd, client_fd, https_server_fd, client_addr_len;

	if (Server.production) {
		port   = 443;
		ipaddr = INADDR_ANY;
	} else {
		port   = Server.https_port;
		ipaddr = INADDR_ANY;
	}

	epoll_fd        = epoll_create1(EPOLL_CLOEXEC);
	https_server_fd = net_tcp_bind(ipaddr, port);
	net_socket_nonblock(https_server_fd);

	if (!WWW_SERVER && !(WWW_SERVER=openssl_server("www/cert.pem", "www/key.pem"))) {
		printf(BOLDRED "[-] Failed to load ssl server on port: %d" RESET "\n", port);
		return NULL;
	}

	https_server_events.events  = EPOLLIN;
	https_server_events.data.fd = https_server_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, https_server_fd, &https_server_events);
	events = (struct epoll_event *)zmalloc(sizeof(*events) * MAXEVENTS);
	while (1) {
		nready = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
		for (int x = 0; x<nready; x++) {
			event = &events[x];
			fd    = event->data.fd;
			if (fd == https_server_fd) {
				client_fd = accept4(https_server_fd, (struct sockaddr *)&client_addr, &client_addr_len, O_NONBLOCK);
				if (client_fd < 0) {
					if (errno == EAGAIN || errno == EWOULDBLOCK)
						continue;
				}
				if (((connection=openssl_accept_async(WWW_SERVER, client_fd))<0)) {
					close(client_fd);
					continue;
				}

				event->events    = EPOLLIN|EPOLLET;
				event->data.ptr  = (void *)connection;
				connection->recv = www_process_async;
				connection->send = www_process_async;
				printf("EPOLLADD fd: %d epoll_fd: %d\n", connection->fd, epoll_fd);
				fflush(stdout);
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, event);
			} else if (event->events & EPOLLIN) {
				/* Connection Event recv() */
				connection = (struct connection *)event->data.ptr;
				connection->recv(connection);
				printf("EPOLLIN fd: %d epoll_fd: %d\n", connection->fd, epoll_fd);
				fflush(stdout);
			} else if (event->events & EPOLLOUT) {
				/* Connection Event send() */
				connection = (struct connection *)event->data.ptr;
				printf("EPOLLOUT fd: %d epoll_fd: %d\n", connection->fd, epoll_fd);
				fflush(stdout);
				event->events &= ~EPOLLOUT;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, connection->fd, event);
			}
		}
	}
}

void *init_www(void *args)
{
	www_callback_t www_callback;
	int            nr_www_threads;

	www_load_stockdata();
	www_init_requests();
	www_load_website(&Server);
	blob_load_images("www/img");

	thread_create(www_http_server_sync, NULL);

	if (Server.XLS->config->async)
		www_callback = www_server_async;
	else
		www_callback = www_https_server_sync;

	if (Server.nr_vcpu > 4)
		nr_www_threads = (Server.nr_vcpu - 2);

	for (int x=0; x<nr_www_threads; x++)
		thread_create(www_callback, NULL);
	return (NULL);
}

