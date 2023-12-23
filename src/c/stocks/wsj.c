#include <conf.h>
#include <stocks/stocks.h>
#include <extern.h>

/*
 * GSPC: INDEX%2FUS%2FS%26P%20US%2FSPX             INDEX/US/S&P US/SPX              SPX
 * IXIC: INDEX%2FUS%2FXNAS%2FNDX                   INDEX/US/XNAS/NDX                NDX
 * DJI:  INDEX%2FUS%2FDOW%20JONES%20GLOBAL%2FDJIA  INDEX/US/DOW JONES GLOBAL/DJIA   DJIA
 * RUT:  INDEX%2FUS%2F%2FRUT                       INDEX/US/RUT                     RUT
 * SPY:  FUND%2FUS%2FARCX%2FSPY                    FUND/US/ARCX/SPY                 SPY
 * XLB:  FUND%2FUS%2FARCX%2FXLB                    FUND/US/ARCX/XLB                 XLB
 * XLC:  FUND%2FUS%2FARCX%2FXLC                    FUND/US/ARCX/XLC                 XLC
 * XLE:  FUND%2FUS%2FARCX%2FXLE                    FUND/US/ARCX/XLE                 XLE
 * XLF:  FUND%2FUS%2FARCX%2FXLF                    FUND/US/ARCX/XLF                 XLF
 * XLI:  FUND%2FUS%2FARCX%2FXLI                    FUND/US/ARCX/XLI                 XLI
 * XLK:  FUND%2FUS%2FARCX%2FXLK                    FUND/US/ARCX/XLK                 XLK
 * XLP:  FUND%2FUS%2FARCX%2FXLP                    FUND/US/ARCX/XLP                 XLP
 * XLU:  FUND%2FUS%2FARCX%2FXLU                    FUND/US/ARCX/XLU                 XLU
 * XLV:  FUND%2FUS%2FARCX%2FXLV                    FUND/US/ARCX/XLV                 XLV
 * XLY:  FUND%2FUS%2FARCX%2FXLY                    FUND/US/ARCX/XLY                 XLY
 */
char  *WSJ_US_IDX_SPX[] = { WSJ_SPX_CLO,    WSJ_SPX_CLO2,   WSJ_SPX_ALL,    WSJ_SPX_CUR     };
char  *WSJ_US_IDX_DOW[] = { WSJ_DOW_CLO,    WSJ_DOW_CLO2,   WSJ_DOW_ALL,    WSJ_DOW_CUR     };
char  *WSJ_US_IDX_NDX[] = { WSJ_NDX_CLO,    WSJ_NDX_CLO2,   WSJ_NDX_ALL,    WSJ_NDX_CUR     };
char  *WSJ_US_IDX_RUT[] = { WSJ_RUT_CLO,    WSJ_RUT_CLO2,   WSJ_RUT_ALL,    WSJ_RUT_CUR     };
char **WSJ_INDEXES[]    = { WSJ_US_IDX_SPX, WSJ_US_IDX_NDX, WSJ_US_IDX_DOW, WSJ_US_IDX_RUT  };

void wsj_stock_api(struct stock *stock)
{
	struct WSJ  *WSJ;
	char        *wsj_api_src,**wsj_api_dst,   *ticker,     *buf;
	int          wsj_api_size, type, exchange, ticker_size, total_size, x;

	WSJ         = &stock->API.WSJ;
	ticker      = stock->sym;
	exchange    = stock->exchange;
	ticker_size = strlen(ticker);
	type        = stock->type;
	if (ticker_size > 7)
		return;

	for (x=0; x<4; x++) {
		switch (x) {
			case WSJ_API_STOCK_CLOSE:
				wsj_api_src  = WSJ_CLOSE;
				wsj_api_dst  = &WSJ->CLOSE;
				wsj_api_size = sizeof(WSJ_CLOSE)-1;
				break;
			case WSJ_API_STOCK_CLOSE2:
				wsj_api_src  = WSJ_CLOSE2;
				wsj_api_dst  = &WSJ->CLOSE2;
				wsj_api_size = sizeof(WSJ_CLOSE2)-1;
				break;
			case WSJ_API_STOCK_ALLDAY:
				wsj_api_src  = WSJ_ALL;
				wsj_api_dst  = &WSJ->ALL;
				wsj_api_size = sizeof(WSJ_ALL)-1;
				break;
			case WSJ_API_STOCK_CURTICK:
				wsj_api_src  = WSJ_CUR;
				wsj_api_dst  = &WSJ->CUR;
				wsj_api_size = sizeof(WSJ_CUR)-1;
				break;
		}
		total_size   = wsj_api_size + ticker_size + sizeof(WSJ_OHLC)-1;// + sizeof(WSJ_HDR)-1;
		*wsj_api_dst = buf = (char *)malloc(total_size+1);
		strcpy(buf, wsj_api_src);
		if (exchange == MARKET_NYSE) {
			buf[wsj_api_size-6] = 'Y';
			buf[wsj_api_size-5] = 'S';
		} else if (exchange == MARKET_OTC) {
			buf[wsj_api_size-8] = 'O';
			buf[wsj_api_size-7] = 'O';
			buf[wsj_api_size-6] = 'T';
			buf[wsj_api_size-5] = 'C';
		} else if (exchange == MARKET_ASE) {
			buf[wsj_api_size-7] = 'A';
			buf[wsj_api_size-6] = 'S';
			buf[wsj_api_size-5] = 'E';
		}
		strcat(buf+wsj_api_size, ticker);
		memcpy(buf+wsj_api_size+ticker_size,                    WSJ_OHLC, sizeof(WSJ_OHLC));
//		memcpy(buf+wsj_api_size+ticker_size+sizeof(WSJ_OHLC)-1, WSJ_HDR,  sizeof(WSJ_HDR)-1);
	}
}

