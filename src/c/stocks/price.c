#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

int       get_timestamps (char *page, time_t *timestamps);
uint64_t  get_last_volume(char *page);
time_t    get_last_tick  (char *page, char **tptr);
char     *get_last_OHLC  (double *xopen, double *xhigh, double *xlow, double *xclose, char *page);

/* date format: 07/21/18 */
double price_by_date(struct stock *stock, char *date)
{
	int x, year, month, day, row, nr_entries, yr, mo;

	if (!date || !stock->mag)
		return 0.0;
	nr_entries = stock->mag->nr_entries;
	yr = year = atoi(date+6);
	switch (year) {
		case 18: year = stock->mag->year_2018; break;
		case 19: year = stock->mag->year_2019; break;
		case 20: year = stock->mag->year_2020; break;
		case 21: year = stock->mag->year_2021; break;
		default: return 0.0;
	}

	mo = month = atoi(date);
	if (!month)
		return 0.0;
	if (month <= 0 || month > 12)
		return 0.0;
	day   = atoi(date+3);
	if (day <= 0 || day > 31)
		return 0.0;
	row   = year+((month-1)*18);
//	printf("date: %s year: %d month: %d day: %d row: %d YEAR: %d\n", date, yr, mo, day, row, year);
	for (x=0; x<128; x++) {
		if (atoi(stock->mag->date[row]+8) == day && atoi(stock->mag->date[row]+5) == month)
			return stock->mag->close[row];
		if (++row >= nr_entries)
			return 0.0;
	}
	return 0.0;
}


/* 1 Minute has passed */
void addPoint(struct stock *stock, time_t new_tick, double new_open, double new_high, double new_low, double new_close, uint64_t new_volume)
{
	struct price *price;
	struct tick *tmp, *tick = stock->sparetick;

	price = stock->price;
	if (!price->price_1d)
		return;

	/* Candlestick Update */
	tick->current_ohlc_size = sprintf(tick->current_ohlc, "[%lu,%.2f,%.2f,%.2f,%.2f,%llu]", new_tick, new_open, new_high, new_low, new_close, new_volume);

	/* Update 1m OHLC that gets sent to browser on entry */
	if (price->price_1m_len) {
		*(price->price_1m+price->price_1m_len-1) = ',';
		strcpy(price->price_1m+price->price_1m_len, tick->current_ohlc);
		*(price->price_1m+price->price_1m_len+tick->current_ohlc_size) = ']';
		price->price_1m_len += tick->current_ohlc_size+1;
	}
	/* Update 1m OHLC that gets sent to browser on entry */
	if (*(price->price_1d+price->price_1d_len-1) == ']' && *(price->price_1d+price->price_1d_len-2) == ']') {
		*(price->price_1d+price->price_1d_len-1) = ',';
		strcpy(price->price_1d + price->price_1d_len, tick->current_ohlc);
		price->price_1d_len++;
		*(price->price_1d+price->price_1d_len+tick->current_ohlc_size-1) = ']';
		price->price_1d_len += tick->current_ohlc_size;
	} else {
		strcpy(price->price_1d+price->price_1d_len, tick->current_ohlc);
		*(price->price_1d+price->price_1d_len+tick->current_ohlc_size) = ']';
		*(price->price_1d+price->price_1d_len-1) = ',';
		*(price->price_1d+price->price_1d_len-1) = ',';
		price->price_1d_len += tick->current_ohlc_size+1;
	}

	/* Line Chart Update */
	tick->current_mini_size = sprintf(tick->current_mini, "[%lu,%.2f]", new_tick, new_close);
	tmp                 = stock->current_tick;
	stock->current_tick = tick;
	stock->sparetick    = tmp;
}

/* 
 * Called every second to extract latest OHLC + Volume + Timestamp
 * 2) IF the latest Timestamp is the same as the previous tick AND it's still in the same minute, then update otherwise it's stale.
 */
void update_current_tick(struct stock *stock, char *page)
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

void wsj_get_EOD(struct stock *stock, char *page)
{
	struct stat sb;
	char        path[256];
	char       *csv, *p;
	double      csv_open, csv_high, csv_low, csv_close;
	uint64_t    csv_volume;
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
	csv_size += snprintf(csv+csv_size, 256, "%s,%.3f,%.3f,%.3f,%.3f,%llu\n", QDATE[1], csv_open, csv_high, csv_low, csv_close, csv_volume);
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
		wsj_query(stock, WSJ->CLOSE, WSJ->CLOSE_SZ, page, wsj_get_EOD);
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
	wsj_query(stock, WSJ->CLOSE, WSJ->CLOSE_SZ, page, wsj_get_EOD);
}

void update_current_price(struct stock *stock)
{
	char  page[96 KB];
	struct WSJ *WSJ = &stock->API.WSJ;

	wsj_query(stock, WSJ->CUR, WSJ->CUR_SZ, page, update_current_tick);
}

