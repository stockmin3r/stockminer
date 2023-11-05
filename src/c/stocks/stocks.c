#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

struct XLS    *CURRENT_XLS;
struct stock **MENU_STOCKS_PTR;
char          *delisted_table;
int            delisted_table_size;
int            AFH_START;
int            AFH_DONE;
struct curl_slist *resolve = NULL; // to be removed

char *FUND[]            = { "SPY",   "XLB",   "XLC",  "XLE", "XLF", "XLI", "XLK", "XLP", "XLU", "XLV", "XLY", "XLRE" };
char *IX[]              = { "^GSPC", "^IXIC", "^DJI", "^RUT" };
char *MENU_STOCKS_STR[] = { "SPY",   "^IXIC", "^DJI", "XLK"  };

char *SPDR[]            = { "XLC", "XLY", "XLP", "XLE", "XLF", "XLV", "XLI", "XLB", "XLRE", "XLK", "XLU", "XLRE" };
char *SPDR_SECTORS[]    = { "Communication Services", "Consumer Discretionary", "Consumer Staples", "Energy", "Finance", "Health Care", "Industrials", "Materials", "Real Estate", "Technology", "Utilities" };
char *SECTORS[]         = { "Communication Services", "Consumer Discretionary", "Consumer Staples", "Energy", "Finance",
                            "Health Care","Industrials","Materials","Real Estate", "Technology","Utilities" };

int is_index(char *ticker)
{
	int x;

	for (x=0; x<sizeof(IX)/sizeof(char *); x++) {
		if (!strcmp(IX[x], ticker))
			return 1;
	}
	return 0;
}

int is_fund(char *ticker)
{
	int x;

	for (x=0; x<sizeof(FUND)/sizeof(char *); x++) {
		if (!strcmp(FUND[x], ticker))
			return 1;
	}
	return 0;
}

void set_index(struct stock *stock)
{
	int idx = -1, x;

	for (x=0; x<NR_MENU_STOCKS; x++) {
		if (!strcmp(MENU_STOCKS_STR[x], stock->sym)) {
			idx = x;
			break;
		}
	}
	if (idx != -1)
		MENU_STOCKS_PTR[idx] = stock;
	if (is_index(stock->sym) || is_fund(stock->sym))
		stock->type = STOCK_TYPE_INDEX;
}

int stocks_get_indexes(char *ibuf)
{
	struct stock *stock;
	int x, nbytes, isize = 0;

	for (x=0; x<NR_MENU_STOCKS; x++) {
		stock  = MENU_STOCKS_PTR[x];
		if (!stock)
			continue;
		nbytes = snprintf(ibuf, 32, "%.2f!%.2f&", stock->current_price, stock->pr_percent);
		ibuf  += nbytes;
		isize += nbytes;
	}
	return (isize);
}

void cmd_delisted(void)
{
	struct filemap filemap;
	char buf[256 KB];
	char *tickers[132];
	int nr_tickers, x;

	if (!fs_readfile_str("/dev/shm/delisted.stocks", buf, sizeof(buf)))
		return;
	nr_tickers = cstring_split(buf, tickers, 128, '\n');
	if (!nr_tickers)
		return;

	delisted_table = (char *)malloc(32 KB);
	strcpy(delisted_table, "<table id=DELISTED><thead><tr><th>Ticker</th><th>Date</th><th>Report Error</th></tr></thead><tbody>");
	delisted_table_size = strlen(delisted_table);
	for (x=0; x<nr_tickers; x++)
		delisted_table_size += snprintf(delisted_table+delisted_table_size, 128, "<tr><td>%s</td><td>%s</td><td class=delisted onclick=report_delisted()</td></tr>", tickers[x], QDATE[1]);
	strcpy(delisted_table+delisted_table_size, "</tbody></table>");
	delisted_table_size += 16;
}

void websocket_send_delisted(struct connection *connection)
{
	websocket_send(connection, delisted_table, delisted_table_size);
}

void init_ticker_hashtable(struct XLS *XLS)
{
	int x;

	for (x=0; x<XLS->nr_stocks; x++) {
		struct thash *thash = (struct thash *)malloc(sizeof(*thash));
		struct stock *stock = &XLS->STOCKS_ARRAY[x];
		*(uint64_t *)thash->ticker = *(uint64_t *)stock->sym;
		thash->stock  = stock;
		HASH_ADD_STR(XLS->hashtable, ticker, thash);
	}
}