void wsj_crypto_api(struct stock *stock)
{
	struct WSJ  *WSJ;
	char         cpair[16];
	char        *wsj_api_src,**wsj_api_dst, *ticker, *buf, *p;
	int          wsj_api_size, type, exchange, ticker_size = 0, total_size, x = 0;

	ticker = stock->sym;
	if (strlen(ticker) > 15)
		return;

	while (*ticker != '\0') {
		if (*ticker == '-') {
			ticker++;
			continue;
		}
		cpair[ticker_size++] = *ticker++;
	}
	ticker = cpair;

	WSJ         = &stock->API.WSJ;
	exchange    = stock->exchange;
	type        = stock->type;

	for (x=0; x<4; x++) {
		switch (x) {
			case WSJ_API_STOCK_CLOSE:
				wsj_api_src  = WSJ_CLOSE;
				wsj_api_dst  = &WSJ->CLOSE;
				wsj_api_size = sizeof(WSJ_CLOSE)-1;
				break;
			case WSJ_API_STOCK_CLOSE2:
				wsj_api_src  = WSJ_CLOSE2;
				wsj_api_dst  = &WSJ->CLOSE2;
				wsj_api_size = sizeof(WSJ_CLOSE2)-1;
				break;
			case WSJ_API_STOCK_ALLDAY:
				wsj_api_src  = WSJ_CRYPTO_1M;
				wsj_api_dst  = &WSJ->ALL;
				wsj_api_size = sizeof(WSJ_CRYPTO_1M)-1;
				break;
			case WSJ_API_STOCK_CURTICK:
				wsj_api_src  = WSJ_CUR;
				wsj_api_dst  = &WSJ->CLOSE;
				wsj_api_size = sizeof(WSJ_CUR)-1;
				break;
		}
		total_size   = wsj_api_size + ticker_size + sizeof(WSJ_OHLC)-1 + sizeof(WSJ_HDR)-1;
		*wsj_api_dst = buf = (char *)malloc(total_size+1);
		strcpy(buf,wsj_api_src);
		strcat(buf+wsj_api_size, ticker);
		memcpy(buf+wsj_api_size+ticker_size,                    WSJ_OHLC, sizeof(WSJ_OHLC));
		memcpy(buf+wsj_api_size+ticker_size+sizeof(WSJ_OHLC)-1, WSJ_HDR,  sizeof(WSJ_HDR)-1);
	}
}
/*
GET /api/michelangelo/timeseries/history?json=%7B%22Step%22%3A%22PT1M%22%2C%22TimeFrame%22%3A%22D1%22%2C%22EntitlementToken%22%3A%2257494d5ed7ad44af85bc59a51dd87c90%22%2C%22IncludeMockTick%22%3Atrue%2C%22FilterNullSlots%22%3Atrue%2C%22FilterClosedPoints%22%3Atrue%2C%22IncludeClosedSlots%22%3Afalse%2C%22IncludeOfficialClose%22%3Atrue%2C%22InjectOpen%22%3Afalse%2C%22ShowPreMarket%22%3Afalse%2C%22ShowAfterHours%22%3Afalse%2C%22UseExtendedTimeFrame%22%3Atrue%2C%22WantPriorClose%22%3Afalse%2C%22IncludeCurrentQuotes%22%3Afalse%2C%22ResetTodaysAfterHoursPercentChange%22%3Afalse%2C%22Series%22%3A%5B%7B%22Key%22%3A%22CRYPTOCURRENCY%2FUS%2FCOINDESK%2FBTCUSD%22%2C%22Dialect%22%3A%22Charting%22%2C%22Kind%22%3A%22Ticker%22%2C%22SeriesId%22%3A%22s1%22%2C%22DataTypes%22%3A%5B%22Open%22%2C%22High%22%2C%22Low%22%2C%22Last%22%5D%2C%22Indicators%22%3A%5B%7B%22Parameters%22%3A%5B%5D%2C%22Kind%22%3A%22Volume%22%2C%22SeriesId%22%3A%22i3%22%7D%5D%7D%5D%7D&ckey=57494d5ed7 HTTP/1.1
*/

