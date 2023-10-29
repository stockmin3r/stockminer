#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

static struct ssl_server *APC_SERVER;
static socket_t           apc_server_fd;

static __inline__ int net_string(int sockfd, char *str, int maxlen)
{
	int len;

	if (recv(sockfd, (void *)&len, 4, 0) != 4)
		return 0;
	if (len <= 0 || (maxlen && len >= maxlen))
		return 0;
	if (recv(sockfd, str, len, 0) != len)
		return 0;
	str[len] = 0;
	return (len);
}

struct stock *cmd_get_stock(int sockfd)
{
	char ticker[16] = {0};

	if (!net_string(sockfd, ticker, 8))
		return NULL;
	return search_stocks(ticker);
}

void cmd_json_clear(int sockfd)
{
	struct session *session;
	char username[MAX_USERNAME_SIZE+1];

	if (!net_string(sockfd, username, MAX_USERNAME_SIZE))
		return;

	if (!(session = session_by_username(username))) {
		printf(BOLDRED "cmd_json_clear(): username: %s does not exist" RESET "\n", username);
		return;
	}
	session->qcache = NULL;
}

void cmd_obj_list(int sockfd)
{
	struct object *obj = NULL;
	struct object *tmp = NULL;

	HASH_ITER(hh, Objects, obj, tmp) {
		printf("obj: %p key: %s uid: %d\n", obj, obj->key, obj->uid);
	}
}

void cmd_www(int sockfd)
{
	struct www *www = NULL, *tmp;
	char url[256];
	char *colors[] = { BOLDRED, BOLDGREEN, BOLDMAGENTA, BOLDCYAN, BOLDWHITE, BOLDYELLOW };
	char msg[512 KB];
	char *color;
	int nbytes, color_index = 0, x;

	nbytes = net_string(sockfd, url, sizeof(url)-1);
	if (nbytes <= 0 || nbytes > sizeof(url))
		return;

	if (*url != '*')
		HASH_FIND_STR(WWW_HASHTABLE, url, www);
	if (!www) {
		printf(BOLDRED "no entry for: %s" RESET "\n", url);
		HASH_ITER(hh, WWW_HASHTABLE, www, tmp) {
			printf(BOLDWHITE "URL: %s nr_tables: %d www: %p" RESET "\n", url, www->nr_tables, www);
		}
		return;
	}
	for (x=0; x<www->nr_tables; x++) {
		struct table *table = www->tables[x];
		if (!table)
			continue;
		color  = colors[color_index++];
		nbytes = snprintf(msg, sizeof(msg)-1, "%s%s" RESET, color, table->html);
		if (color_index >= 6)
			color_index = 0;
		printf("%s\n", msg);
	}
	printf(BOLDGREEN "URL: %s nr_tables: %d www: %p" RESET "\n", url, www->nr_tables, www);
}

void cmd_ebuild_stock(int sockfd)
{
	struct stock *stock = cmd_get_stock(sockfd);

	if (!stock)
		return;
	build_earnings(stock);
}

void cmd_candle(int sockfd)
{
	struct stock *stock = cmd_get_stock(sockfd);
	struct mag   *mag;
	char          cname[8];
	int x, len, nr_candles;

	if (!stock || !(mag=stock->mag))
		return;

	len = net_string(sockfd, cname, 7);
	if (!len)
		return;

	nr_candles = mag->nr_candles;
	for (x=0; x<nr_candles; x++) {
		struct CANDLE *C = &mag->candles[x];
		if (!C->name || strcmp(cname, C->name))
			continue;
		printf("%s %-24s entry: %d type: %d\n", C->date, C->name, C->entry, C->type);
	}

	for (x=0; x<NR_CANDLES; x++) {
		if (!strcmp(cname,  CTYPES[x].name)) {
			struct candle *candle = stock->candles[x];
			if (candle->type == CANDLE_BULL)
				printf("[%s] day1_bull: %.2f day3_bull: %.2f day7_bull: %.2f day21_bull: %.2f\n", CTYPES[x].name, candle->day1_bull, candle->day3_bull, candle->day7_bull, candle->day21_bull);
			else if (candle->type == CANDLE_BEAR)
				printf("[%s] day1_bear: %.2f day3_bear: %.2f day7_bear: %.2f day21_bear: %.2f\n", CTYPES[x].name, candle->day1_bear, candle->day3_bear, candle->day7_bear, candle->day21_bear);
		}
	}
}