void load_fundamentals(struct stock *stock)
{
	char    path[256];
	int64_t filesize;
	int     pathsize, nr_funds;

	/* Yahoo Fundamentals */
	pathsize = snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.f", stock->sym);
	filesize = fs_filesize(path);
	if (filesize > 0) {
		nr_funds         = filesize/sizeof(struct fund_int);
		stock->fund_int  = (struct fund_int *)malloc(filesize);
		fs_readfile(path, (char *)stock->fund_int, sizeof(stock->fund_int));
		path[pathsize++] = 's';
		path[pathsize]   = 0;
		fs_readfile(path, (char *)&stock->fund_str, sizeof(stock->fund_str));
	}
	/* Yahoo Earnings */
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.e", stock->sym);
	fs_readfile(path, (char *)&stock->earnings, sizeof(stock->earnings));
	stock->EARNINGS = &stock->earnings;
}

unsigned short stock_id(char *ticker)
{
	struct stock *stock;

	if (unlikely(!ticker || !(stock=search_stocks(ticker))))
		return -1;
	return stock->id;
}

struct stock *search_stocks(char *ticker)
{
	struct thash *thash = NULL;

	if (!ticker) {
		printf(BOLDRED "search_stocks ticker: %p" RESET "\n", ticker);
		return NULL;
	}
	HASH_FIND_STR(CURRENT_XLS->hashtable, ticker, thash);
	if (thash)
		return thash->stock;
	return (NULL);
}

struct stock *search_stocks_XLS(struct XLS *XLS, char *ticker)
{
	struct thash *thash = NULL;
	HASH_FIND_STR(XLS->hashtable, ticker, thash);
	if (thash)
		return thash->stock;
	return (NULL);
}

void verify_stocks(struct XLS *XLS)
{
	struct server *config = XLS->config;
	int x, nr_stocks      = XLS->nr_stocks;

	for (x=0; x<nr_stocks; x++) {
		struct stock *stock = &XLS->STOCKS_ARRAY[x];
//		if (stock->mag && stock->mag->nr_entries != stock->nr_mag2_entries && (config->production))
//			printf("stock: %s nr_entries: %d nr_mag2_entries: %d\n", stock->sym, (int)stock->mag->nr_entries, (int)stock->nr_mag2_entries);
	}
}

void init_ip()
{
	struct hostent *hp;
	struct in_addr ipaddr;
	char addr[512];

	Server.WSJ_ADDR = ipaddr.s_addr = net_get_hostname("api.wsj.net");
	snprintf(addr, sizeof(addr)-1, "api.wsj.net:443:%s", inet_ntoa(ipaddr));
	resolve = curl_slist_append(resolve, addr);

	Server.CBOE_ADDR  = net_get_hostname("cdn.cboe.com");
	Server.YAHOO_ADDR = net_get_hostname("finance.yahoo.com");
}

static __inline__ void preset_path(struct session *session, char *path)
{
	struct user *user = session->user;
	char cookie[8];

	if (!user->logged_in) {
//		*(uint64_t *)cookie = user->cookie;
		cookie[7] = 0;
		snprintf(path, 32, "db/uid/free/%s.indi", cookie);
	} else {
		snprintf(path, 32, "db/uid/%d.indi", user->uid);
	}
}

void http_stock_api(struct connection *connection, char *ticker, char *api, char **opt)
{
	struct stock *stock;
	struct ohlc  *ohlc;
	char packet[512];
	char json[512];
	int packet_len, json_size, header_size;

	if (!ticker || !api || !(stock=search_stocks(ticker)))
		return;

	ohlc        = &stock->ohlc[stock->nr_ohlc-1];
	json_size   = snprintf(json,   128, "{\"t\":\"%lu\",\"o\":\"%.3f\",\"h\":\"%.3f\",\"l\":\"%.3f\",\"c\":\"%.3f\"}", ohlc->timestamp, ohlc->open, ohlc->high, ohlc->low, ohlc->close);
	header_size = snprintf(packet, 128, "HTTP/1.1 200 OK\r\nContent-type: application/json\r\nContent-Length: %d\r\n\r\n", json_size);
	memcpy(packet+header_size, json, json_size);
	packet_len  = (header_size + json_size);
	openssl_write_sync(connection, packet, packet_len);
}