void wsj_index_api(struct stock *stock)
{
	struct WSJ  *WSJ;
	char        *wsj_api_src, **wsj_api_dst,  *ticker,     *buf;
	int          wsj_api_size, type, exchange, ticker_size, total_size, x, y;

	return;
	for (x=0; x<NR_GLOBAL_INDEXES; x++) {
		switch (x) {
			case WSJ_US_INDEX_SPX: {
				for (y=0; y<4; y++) {
					switch (y) {
						case WSJ_API_INDEX_CLOSE:
							wsj_api_src     = WSJ_INDEXES[x][0];
							wsj_api_dst     = &WSJ->CLOSE;
							wsj_api_size    = sizeof(WSJ_SPX_CLO)-1;
							break;
						case WSJ_API_INDEX_CLOSE2:
							wsj_api_src     = WSJ_INDEXES[x][2];
							wsj_api_dst     = &WSJ->CLOSE2;
							wsj_api_size    = sizeof(WSJ_SPX_CLO2)-1;
							break;
						case WSJ_API_INDEX_ALLDAY:
							wsj_api_src     = WSJ_INDEXES[x][3];
							wsj_api_dst     = &WSJ->ALL;
							wsj_api_size    = sizeof(WSJ_SPX_ALL)-1;
							break;
						case WSJ_API_INDEX_CURTICK:
							wsj_api_src     = WSJ_INDEXES[x][4];
							wsj_api_dst     = &WSJ->CUR;
							wsj_api_size    = sizeof(WSJ_SPX_CUR)-1;
							break;
					}
				}
			};
			case WSJ_US_INDEX_DOW: {
				for (y=0; y<4; y++) {
					switch (y) {
						case WSJ_API_INDEX_CLOSE:
							wsj_api_src     = WSJ_INDEXES[x][0];
							wsj_api_dst     = &WSJ->CLOSE;
							wsj_api_size    = sizeof(WSJ_DOW_CLO)-1;
							break;
						case WSJ_API_INDEX_CLOSE2:
							wsj_api_src     = WSJ_INDEXES[x][1];
							wsj_api_dst     = &WSJ->CLOSE2;
							wsj_api_size    = sizeof(WSJ_DOW_CLO2)-1;
							break;
						case WSJ_API_INDEX_ALLDAY:
							wsj_api_src     = WSJ_INDEXES[x][2];
							wsj_api_dst     = &WSJ->ALL;
							wsj_api_size    = sizeof(WSJ_DOW_ALL)-1;
							break;
						case WSJ_API_INDEX_CURTICK:
							wsj_api_src     = WSJ_INDEXES[x][3];
							wsj_api_dst     = &WSJ->CUR;
							wsj_api_size    = sizeof(WSJ_DOW_CUR)-1;
							break;
					}
				}
			};
			case WSJ_US_INDEX_NDX: {
				for (y=0; y<4; y++) {
					switch (y) {
						case WSJ_API_INDEX_CLOSE:
							wsj_api_src     = WSJ_INDEXES[x][0];
							wsj_api_dst     = &WSJ->CLOSE;
							wsj_api_size    = sizeof(WSJ_NDX_CLO)-1;
							break;
						case WSJ_API_INDEX_CLOSE2:
							wsj_api_src     = WSJ_INDEXES[x][1];
							wsj_api_dst     = &WSJ->CLOSE2;
							wsj_api_size    = sizeof(WSJ_NDX_CLO2)-1;
							break;
						case WSJ_API_INDEX_ALLDAY:
							wsj_api_src     = WSJ_INDEXES[x][2];
							wsj_api_dst     = &WSJ->ALL;
							wsj_api_size    = sizeof(WSJ_NDX_ALL)-1;
							break;
						case WSJ_API_INDEX_CURTICK:
							wsj_api_src     = WSJ_INDEXES[x][3];
							wsj_api_dst     = &WSJ->CUR;
							wsj_api_size    = sizeof(WSJ_NDX_CUR)-1;
							break;
					}
				}
			};
			case WSJ_US_INDEX_RUT: {
				for (y=0; y<4; y++) {
					switch (y) {
						case WSJ_API_INDEX_CLOSE:
							wsj_api_src     = WSJ_INDEXES[x][0];
							wsj_api_dst     = &WSJ->CLOSE;
							wsj_api_size    = sizeof(WSJ_RUT_CLO)-1;
							break;
						case WSJ_API_INDEX_CLOSE2:
							wsj_api_src     = WSJ_INDEXES[x][1];
							wsj_api_dst     = &WSJ->CLOSE2;
							wsj_api_size    = sizeof(WSJ_RUT_CLO2)-1;
							break;
						case WSJ_API_INDEX_ALLDAY:
							wsj_api_src     = WSJ_INDEXES[x][2];
							wsj_api_dst     = &WSJ->ALL;
							wsj_api_size    = sizeof(WSJ_RUT_ALL)-1;
							break;
						case WSJ_API_INDEX_CURTICK:
							wsj_api_src     = WSJ_INDEXES[x][3];
							wsj_api_dst     = &WSJ->CUR;
							wsj_api_size    = sizeof(WSJ_RUT_CUR)-1;
							break;
					}
				}
			};
		} // switch
	} // for

	wsj_api_size += sizeof(WSJ_HDR)-1;
	*wsj_api_dst  = buf = (char *)malloc(wsj_api_size+1);
	strcpy(buf, wsj_api_src);
	strcat(buf, WSJ_HDR);
}