void cmd_session_list(int http_fd)
{
	struct session   *session;
	struct list_head *session_list;
	char              buf[1024 KB];
	char             *ptr = buf;
	int               nbytes = 0, size;

	session_list = get_session_list();
	SESSION_LOCK();
	DLIST_FOR_EACH_ENTRY(session, session_list, list) {
		size = snprintf(ptr+nbytes, MAX_USERNAME_SIZE+1, "%s\n", session->user->uname);
		ptr      += size;
		nbytes   += size;
		printf("session user: %s (cookie(s) will go here) %p\n", session->user->uname, session);
	}
	SESSION_UNLOCK();
	net_send(http_fd, (void *)&nbytes, 4);
	net_send(http_fd, buf, nbytes);
}

void cmd_ohlc(int sockfd)
{
	struct stock *stock;
	char name[256];
	int len, error = -1, x;

	recv(sockfd, (void *)&len, 4, 0);
	if (len <= 0 || len >= 6)
		return;
	recv(sockfd, name, len, 0);
	name[len] = 0;
	stock = search_stocks(name);
	if (!stock) {
		write(sockfd, (void *)&error, 4);
		return;
	}
	for (x=0; x<stock->nr_ohlc; x++) {
		struct ohlc *ohlc = &stock->ohlc[x];
		printf("Timestmap: %lu Open: %.2f Close: %.2f\n", ohlc->timestamp, ohlc->open, ohlc->close);
	}
}

void cmd_yahoo(int sockfd)
{
	struct stock *stock = cmd_get_stock(sockfd);
	char buf[1024 KB];
	char url[256];
	int x;

	if (!stock)
		return;
	sprintf(url, YAHOO_OHLC, stock->sym);
	for (x=0; x<3; x++) {
		if (!curl_get(url, buf))
			break;
	}
	printf("%s\n", buf);
}

void cmd_live(int sockfd)
{
	struct stock *stock = cmd_get_stock(sockfd);
	int x;

	if (!stock)
		return;
	printf(BOLDGREEN "Open: %.2f High: %.2f Low: %.2f Close: %.2f" RESET "\n", stock->current_open, stock->current_high, stock->current_low, stock->current_close);
	for (x=0; x<stock->nr_ohlc; x++) {
		struct ohlc *ohlc = &stock->ohlc[x];
		printf(BOLDWHITE "[%d] Open: %.2f High: %.2f Low: %.2f Close: %.2f Volume: %llu" RESET "\n", x, ohlc->open, ohlc->high, ohlc->low, ohlc->close, ohlc->volume);
	}
	printf("nr_ohlc: %d\n", stock->nr_ohlc);
}

void cmd_user_add(int sockfd)
{
	struct rpc rpc;
	char       username[256];
	char       password[256];
	char      *argv[3];

	if (!net_string(sockfd, username, MAX_USERNAME_SIZE) || !net_string(sockfd, password, 255))
		return;

	memset(&rpc, 0, sizeof(rpc));
	argv[1]  = username;
	argv[2]  = password;
	rpc.argv = argv;
	rpc_user_register(&rpc);
}

void cmd_alerts(int sockfd)
{
	struct watchlist *watchlist;
	struct watchcond *watchcond;
	struct session *session;
	char username[256];
	char packet[64 KB]; // XXX: HEAP
	int x, y, nbytes, nr_alerts, packet_len = 0;

	if (!net_string(sockfd, username, MAX_USERNAME_SIZE))
		return;

	session = session_by_username(username);
	for (x=0; x<session->nr_watchlists; x++) {
		watchlist = session->watchlists[x];
		nr_alerts = watchlist->nr_conditions;
		for (y=0; y<nr_alerts; y++) {
			watchcond   = &watchlist->conditions[y];
			nbytes      = sprintf(packet+packet_len, "[exec %s]\n", watchcond->exec);
			packet_len += nbytes;
		}
	}
	packet[packet_len] = 0;
	net_send(sockfd, &packet_len, 4);
	net_send(sockfd, packet, packet_len);
}