void rpc_indicator_save(struct rpc *rpc)
{
	struct session *session     = rpc->session;
	char           *preset_name = rpc->argv[1]; // argv[1]: chart indicator preset name
	char           *indicators  = rpc->argv[2]; // argv[2]: stock indicators
	struct preset  *preset;
	char            path[256];

	printf(BOLDYELLOW "rpc_indicator_save(): name: %s indicators: %s" RESET "\n", preset_name, indicators);
	preset_path(session, path);
	preset = (struct preset *)malloc(sizeof(*preset));
	strncpy(preset->name, preset_name, 63);
	strncpy(preset->indicators, indicators, 63);

	fs_appendfile(path, (void *)preset, sizeof(*preset));

//	mutex_lock(&session->watchlist_lock);
	session->presets[session->nr_presets++] = preset;
//	mutex_unlock(&session->watchlist_lock);
}

void rpc_indicator_edit(struct rpc *rpc)
{
	struct session *session   = rpc->session;
	int             pid       = atoi(rpc->argv[1]);
	int             workspace = atoi(rpc->argv[2]);
	int             activate  = atoi(rpc->argv[3]);
	struct preset  *preset;
	struct filemap  filemap;
	char           *map;
	char            path[256];
	int             filesize, fd, len;

	printf("PRESET MODIFY: %d workspace: %d activate: %d\n", pid, workspace, activate);
	if (pid >= session->nr_presets || pid < 0)
		return;
	preset = session->presets[pid];
	if (!preset)
		return;
	len = strlen(preset->indicators);
	if (len < 4)
		return;
	*(preset->indicators+len-3) = (workspace+48);
	if (activate != 2)
		*(preset->indicators+len-1) = (activate+48);

	preset_path(session, path);
	map = MAP_FILE_RW(path, &filemap);
	if (!map)
		return;
	preset = (struct preset *)(map+(sizeof(*preset)*pid));
	*(preset->indicators+len-3) = (workspace+48);
	if (activate != 2)
		*(preset->indicators+len-1) = (activate+48);
	UNMAP_FILE(map, &filemap);
}

int websocket_presets(struct session *session, char *packet)
{
	struct preset *preset;
	int x, nbytes, packet_len = 0, nr_presets;

	nr_presets = session->nr_presets;
	if (!nr_presets)
		return 0;
	for (x=0; x<nr_presets; x++) {
		preset = session->presets[x];
		if (!preset)
			continue;
		nbytes = sprintf(packet+packet_len, "preset %s %s@", preset->name, preset->indicators);
		packet_len += nbytes;
	}
	return (packet_len);
}

void session_load_presets(struct session *session)
{
	struct preset *preset;
	struct filemap filemap;
	char *map;
	char path[256];
	int filesize, fd, nr_presets, x;

	preset_path(session, path);
	map = MAP_FILE_RO(path, &filemap);
	if (!map) {
		session->nr_presets = 0;
		return;
	}
	nr_presets = (filemap.filesize/sizeof(struct preset));
	preset = (struct preset *)map;
	for (x=0; x<nr_presets; x++) {
		session->presets[x] = (struct preset *)malloc(sizeof(*preset));
		memcpy(session->presets[x], preset, sizeof(*preset));
		preset++;
	}
	session->nr_presets = nr_presets;
	UNMAP_FILE(map, &filemap);
}
/*
void market_update(struct session *session, struct XLS *XLS)
{
	struct session *session;
	int x, y;

	mutex_lock(&session_lock);
	DLIST_FOR_EACH_ENTRY(session, &session_list, list) {
		//printf("session: %p\n", session);
		mutex_lock(&session->session_lock);
		for (x=0; x<session->nr_watchlists; x++) {
			struct watchlist *watchlist = session->watchlists[x];
			for (y=0; y<watchlist->nr_stocks; y++) {
				struct watchstock *watchstock = &watchlist->stocks[y];
				watchstock->stock = search_stocks(watchstock->ticker);
				//printf(BOLDGREEN "replacing stock: %-8s new: %p" RESET "\n", watchstock->ticker, watchstock->stock);
			}
		}
		mutex_unlock(&session->session_lock);
	}
	mutex_unlock(&session_lock);
}*/