void load_WSJ(struct XLS *XLS)
{
	struct stock *stock, **STOCKS;
	int           nr_stocks, type, x;

	STOCKS    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = STOCKS[x];
		switch (stock->type) {
			case STOCK_TYPE_STOCK:
				wsj_stock_api(stock);
				break;
			case STOCK_TYPE_INDEX:
				wsj_index_api(stock);
				break;
			case STOCK_TYPE_FUND:
				break;
			case STOCK_TYPE_CRYPTO:
				wsj_crypto_api(stock);
				break;
		}
	}
}

/* 
 * WSJ EXTRACTORS
 *
 */
/* Get Last Timestamp: WSJ_CUR */
time_t get_last_tick(char *page, char **tptr)
{
	char  *ticks = strstr(page, "]},\"S");
	time_t timestamp;

	if (!ticks) return 0;
	ticks -= 13;
	if (*ticks == ']') return 0;
	timestamp = strtoul(ticks, NULL, 10);
	*tptr = ticks;
	return (timestamp);
}

/* Get Last OHLC: WSJ_CUR */
char *get_last_OHLC(double *xopen, double *xhigh, double *xlow, double *xclose, char *page)
{
	char *p = strstr(page, "FormatHints");
	if (!p) return NULL;
	while (*p != '[') p--;

	*xopen = strtod(p+1, NULL);
	while (*p != ',') p++; p += 1; *xhigh  = strtod(p, NULL);
	while (*p != ',') p++; p += 1; *xlow   = strtod(p, NULL);
	while (*p != ',') p++; p += 1; *xclose = strtod(p, NULL);
	return (p);
}

/* Get Last Volume: WSJ_CUR */
uint64_t get_last_volume(char *page)
{
	char *p = strstr(page, "Volume");
	if (!p) return 0;
	p = strstr(p+64, "FormatHints");
	if (!p) return 0;
	while (*p != '[') p--;
	return strtoul(p+1, NULL, 10);
}

/* Get Timestamps: WSJ_ALL */
int get_timestamps(char *page, time_t *timestamps)
{
	char *ticks, *p;
	int nr_timestamps = 0;

	ticks     = strstr(page, "Ticks");
	if (!ticks)
		return 0;
	ticks += 8;
	if (*ticks == ']')
		return 0;
	/* TIMESTAMPS */
	while ((p=strchr(ticks, ','))) {
		timestamps[nr_timestamps++] = strtoul(ticks, NULL, 10);
		if (*(p-1) == '}')
			break;
		ticks = p + 1;
	}
	return (nr_timestamps);
}