void cmd_signals(int sockfd)
{
	struct stock  *stock = cmd_get_stock(sockfd);
	struct action *action;
	struct mag    *mag;
	char          *date;
	int            x;

	if (!stock || !(mag=stock->mag))
		return;

	for (x=0; x<mag->nr_action1; x++) {
		action = &mag->action1[x];
		if (action->entry <= 0 || action->entry >= mag->nr_entries) {
			printf(BOLDRED "ENTRY SIGNAL ERROR: %d 5days: %d 5pc: %.2f 10days: %d 10pc: %.2f" RESET "\n",mag->nr_entries, action->five_days, action->five_pt, action->ten_days, action->ten_pt);
			continue;
		}
		date = mag->date[action->entry];
		printf("[%s] {Action 1} 5pt: %.2f (%d days) 10pt: %.2f (%d days) (STATUS: %d, ENTRY: %d)\n", date, action->five_pt, action->five_days, action->ten_pt, action->ten_days, action->status, action->entry);
	}
	for (x=0; x<mag->nr_action4; x++) {
		action = &mag->action4[x];
		if (action->entry <= 0 || action->entry >= mag->nr_entries) {
			printf(BOLDRED "ENTRY SIGNAL ERROR: %d 5days: %d 5pc: %.2f 10days: %d 10pc: %.2f" RESET "\n",mag->nr_entries, action->five_days, action->five_pt, action->ten_days, action->ten_pt);
			continue;
		}
		date = mag->date[action->entry];
		printf("[%s] {Action 4} 5pt: %.2f (%d days) 10pt: %.2f (%d days) (STATUS: %d, ENTRY: %d)\n", date, action->five_pt, action->five_days, action->ten_pt, action->ten_days, action->status, action->entry);
	}
}

void cmd_print_stocks(int sockfd)
{
	struct stock *stock;
	struct stock **stocks;
	struct XLS *XLS;
	char buf[64 KB]; // XXX: HEAP
	int nbytes = 0, x, nr_stocks;

	XLS       = CURRENT_XLS;
	nr_stocks = XLS->nr_stocks;
	stocks    = XLS->STOCKS_PTR;
	for (x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		if (stock->price_15m > 1.0) {
			nbytes += sprintf(buf+nbytes, "%s (ID: %d) %.2f [%.2f,%.2f,%.2f]\n",stock->sym, stock->id, stock->pr_percent, stock->price_15m, stock->price_5m, stock->price_1min);
			// XXX: REALLOC
		}
	}
	net_send(sockfd, (void *)&nbytes, 4);
	net_send(sockfd, buf, nbytes);
}

void cmd_print_stock(int sockfd)
{
	struct stock *stock = cmd_get_stock(sockfd);
	char buf[4096];
	int len, x;

	if (!stock)
		return;

	len = snprintf(buf, sizeof(buf)-1, "%p (ID: %d) current_price: %.2f pr_percent: %.2f vol: %llu prior_close: %.2f (15m: %.2f, 5m: %.2f 1m: %.2f)\n",
				  stock, stock->id, stock->current_price, stock->pr_percent, stock->current_volume, stock->prior_close,
				  stock->price_15m, stock->price_5m, stock->price_1min);
	send(sockfd, buf, len, 0);
	printf("%s\n", buf);
	for (x=0; x<stock->nr_signals; x++) {
		struct sig *sig = stock->signals[x];
		printf("sig_entry: %s sig_exit: %s nr_days: %d\n", sig->entry_date, sig->exit_date, sig->nr_days);
	}
	printf("thread_id: %d update: %d nr_ohlc: %d sig_avgdays: %.2f\n", stock->thread_id, stock->update, stock->nr_ohlc, stock->sig_avgdays);

//	printf("rank 2020-11-30: %d\n", date_to_rank(stock, "2020-11-30"));
//	printf("rank 2021-02-04: %d\n", date_to_rank(stock, "2021-02-04"));
}

void cmd_debug(int sockfd)
{
	char msg[256];
	char *ticker, *p;

	if (!net_string(sockfd, msg, 32))
		return;

	ticker = msg;
	p = strchr(ticker, ':');
	if (p)
		Server.DEBUG_THREAD = atoi(p+1);
	Server.DEBUG_MODE  = 1;
	printf(BOLDWHITE "ENTERING DEBUG MODE" RESET "\n");
	if (*ticker == '-')
		return;
	Server.DEBUG_STOCK = strdup(ticker);
}

void cmd_getpid(int sockfd)
{
	int pid = os_getpid();
	send(sockfd, (const char *)&pid, 4, 0);
}

void cmd_netsh(int sockfd)
{
	struct session *session;
	char buf[8 KB];
	char cmd[9 KB];
	size_t packet_len = 0;
	int nbytes;

	recv(sockfd, (void *)&packet_len, 4, 0);
	if (packet_len > 256)
		return;
	nbytes = recv(sockfd, buf, sizeof(buf)-1, 0);
	buf[nbytes] = 0;
	if (nbytes <= 0)
		return;
	session = session_by_cookie(buf);
	if (!session)
		return;
	nbytes = snprintf(cmd, sizeof(cmd)-1, "netsh %s", buf+9);
	websockets_sendall(session, cmd, nbytes);
}