static void sessions_update_XLS(struct XLS *XLS)
{
	struct session   *session;
	struct list_head *session_list;
	int x, y;

	session_list = get_session_list();
	SESSION_LOCK();
	DLIST_FOR_EACH_ENTRY(session, session_list, list) {
		//printf("session: %p\n", session);
		mutex_lock(&session->session_lock);
		for (x=0; x<session->nr_watchlists; x++) {
			struct watchlist *watchlist = session->watchlists[x];
			for (y=0; y<watchlist->nr_stocks; y++) {
				struct watchstock *watchstock = &watchlist->stocks[y];
				watchstock->stock = search_stocks(watchstock->ticker);
				//printf(BOLDGREEN "replacing stock: %-8s new: %p" RESET "\n", watchstock->ticker, watchstock->stock);
			}
		}
		mutex_unlock(&session->session_lock);
	}
	SESSION_UNLOCK();
}

void rpc_search(struct rpc *rpc)
{
	char         *query = rpc->argv[1];
	struct stock *stock, *stocks;
	struct XLS   *XLS;
	char         *p;
	int           x, y, n, nr_stocks, packet_len, nr_results = 0, qlen = strlen(query);

	if (qlen > 6)
		return;

	strcpy(rpc->packet, "query ");
	packet_len = 6;

	XLS        = CURRENT_XLS;
	nr_stocks  = XLS->nr_stocks;
	stocks     = &XLS->STOCKS_ARRAY[0];
	for (x=0; x<nr_stocks; x++) {
		stock = &stocks[x];
		if (!strncmp(stock->sym, query, qlen)) {
			packet_len += snprintf(rpc->packet+packet_len, 256, "%s^%s#", stock->sym, stock->name);
			p = rpc->packet+packet_len-1;
			n = strlen(stock->name);
			nr_results++;
			for (y=0; y<n; y++) {
				if (*(p-y) == ' ')
					*(p-y) = '%';
			}
		}
	}
	if (!nr_results)
		return;
	websocket_send(rpc->connection, rpc->packet, packet_len-1);
}

void apc_update_EOD(struct connection *connection, char **argv)
{
	struct XLS *XLS;
	struct XLS *OLD_XLS = CURRENT_XLS;

	printf(BOLDWHITE "UPDATE_EOD(): ENTER" RESET "\n");
	XLS = load_stocks(XLS_DATA_SOURCE_WSJ, DATA_FORMAT_WEBSOCKET_INTERNAL);
	if (!XLS)
		return;
	init_ranks(XLS);
	printf(BOLDWHITE "UPDATE_EOD(): loaded stocks" RESET "\n");

	Server.stock_boot = 0;
	create_stock_threads(XLS);

	/* Kill old threads */
	for (int x=0; x<OLD_XLS->nr_stock_threads; x++)
		XLS->stock_threads[x].stop = 1;

	while (Server.stock_boot  != XLS->nr_stock_threads)  os_usleep(100000);
	printf(BOLDWHITE "UPDATE_EOD(): reloaded threads" RESET "\n");

	unlink(SIGNALS_DB);
	unlink(SIGNALS_CSV);
	init_anyday(XLS);
	init_BIX(XLS);
	init_monster(XLS, 0);
	printf(BOLDWHITE "UPDATE_EOD(): init_monster() done" RESET "\n");
	init_ufo(XLS);
	printf(BOLDWHITE "UPDATE_EOD(): init_ufo() done" RESET "\n");
//	process_mag3(XLS);

	// Switch to new XLS
	CURRENT_XLS = XLS;
	sessions_update_XLS(XLS);
	printf(BOLDWHITE "UPDATE_EOD(): session_update_XLS() done" RESET "\n");
	time_EOD();
	printf(BOLDGREEN "TIME EOD DONE" RESET "\n");
}