/* [WSJ EOD CLOSE API]
 - Example for the AAPL "close" price that can be used after EOD to get the previous day's OHLC bar, so it can be appended to its respective data/stocks/stockdb/csv ticker
 {"TimeInfo":{"TickCount":1,"FirstTick":1703203200000,"LastTick":1703203200000,"IsIntraday":false,"Step":86400000,
  "TimeFrameStart":1703255400000,"TimeFrameEnd":1703278740000,"Ticks":[1703203200000]},
  "Series":[{"SeriesId":"s1","ResponseId":"s1-1","DesiredDataPoints":["Open","High","Low","Last"],"ToolTipDataPoints":[],
  "Ticker":"AAPL","CountryCode":"US","CommonName":"Apple Inc.","OfficialId":"STOCK-US-AAPL","BlueGrassChannels":[{"ChannelType":"DelayedChannel","ChannelName":"/zigman2/quotes/202934861/delayed"},{"ChannelType":"CompositeChannel","ChannelName":"/zigman2/quotes/202934861/composite"},{"ChannelType":"RealtimeChannel","ChannelName":"/zigman2/quotes/202934861/lastsale"}],"UtcOffset":-300,"TimeZoneAbbreviation":"ET",
  "DataPoints":[[195.18,195.41,192.97,193.6]],"FormatHints":{"UnitSymbol":"$","Suffix":false,"DecimalPlaces":4},"MarketSessions":null,
  "ExtraData":
     [{"Name":"MostRecentOpen","Date":1703203200000,"Value":195.18},
     {"Name":"MostRecentLast", "Date":1703203200000,"Value":193.6},
     {"Name":"PriorClose",     "Date":1703116800000,"Value":194.68}],
  "TimeZoneData":{"SwitchDateUtc":1699167600000,"UtcOffsetBeforeSwitch":-240,"UtcOffsetAfterSwitch":-300,"IsDstBeforeSwitch":true,"IsDstAfterSwitch":false,"StandardAbbreviation":"EST","DaylightAbbreviation":"EDT","ShortAbbreviation":"ET"},"ExtraToolTips":null,"InstrumentType":"Stock","DjId":"13-3122","CurrentQuote":null},{"SeriesId":"i3","ResponseId":"i3-2","DesiredDataPoints":["Volume"],"ToolTipDataPoints":[],"Ticker":null,"CountryCode":null,"CommonName":null,"OfficialId":null,"BlueGrassChannels":[],"UtcOffset":0,"TimeZoneAbbreviation":null,"DataPoints":[[37149566.0]],"FormatHints":{"UnitSymbol":null,"Suffix":false,"DecimalPlaces":0},"MarketSessions":null,"ExtraData":null,"TimeZoneData":null,"ExtraToolTips":null,"InstrumentType":null,"DjId":null,"CurrentQuote":null}],"Events":[],"Aggregates":[],"Debug":null}
*/
void wsj_get_EOD(struct stock *stock, char *page)
{
	struct stat sb;
	char        path[256];
	char       *csv, *p;
	double      csv_open, csv_high, csv_low, csv_close;
	uint64_t    csv_volume;
	time_t      start_timestamp;
	int fd,     csv_size;

	p = strstr(page, "Points\":[[");
	if (!p || !page)
		return;
	p += 10;
	csv_open = strtod(p, NULL);
	p = strchr(p, ',');
	if (!p)
		return;
	csv_high = strtod(p+1, NULL);
	p = strchr(p+1, ',');
	if (!p)
		return;
	csv_low = strtod(p+1, NULL);
	p = strchr(p+1, ',');
	if (!p)
		return;
	csv_close = strtod(p+1, NULL);
	p = strstr(p+1, ":[[");
	if (!p)
		return;
	csv_volume = strtoul(p+3, NULL, 10);

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/csv/%s.csv", stock->sym);
	fd = open(path, O_RDWR);
	if (fd == -1)
		return;
	fstat(fd, &sb);
	csv = (char *)malloc(sb.st_size+260);
	if (!csv)
		return;
	read(fd, csv, sb.st_size);

	csv_size  = cstring_remove_line(csv, 1, sb.st_size);
	csv_size += snprintf(csv+csv_size, 256, "%s,%.3f,%.3f,%.3f,%.3f,%llu\n", stock->market->prev_eod_timestamp, csv_open, csv_high, csv_low, csv_close, csv_volume);
	lseek(fd, 0, SEEK_SET);
	write(fd, csv, csv_size);
	ftruncate(fd, csv_size);
	close(fd);
}

void apc_update_WSJ(struct connection *connection, char **argv)
{
	struct stock *stock, *stocks;
	struct WSJ   *WSJ;
	struct XLS   *XLS = CURRENT_XLS;
	char          page[128 KB];
	int           nr_stocks;

	stocks    = XLS->STOCKS_ARRAY;
	nr_stocks = XLS->nr_stocks;
	for (int x=0; x<nr_stocks; x++) {
		stock = &stocks[x];
		WSJ   = &stock->API.WSJ;
		if (!WSJ || !WSJ->CLOSE)
			continue;
		wsj_query(stock, WSJ->CLOSE, strlen(WSJ->CLOSE), page, wsj_get_EOD);
	}
}

void wsj_update_stock(char *ticker)
{
	struct WSJ *WSJ;
	char page[128 KB];
	struct stock *stock = search_stocks(ticker);

	if (!stock)
		return;

	WSJ = &stock->API.WSJ;
	wsj_query(stock, WSJ->CLOSE, strlen(WSJ->CLOSE), page, wsj_get_EOD);
}

/*
 * Load WSJ's 1m OHLC+volume data for today (partial|complete) (or for the previous day: complete)
 */