void *admin_server_loop(void *args)
{
	struct connection *connection = (struct connection *)args;
	char               packet[4096];
	packet_size_t      packet_size;
	apc_t              apc;
	char              *arg_str;
	void              *arg_obj;
	int                arg_str_size, arg_obj_size;
	int                arg_int;

	packet_size = openssl_read_sync(connection, packet, sizeof(packet)-1);
	if (!packet_size || packet_size >= sizeof(packet)-1)
		return NULL;
	
	apc = *(unsigned int *)packet;
	switch (APC_GET_ARG(apc)) {
		case APC_ARG_INT:
			arg_int = *(int *)(packet+sizeof(apc));
			break;
		case APC_ARG_STR:
			arg_str_size = *(packet_size_t *)(packet+sizeof(apc));
			if (arg_str_size >= MAX_APC_STRLEN)
				return NULL;
			arg_str = (packet+sizeof(apc)+sizeof(packet_size_t));
			break;
	}
	printf("apc: %d packet: %p\n", apc, packet);
	switch (apc) {
		case APC_RELOAD_WEBSITE:
			www_load_website(&Server);
			printf(BOLDCYAN "RELOADED HTTP FILES" RESET "\n");
			break;
		case APC_UPDATE_WSJ:
			wsj_update_EOD();
			printf("wsj_update_EOD done\n");
			break;
		case APC_UPDATE_EOD:
			UPDATE_EOD();
			printf("update_EOD done\n");
			break;
		case APC_UPDATE_FORKS:
			update_forks_EOD();
			break;
		case APC_UPDATE_EARNINGS:
			update_earnings(CURRENT_XLS);
			break;
		case APC_EBUILD_STOCKS:
			build_stock_earnings();
			break;
		case APC_OBJ_LIST:
//			cmd_obj_list(sockfd);
			break;
		case APC_JSON_CLEAR:
//			cmd_json_clear(sockfd);
			break;
		case APC_PRINT_STOCKS:
//			cmd_print_stocks(sockfd);
			break;
		case APC_PRINT_STOCK:
//			cmd_print_stock(sockfd);
			break;
		case APC_SESSION_LIST:
///			cmd_session_list(sockfd);
			break;
		case APC_USER_ADD:
//			cmd_user_add(sockfd);
			break;
		case APC_OHLC:
//			cmd_ohlc(sockfd);
			break;
		case APC_LIVE:
//			cmd_live(sockfd);
			break;
		case APC_EARNINGS: //int
//			cmd_dump_earnings(sockfd);
			break;
		case APC_EBUILD_STOCK:
//			cmd_ebuild_stock(sockfd);
			break;
		case APC_SIGNALS:
//			cmd_signals(sockfd);
			break;
		case APC_WATCHLIST:
//			list_watchlists(sockfd);
			break;
		case APC_ALERTS:
//			cmd_alerts(sockfd);
			break;
		case APC_DEBUG_STOCK:
//			cmd_debug(sockfd);
			break;
		case APC_CANDLE:
//			cmd_candle(sockfd);
			break;
		case APC_GETPID:
//			cmd_getpid(sockfd);
			break;
		case APC_NETSH:
//			cmd_netsh(sockfd);
			break;
		case APC_WWW:
//			cmd_www(sockfd);
			break;
	}
	close(connection->fd);
	return NULL;
}

void *netctl_thread(void *args)
{
	struct sockaddr_in  srv, cli;
	struct connection  *connection = NULL;
	socklen_t           slen       = 16;
	int                 client_fd, err;

	apc_server_fd = net_tcp_bind(LOCALHOST, NETCTL_PORT);
	if (apc_server_fd == -1) {
		printf(BOLDRED "[-] Failed to bind to NETCTL_PORT: %d" RESET "\n", NETCTL_PORT);
		return NULL;
	}

	APC_SERVER = openssl_server("www/cert.pem", "www/key.pem");
	if (!APC_SERVER)
		return NULL;

	for (;;) {
		if (!connection) {
			connection      = (struct connection *)zmalloc(sizeof(*connection));
			connection->ssl = SSL_new(APC_SERVER->ssl_ctx);
		}
		client_fd = accept4(apc_server_fd, (struct sockaddr *)&cli, &slen, SOCK_CLOEXEC);
		if (client_fd < 0)
			continue;
		connection->fd = client_fd;
		SSL_set_fd(connection->ssl, client_fd);
		SSL_set_accept_state(connection->ssl);
		if (!(connection->packet=malloc(MAX_PACKET_SIZE))) {
			close(client_fd);
			continue;
		}

		if ((err=SSL_accept(connection->ssl)) <= 0 && SSL_get_error(connection->ssl, err)) {
			close(client_fd);
			SSL_free(connection->ssl);
			connection = NULL;
			continue;
		}
		thread_create(admin_server_loop, (void *)connection);
		connection = NULL;
	}
}