void stock_loop(struct server *config)
{
	struct stock   *stock;
	struct XLS     *XLS;
	struct board  **boards;
	double          current_price;
	time_t          timenow, timecheck = time(NULL);
	int             nr_stocks, x;

	while (1) {
		XLS = config->XLS;
		nr_stocks = XLS->nr_stocks;
		for (x=0; x<nr_stocks; x++) {
			stock  = &XLS->STOCKS_ARRAY[x];
			boards = XLS->boards;
			/* *****************************
			 *       Gainers/Losers
			 *******************************
			*/
			current_price = stock->current_price;
			if ((!current_price || (market == NO_MARKET)))// || (stock->type == STOCK_TYPE_HIGHCAPS && market != DAY_MARKET))
				continue;
			if (current_price == stock->prior_close) {
				stock->pr_percent = 0;
				continue;
			}
			if (current_price < stock->prior_close)
				stock->pr_percent = -((stock->prior_close-current_price)/stock->prior_close)*100;
			else
				stock->pr_percent = ((current_price-stock->prior_close)/stock->prior_close)*100;
//			if (DEBUG_STOCK && !strcmp(stock->sym, DEBUG_STOCK))
//				printf(BOLDBLUE "[%s] pr_percent: %.2f current_price: %.2f" RESET "\n", stock->sym, stock->pr_percent, current_price);
			if (stock->pr_percent > 0 && stock->pr_percent < 1600 && stock->current_price != 0.0)
				(stock->type == STOCK_TYPE_LOWCAPS) ? add_delta_gainer(stock, boards[LOWCAP_GAINER_BOARD]) : add_delta_gainer(stock, boards[HIGHCAP_GAINER_BOARD]);
			else if (stock->pr_percent < 0 && stock->pr_percent > -100.0 && stock->current_price != 0.0)
				(stock->type == STOCK_TYPE_LOWCAPS) ? add_delta_loser (stock, boards[LOWCAP_LOSER_BOARD])  : add_delta_loser (stock, boards[HIGHCAP_LOSER_BOARD]);			
		}

		timenow = time(NULL);
		if (timenow-timecheck > 1) {
			timecheck = timenow;
			market    = get_time();
		}
		if ((timenow - current_minute) >= 60) {
			current_minute     = timenow;
			current_timestamp += 60000;
			ufo_scan(XLS);
/*			if (AFH_START && !AFH_DONE) {
				reset_price_volume();
				AFH_START = 0;
				AFH_DONE = 1;
			}*/
		}
		os_usleep(500000);
	}
}


void setDataSource(struct XLS *XLS, int data_source, int network_protocol)
{
	switch (data_source) {
		case XLS_DATA_SOURCE_WSJ:
			load_WSJ(XLS);
			break;
	}
	XLS->data_source      = network_protocol;
	XLS->network_protocol = data_source;
}

#define STOCKS_TICKER_IDX   0
#define STOCKS_TYPE_IDX     1
#define STOCKS_COUNTRY_IDX  2
#define STOCKS_EXCHANGE_IDX 3
#define STOCKS_MCAP_IDX     4
#define STOCKS_NAME_IDX     5
#define STOCKS_SECTOR_IDX   6
#define STOCKS_INDUSTRY_IDX 7

struct XLS *load_stocks(int data_source, int data_format)
{
	struct XLS   *XLS = (struct XLS *)zmalloc(sizeof(struct XLS));
	struct stock *stock;
	char         *line_argv[8];
	char         *line, *p, *ticker, *stock_type, *market_cap, *ticker_path = NULL;
	int           x, argc, count = 0, nr_stocks = 0;

	if (!XLS)
		return NULL;

	MENU_STOCKS_PTR = (struct stock **)malloc(NR_MENU_STOCKS * sizeof(struct stock *));
	if (!MENU_STOCKS_PTR)
		return NULL;

	if (!config_get("ticker_path", CONF_TYPE_STR, &ticker_path, NULL))
		return NULL;
	XLS->tickers = fs_mallocfile_str(ticker_path?ticker_path:(char *)"data/stocks/STOCKS.TXT", NULL);
	if (!XLS->tickers)
		return NULL;

	XLS->nr_stocks = nr_stocks = cstring_line_count(XLS->tickers);
	if (!nr_stocks || nr_stocks >= 8191) {
		printf(BOLDRED "load_stocks() nr_stocks (%d) too large - increase size and recompile" RESET "\n", nr_stocks);
		return NULL;
	}