int wsj_load_1m(struct stock *stock)
{
	struct price   *price;
	struct stat     sb;
	struct filemap  filemap;
	char           *line, *map, *p;
	char            path[256];
	int             nr_ohlc = 0, mini_len = 0, nbytes, filesize;

	if (stock->dead)
		return 0;
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/wsj/%s", stock->sym);
	if (!(map=MAP_FILE_RO(path, &filemap)))
		return 0;
	price               = stock->price;
	filesize            = filemap.filesize;
	price->price_1m_len = filesize;
	price->price_1m     = (char *)malloc(filesize+1);
	memcpy(price->price_1m, map, filesize);
	price->price_1m[filesize] = 0;
	UNMAP_FILE(map, &filemap);

	line = price->price_1m+1;
	do {
		stock->ohlc[nr_ohlc].timestamp = strtoul(line+1, NULL, 10);
		p = strchr(line, ',');
		if (!p) break;
		stock->ohlc[nr_ohlc].open   = strtod(p+1, NULL);
		p = strchr(p+1, ',');
		if (!p) break;
		stock->ohlc[nr_ohlc].high   = strtod(p+1, NULL);
		p = strchr(p+1, ',');
		if (!p) break;
		stock->ohlc[nr_ohlc].low    = strtod(p+1, NULL);
		p = strchr(p+1, ',');
		if (!p) break;
		stock->ohlc[nr_ohlc].close  = strtod(p+1, NULL);
		p = strchr(p+1, ',');
		if (!p) break;
		stock->ohlc[nr_ohlc].volume = strtoul(p+1, NULL, 10);

		if (nr_ohlc == 0)
			nbytes    = sprintf(price->price_mini+mini_len, "[[%lu,%.2f],", stock->ohlc[nr_ohlc].timestamp, stock->ohlc[nr_ohlc].close);
		else
			nbytes    = sprintf(price->price_mini+mini_len, "[%lu,%.2f],",  stock->ohlc[nr_ohlc].timestamp, stock->ohlc[nr_ohlc].close);
		mini_len += nbytes;
		p = strchr(p+1, ']');
		if (!p) break;
		line = p + 2;
		nr_ohlc += 1;
	} while (*(p-1) != ']');
	if (nr_ohlc)
		*(price->price_mini+mini_len-1) = ']';
	price->price_mini_len = mini_len;
	stock->nr_ohlc = nr_ohlc;

	/* Append the 1m data after the 1d data */
	price->price_1d[price->price_1d_len-1] = ',';
	memcpy(price->price_1d+price->price_1d_len, price->price_1m+1, price->price_1m_len-1);
	price->price_1d_len = price->price_1m_len+price->price_1d_len-1;
	*(price->price_1d+price->price_1d_len) = ']';
	if (price->price_1d_len >= 256 KB) {printf(BOLDRED "CRITICAL ERROR PRICE: %d" RESET "\n", price->price_1d_len);exit(-1);}
	return 1;
}

/*
 * Fill in stock->ohlc struct array with today's trading ticks - 1M OHLCvs since the trading period began
 *  - set the "current" OHLCv into stock->current_open|current_high|current_low|current_close|current_volume
 */
void update_allday(struct stock *stock, char *page)
{
	struct ohlc  *ohlc;
	struct price *price;
	char         *jptr, *mptr, *p;
	char          path[64];
	int           json_size = 0, mini_size = 0, nr_timestamps = 0, nr_ohlc = 0, nr_volume = 0;
	int           nr_trading_minutes, nbytes, fd, x;
	double        o,h,l,c = 0.0;
	time_t        timestamp, vol;

	if ((nr_trading_minutes=market_get_nr_minutes(stock)) <= 0 || (nr_trading_minutes > 1440))
		return;

	uint64_t      volume    [nr_trading_minutes];
	time_t        timestamps[nr_trading_minutes];

	price     = stock->price;
	jptr      = price->price_1m;   *jptr++   = '[';
	mptr      = price->price_mini; *mptr++   = '[';
	mini_size = 1; json_size = 1;

	/* TIMESTAMPS */
	if (!(nr_timestamps=get_timestamps(page, timestamps)))
		return;

	/* VOLUME */
	p = strstr(page, "Volume");
	if (!p) return;
	p    = strstr(p, "DataPoints\":[[");
	if (!p) return;
	for (x=0; x<nr_timestamps; x++) {
		if (*p == '0') {
			volume[nr_volume++] = 0;
			p += 6;
			continue;
		}
		vol = strtoul(p, NULL, 10);
		if (market == DAY_MARKET || market == AFH_MARKET)
			stock->day_volume += vol;
		volume[nr_volume++] = vol;
		p = strchr(p, ']');
		if (!p || *(p+1) == ']')
			break;
		p += 3;
	}

	/* OHLC PRICES */
	p = strstr(page, "DataPoints\":[[");
	if (!p) return;
	p += 14;
	ohlc = &stock->ohlc[0];
	for (x=0; x<nr_timestamps; x++) {
		o = strtod(p, NULL);
		while (*p != ',') p++; p += 1; h = strtod(p, NULL);
		while (*p != ',') p++; p += 1; l = strtod(p, NULL);
		while (*p != ',') p++; p += 1; c = strtod(p, NULL);
		timestamp   = timestamps[x];
		nbytes      = sprintf(jptr, "[%lu,%.2f,%.2f,%.2f,%.2f,%llu],", timestamp, o, h, l, c, volume[x]);
		jptr       += nbytes;
		json_size  += nbytes;
		nbytes      = sprintf(price->price_mini+mini_size, "[%lu,%.2f],", timestamp, c);
		mini_size  += nbytes;
		/* Intraday Low & Intraday High */
		if (l < stock->intraday_low)  stock->intraday_low  = l;
		if (h > stock->intraday_high) stock->intraday_high = h;
		ohlc->open = o; ohlc->high = h; ohlc->low = l;ohlc->close = c; ohlc->timestamp = timestamp;
		nr_ohlc++; ohlc++; p++;
//		printf("[%d] p: %.10s timestamp: %d open: %.2f high: %.2f low: %.2f close: %.2f\n", stock->nr_ohlc, p,  timestamp, o,h,l,c);
		while (*p != ',') p++;
		if (*p == ']' && *(p+1) == ']') break;
		p += 2;
	}
	stock->nr_ohlc = nr_ohlc;

	if (nr_volume != nr_timestamps || stock->nr_ohlc != nr_timestamps) {
		printf("corrupt WSJ data: nr_timestmaps: %d nr_ohlc: %d page: %s\n", nr_timestamps, stock->nr_ohlc, page);
		return;
	}

	ohlc = &stock->ohlc[nr_ohlc-1];
	stock->current_timestamp = timestamps[nr_timestamps-1];
	stock->price_open        = stock->ohlc[0].open;
	stock->current_volume    = ohlc->volume;
	stock->current_open      = ohlc->open;
	stock->current_high      = ohlc->high;
	stock->current_low       = ohlc->low;
	stock->current_close     = ohlc->close;
	stock->current_price     = ohlc->close;

	price->price_1m_len      = json_size;
	price->price_mini_len    = mini_size;
	*(price->price_1m+json_size-1)   = ']';
	*(price->price_mini+mini_size-1) = ']';

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/wsj/%s", stock->sym);
	fs_newfile(path, price->price_1m, price->price_1m_len);
}