int load_ohlc(struct stock *stock)
{
	struct price   *price;
	struct stat     sb;
	struct filemap  filemap;
	char           *line, *map, *p;
	char            path[256];
	int              nr_ohlc = 0, mini_len = 0, nbytes, filesize;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/wsj/%s", stock->sym);
	if (stock->dead || !fs_file_exists(path) || !(map=MAP_FILE_RO(path, &filemap)))
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
	memcpy(price->price_1d+price->price_1d_len, price->price_1m+1, price->price_1m_len-1);
	price->price_1d_len = price->price_1m_len+price->price_1d_len-1;
	*(price->price_1d+price->price_1d_len) = ']';
	if (price->price_1d_len >= 256 KB) {printf(BOLDRED "CRITICAL ERROR PRICE: %d" RESET "\n", price->price_1d_len);exit(-1);}
	return 1;
}

void update_allday(struct stock *stock, char *page)
{
	struct ohlc  *ohlc;
	struct price *price;
	char         *jptr, *mptr, *p;
	char          path[64];
	int           x, fd, nbytes, json_size = 0, mini_size = 0, nr_timestamps = 0, nr_ohlc = 0, nr_volume = 0;
	double        o,h,l,c = 0.0;
	uint64_t      volume    [TRADE_MINUTES];
	time_t        timestamps[TRADE_MINUTES];
	time_t        timestamp, vol;

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

	snprintf(path, sizeof(path)-1, "data/stocks/wsj/%s", stock->sym);
	fs_newfile(path, price->price_1m, price->price_1m_len);
}

/* Called once on INIT */
int update_allday_price(struct XLS *XLS, struct stock *stock)
{
	struct WSJ   *WSJ = &stock->API.WSJ;
	struct price *price;
	char          page[96 KB];
	char          msgbuf[32 KB];
	int           msgbuf_len = 0;

	price = stock->price;
	if (stock->dead)
		return 0;
	if (market == NO_MARKET) {
		if (!load_ohlc(stock) && price->price_1d_len)
			*(price->price_1d+price->price_1d_len-1) = ']';
		price->stale = 1;
		return 0;
	}

	/*
	 * - Loaded During NO_MARKET
	 *    + load_ohlc() of previous day's 1m OHLC
	 *    + set price->stale = 1
	 * - Market ENDS after AFH
 	 */
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
		msgbuf_len += sprintf(msgbuf, "setting stock to stale: %s [%d %d] mini: %d\n", stock->sym, Server.nr_working_stocks, Server.nr_failed_stocks, price->price_mini_len);
		Server.nr_working_stocks = 0;
		Server.nr_failed_stocks  = 0;
	}
	wsj_query(stock, WSJ->ALL, WSJ->ALL_SZ, page, update_allday);
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

int wsj_query(struct stock *stock, char *url, int url_size, char *uncompressed, void (*query)(struct stock *stock, char *page))
{
	struct connection connection;
	char              gzpage[128 KB];
	char             *compressed;
	uint64_t          compressed_size;
	packet_size_t     zbytes;
	int               ret = 0, uncompressed_size;

	if (!openssl_connect_sync(&connection, Server.WSJ_ADDR, 443))
		return 0;
	SSL_write(connection.ssl, url, url_size);
	zbytes = openssl_read_http(&connection, gzpage, 64 KB);
	if (zbytes > 64 KB) {
		if (strstr(gzpage, "Unknown instrument")) {
			stock->update = 0;
			stock->dead   = 1;
			printf(BOLDRED "Unknown Instrument: %s" RESET "\n", stock->sym);
		}
		goto out;
	}
	if (strstr(gzpage, "Unknown instrument")) {
		stock->update = 0;
		stock->dead   = 1;
		printf("Unknown Instrument: %s\n", stock->sym);
		goto out;
	}

	compressed         = strstr(gzpage, "\r\n\r\n");
	compressed        += 4;
	compressed_size    = zbytes-(compressed-gzpage);
	uncompressed_size  = *(int *)(gzpage+zbytes-4);
	if (uncompressed_size > 96 KB || uncompressed_size <= 0)
		goto out;
	uncompressed_size  = zip_decompress2((unsigned char *)compressed, (unsigned char *)uncompressed, compressed_size, uncompressed_size);
	if (uncompressed_size <= 0)
		goto out;
	uncompressed[uncompressed_size] = 0;
	query(stock, uncompressed);
	if (Server.DEBUG_STOCK && !strcmp(stock->sym, Server.DEBUG_STOCK))
		printf(BOLDYELLOW "[stock: %s]\n%s" RESET "\n", stock->sym, uncompressed);
	ret = 1;
out:
	openssl_destroy(&connection);
	return ret;
}

void argv_wsj_allday(char *ticker)
{
	int x;

	holiday = 0;
	market  = DAY_MARKET;
	init_ip();
	for (x=0; x<CURRENT_XLS->nr_stocks; x++) {
		struct stock *stock = &CURRENT_XLS->STOCKS_ARRAY[x];
		if (ticker && strcmp(stock->sym, ticker))
			continue;
		stock->ohlc                  = (struct ohlc  *)zmalloc(sizeof(struct ohlc) * TRADE_MINUTES);
		stock->price                 = (struct price *)zmalloc(sizeof(struct price));
		stock->price->price_1d       = (char *)malloc(256 KB);
		stock->price->price_1d_close = (char *)malloc(168 KB);
		stock->price->price_1m       = (char *)malloc(28 KB);
		stock->price->price_mini     = (char *)malloc(32 KB);
		update_allday_price(CURRENT_XLS, stock);
	}
}