	// Some lines may have errors so we don't dynamically preallocate HIGHCAPS/LOWCAPS until we parse the entire ticker list
	char         *lines   [nr_stocks];
	struct stock *HIGHCAPS[nr_stocks];
	struct stock *LOWCAPS [nr_stocks];

	if (cstring_split(XLS->tickers, lines, 8191, '\n') != nr_stocks) {
		printf(BOLDRED "load_stocks(): stocks.txt error: nr_stocks: %d" RESET "\n", nr_stocks);
		return NULL;
	}

	XLS->STOCKS_ARRAY = (struct stock  *)zmalloc(sizeof(struct stock)   * nr_stocks);
	XLS->STOCKS_PTR   = (struct stock **)malloc (sizeof(struct stock *) * nr_stocks);
	XLS->STOCKS_STR   = (char         **)malloc (sizeof(char *)         * nr_stocks);

	for (x=0; x<nr_stocks; x++) {
		memset(line_argv, 0, sizeof(void *) * 8);
		argc = cstring_split(lines[x], line_argv, 8, ',');
		if (argc <= 0 || argc > 8)
			continue;

		stock              = &XLS->STOCKS_ARRAY[x];
		XLS->STOCKS_PTR[x] = stock;

		// "AAPL,STOCK,US,NASDAQ,H|L,Apple,Tech,InfoTech", stock->sym, security_type, stock->exchange_str, Highcaps/Lowcaps, stock->name, stock->sector, stock->industry);
		//   0    1    2    3    4   5     6    7
		stock->sym         = line_argv[STOCKS_TICKER_IDX];
		market_cap         = line_argv[STOCKS_MCAP_IDX];
		stock_type         = line_argv[STOCKS_TYPE_IDX];
		XLS->STOCKS_STR[x] = stock->sym;
		if (!strcmp(stock_type, "STOCK")) {
			if (market_cap) {
				if (*market_cap == 'H') {
					stock->type = STOCK_TYPE_HIGHCAPS;
					HIGHCAPS[XLS->nr_highcaps++] = &XLS->STOCKS_ARRAY[x];
				} else {
					stock->type = STOCK_TYPE_LOWCAPS;
					LOWCAPS[XLS->nr_lowcaps++]   = &XLS->STOCKS_ARRAY[x];
				}
			}
		} else if (!strcmp(stock_type, "CRYPTO")) {
			stock->type = STOCK_TYPE_CRYPTO;
			XLS->CRYPTO[XLS->nr_crypto++] = stock;
		} else if (!strcmp(stock_type, "FOREX")) {
			stock->type = STOCK_TYPE_FOREX;
			XLS->nr_forex++;
		} else if (!strcmp(stock_type, "INDEX")) {
			stock->type = STOCK_TYPE_INDEX;
		}

		stock->exchange_str = line_argv[STOCKS_EXCHANGE_IDX];
		stock->country      = line_argv[STOCKS_COUNTRY_IDX];
		stock->sector       = line_argv[STOCKS_SECTOR_IDX];
		stock->industry     = line_argv[STOCKS_INDUSTRY_IDX];
		stock->name         = line_argv[STOCKS_NAME_IDX];
		if (!strcmp(stock->exchange_str, "NASDAQ"))
			stock->market   = MARKET_NASDAQ;
		else if (!strcmp(stock->exchange_str, "NYSE"))
			stock->market   = MARKET_NYSE;
		else if (!strcmp(stock->exchange_str, "ASE"))
			stock->market   = MARKET_ASE;
		else
			stock->market   = MARKET_OTC;
	}

	XLS->HIGHCAPS = (struct stock **)malloc(XLS->nr_highcaps * sizeof(struct stock *));
	XLS->LOWCAPS  = (struct stock **)malloc(XLS->nr_lowcaps  * sizeof(struct stock *));

	memcpy(XLS->HIGHCAPS, HIGHCAPS, XLS->nr_highcaps * sizeof(struct stock *));
	memcpy(XLS->LOWCAPS,  LOWCAPS,  XLS->nr_lowcaps  * sizeof(struct stock *));
	init_ticker_hashtable(XLS);
	setDataSource(XLS, data_source, data_format);
	init_wtable();
	return (XLS);
}