/* Called once on INIT */
int WSJ_update_allday_price(struct stock *stock)
{
	struct WSJ   *WSJ = &stock->API.WSJ;
	struct price *price;
	char          page[96 KB];
	bool          loaded = false;

	if (Server.stocks_1M == STOCKDATA_OFF)
		return 0;

	/*
	 * - Loaded During NO_MARKET
	 *    + wsj_load_1m() of previous day's 1m OHLCvs
	 *    + set price->stale = 1
 	 */
	price = stock->price;
	if (stock->dead || !stock->API.WSJ.ALL)
		return 0;
	if (stock->market->status == NO_MARKET) {
		if (wsj_load_1m(stock))
			loaded = true;
		if (price->price_1d_len)
			*(price->price_1d+price->price_1d_len-1) = ']';
//		price->stale = 1; // when the next calendar trading day comes, a new price struct is allocated
	}
/*
	if (market == NO_MARKET) {
		if (!wsj_load_1m(stock) && price->price_1d_len)
			*(price->price_1d+price->price_1d_len-1) = ']';
		price->stale = 1;
		return 0;
	}*/
	if (loaded)
		return 0;

	if (price->stale) {
		struct price *newprice   = (struct price *)zmalloc(sizeof(struct price));
		newprice->price_1d       = (char *)malloc(256 KB);
		newprice->price_1m       = (char *)malloc(96 KB);
		newprice->price_mini     = (char *)malloc(32 KB);
		newprice->price_1d_close = (char *)malloc(96 KB);
		memcpy(newprice->price_1d, price->price_1d, (price->price_1d_len-price->price_1m_len));
		newprice->price_1d[price->price_1d_len++] = ',';
		newprice->price_1d_len   = (price->price_1d_len-price->price_1m_len);
		newprice->price_1m_len   = 0;
		newprice->nr_points_1d   = price->nr_points_1d;
		newprice->stale          = 0;
		stock->price             = newprice;
		price                    = newprice;
		Server.nr_working_stocks = 0;
		Server.nr_failed_stocks  = 0;
	}

	wsj_query(stock, WSJ->ALL, strlen(WSJ->ALL), page, update_allday);
	if (price->price_1m_len) {
		memcpy(price->price_1d+price->price_1d_len, price->price_1m+1, price->price_1m_len-1);
		price->price_1d_len = price->price_1m_len+price->price_1d_len-1;
		*(price->price_1d+price->price_1d_len) = ']';
		Server.nr_working_stocks++;
		stock->update = 1;
		return 1;
	} else {
		Server.nr_failed_stocks++;
		stock->update = 0;
		if (price->price_1d_len)
			*(price->price_1d+price->price_1d_len-1) = ']';
		return 0;
	}
}

/* 
 * Called every second to extract latest OHLC + Volume + Timestamp
 * 2) IF the latest Timestamp is the same as the previous tick AND it's still in the same minute, then update otherwise it's stale.
 */
void WSJ_update_current_tick(struct stock *stock, char *page)
{
	struct ohlc   *ohlc = &stock->ohlc[stock->nr_ohlc-1];
	char          *p    = page;
	double         current_open, current_high, current_low, current_close;
	uint64_t       current_volume;
	time_t         current_tick;
	unsigned short nr_ohlc = stock->nr_ohlc;

	if (Server.DEBUG_STOCK && !strcmp(stock->sym, Server.DEBUG_STOCK))
		printf(BOLDMAGENTA "[%s] update_current_tick: %s" RESET "\n", stock->sym, page);
	/* Current Timestamp */
	if (!(current_tick = get_last_tick(page, &p)))
		return;

	/* Current OHLC */
	if (!(p=get_last_OHLC(&current_open, &current_high, &current_low, &current_close, p)))
		return;

	/* Current Volume */
	current_volume = get_last_volume(p);

	/* If the latest Timestamp is newer than the previous tick then this is a new candle, store data in ohlc++ */
	if (current_tick > ohlc->timestamp) {
		ohlc++;
		addPoint(stock, current_tick, current_open, current_high, current_low, current_close, current_volume);
		nr_ohlc++;
	} else if (current_tick == ohlc->timestamp) {
 		if (ohlc->open != current_open || ohlc->high != current_high || ohlc->low != current_low || ohlc->close != current_close || ohlc->volume != current_volume) {
			stock->nr_updates += 1;
			if (!strcmp(stock->sym, "ZUO"))
				printf("stock added update open:[%.2f %.2f],high:[%.2f %.2f],low:[%.2f %.2f],close:[%.2f %.2f],volume:[%llu %llu]\n",
				ohlc->open, current_open, ohlc->high, current_high, ohlc->low, current_low, ohlc->close, current_close,ohlc->volume, current_volume);
		}
	}
	ohlc->open            = current_open;
	ohlc->high            = current_high;
	ohlc->low             = current_low;
	ohlc->close           = current_close;
	ohlc->timestamp       = current_tick;
	ohlc->volume          = current_volume;
	stock->nr_ohlc        = nr_ohlc;
	stock->current_volume = current_volume;
	stock->current_price  = current_close;
	/* intraday_low & intraday_high */
	if (stock->current_price < stock->intraday_low)  stock->intraday_low  = stock->current_price;
	if (stock->current_price > stock->intraday_high) stock->intraday_high = stock->current_price;
	return;
}

/*
 * Called by price.c::update_current_price() for live data via WSJ (when WSJ is enabled as a 1M data source for this instrument)
 */
void WSJ_update_current_price(struct stock *stock)
{
	char  page[96 KB];
	struct WSJ *WSJ = &stock->API.WSJ;	

	/*
	 * WSJ is only used for live stock prices (also some indexes and funds)
	 *  - it provides both 1D OHLC and 1Minute OHLC where polling every few
	 * seconds or even every second will provide an adjusted current OHLCv
	 * This is not optimal because thousands of GET requests are being made
	 * to WSJ and this is unsustainable and will have to be removed in the
	 * future. Right now it is only used for development purposes.
	 */
	if (Server.stocks_1M == STOCKDATA_OFF)
		return;
	/*
	 * If we haven't got the OHLCv ticks for today's trading day
	 * then there is no use in fetching the "current" OHLCv tick
	 * Try to fetch the 1M Daily data up until now a few more times
 	 */
	if (!stock->update) {
		if (stock->nr_data_retries++ < 2)
			WSJ_update_allday_price(stock);
		return;
	}

	wsj_query(stock, WSJ->CUR, strlen(WSJ->CUR), page, WSJ_update_current_tick);
}

int wsj_query(struct stock *stock, char *url, int url_size, char *uncompressed, void (*query)(struct stock *stock, char *page))
{
	if (curl_get(url, uncompressed))
		query(stock, uncompressed);
}

void argv_wsj_allday(char *ticker)
{
	holiday = 0;
	market  = DAY_MARKET;
	init_ip();

	CURRENT_XLS = load_stocks();
	for (int x=0; x<CURRENT_XLS->nr_stocks; x++) {
		struct stock *stock = &CURRENT_XLS->STOCKS_ARRAY[x];
		if (ticker && strcmp(stock->sym, ticker))
			continue;
		stock->ohlc                  = (struct ohlc  *)zmalloc(sizeof(struct ohlc) * TRADE_MINUTES);
		stock->price                 = (struct price *)zmalloc(sizeof(struct price));
		stock->price->price_1d       = (char *)malloc(256 KB);
		stock->price->price_1d_close = (char *)malloc(168 KB);
		stock->price->price_1m       = (char *)malloc(28 KB);
		stock->price->price_mini     = (char *)malloc(32 KB);
		WSJ_update_allday_price(stock);
	}
}
