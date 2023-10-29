#include <conf.h>
#include <stocks/options.h>

int fetch_options(char *ticker, time_t date, char *page)
{
	char url[512];

	if (!date) {
		strcpy(url, YAHOO_OPTIONS);
		strcpy(url+sizeof(YAHOO_OPTIONS)-1, ticker);
	} else {
		sprintf(url, YAHOO_OPTIONS_DATE, ticker, date);
	}
	if (!curl_get(url, page)) {
		printf("failed to fetch: %s\n", ticker);
		return 0;
	}
	return 1;
}

void refetch_opchain(char *ticker, time_t timestamp)
{
	struct update update;
	char          page[1024 KB];
	char          expbuf[32];

	update.ticker  = ticker;
	update.page    = page;
	update.expiry  = timestamp;
	update.cmd     = OPCHAIN_OVERWRITE;
	update.nr_saved_expiry = -1;

	if (!fetch_options(ticker, 0, page)) {
		printf(BOLDMAGENTA "[%s] [%s] failed to fetch opchain" RESET "\n", ticker, unix2str(timestamp, expbuf));
		return;
	}
	update_opchain(&update);
}

int options_get_expiry(char *page, time_t *expired, int *nr_expired)
{
	char *p, *edate;
	int nr_dates = 0;

	p = strstr(page, "expirationDates");
	if (!p)
		return 0;
	edate = p + 18;
	while ((p=strchr(edate, ','))) {
		expired[nr_dates++] = strtoul(edate, NULL, 10);
		if (*(p-1) == ']')
			break;
		edate = p + 1;
	}
	*nr_expired = nr_dates;
	return 1;
}

int update_opchain(struct update *update)
{
	char   *ticker, *page;
	char    path[256];
	char    expbuf[64];
	time_t  expiry;

	ticker = update->ticker;
	page   = update->page;
	expiry = update->expiry;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, unix2str(update->expiry, expbuf));
	printf("saving to: %s\n", path);

	update->expiry_path = path;
	if (!extract_options(update)) {
		printf(BOLDRED "%s" RESET "\n", update->page);
		printf(BOLDWHITE "failed to extract opchain [%s]: %s" RESET "\n", ticker, unix2str(expiry, expbuf));
		add_broken_opchain(ticker, expiry);
		return 0;
	}
	if (update->nr_saved_expiry == -1)
		return 1;
	if (!update->nr_saved_expiry || !expiry_present(update->saved_expiry, update->nr_saved_expiry, expiry)) {
		printf(BOLDWHITE "adding new expiry to db: %s %lu" RESET "\n", path, expiry);
		write(update->expfd, (time_t *)&expiry, sizeof(time_t));
	}
	return 1;
}

void update_highcap_options(char *offset, int stop, int update)
{
	char *ticker;
	int x, start = 0;

	for (x=0; x<nr_highcap_options; x++) {
		ticker = HIGHCAP_OPTIONS[x];
		if (offset && strcmp(offset, ticker) && !start)
			continue;
		start = 1;
		stock_update_options(ticker, NULL, update);
		if (stop)
			break;
	}
}

void update_lowcap_options(char *offset, int stop, int update)
{
	char *ticker;
	int x, start = 0;

	for (x=0; x<nr_lowcap_options; x++) {
		ticker = LOWCAP_OPTIONS[x];
		if (offset && strcmp(offset, ticker) && !start)
			continue;
		start = 1;
		stock_update_options(ticker, NULL, update);
		if (stop)
			break;
	}
}

void update_options(char *offset, int stop, int update)
{
	update_highcap_options(offset, stop, update);
	update_lowcap_options(offset,  stop, update);
}

void stock_update(char *ticker, char *exp_year, struct server *server)
{
	init_options(server);
	if (ticker) {
		stock_update_options(ticker, exp_year, OPCHAIN_UPDATE);
		return;
	}
	/* update ALL options */
	update_options(NULL, 0, OPCHAIN_UPDATE);
}

/* Non-Server Pull */
void stock_update_options(char *ticker, char *exp_year, int newday)
{
	struct stat    sb;
	struct stock  *stock;
	struct Option  opt;
	struct update  update;
	struct Option  options[1280];
	char           path[256];
	char           expbuf[64];
	char           page[1024 KB];
	char          *expiry;
	time_t         expired[256];
	time_t         saved_expiry[256];
	int            x, y, nr_expired, nr_saved_expiry, found = 0;

	if (!fetch_options(ticker, 0, page))
		return;

	if (!options_get_expiry(page, expired, &nr_expired)) {
		printf("options_get_expiry: error %s\n", ticker);
		return;
	}

	stock = search_stocks(ticker);
	if (!stock)
		return;

	update.saved_expiry    = &saved_expiry[0];
	update.nr_saved_expiry = load_expiry_file(ticker, &update.expfd, update.saved_expiry);
	update.ticker          = ticker;
	update.page            = page;
	update.cmd             = newday?newday:OPCHAIN_CREATE;
	update.option          = &opt;

	for (x=0; x<nr_expired; x++) {
		expiry = unix2str(expired[x], expbuf);
		printf("processing expiry: %lu exp: %s expiry: %s\n", expired[x], unix2str(expired[x], expbuf), expiry);
		if (exp_year && strcmp(expiry, exp_year))
			continue;

		if (!newday && opchain_exists(ticker, expiry)) {
			printf("opchain already exists\n");
			continue;
		}
		if (update.cmd == OPCHAIN_UPDATE) {
			update.opchain = search_opchain(stock->options, expiry);
			if (!update.opchain) {
				printf("corrupt opchain: %s %lu\n", expiry, expired[x]);
				continue;
			}
			if (update.opchain->last_update == QDATESTAMP[0] && (market == NO_MARKET||market==PRE_MARKET)) {
				printf("skipping opchain: %s %s lastUpdate: %s\n", ticker, expiry, unix2str(update.opchain->last_update, expbuf));
				continue;
			}
		}
		if (x != 0 && !fetch_options(ticker, expired[x], page))
			continue;

		update.expiry  = expired[x];
		printf("updating: %s %s\n", ticker, expiry);
		update_opchain(&update);
		os_sleep(4);
	}
	close(update.expfd);
}

int csvop(double *op, char *p, int *nmap, int *volume, int optype)
{
	int nr_op = 0, count = 0;

	while (1) {
		if (*p == 'n') {
			p = p + 5;
			if (*p == ',' || *p == '}' || *p == ']')
				break;
			count++;
			continue;
		}
		if (optype == OPTION_VOLUME)
			volume[nr_op++] = strtoul(p, NULL, 10);
		else
			op[nr_op++]     = strtod(p, NULL);
		if (nmap)
			nmap[nr_op-1] = count++;
		while (*p != ',')
			p++;
		if (*(p-1) == ']')
			break;
		p = p + 1;
	}
	return (nr_op);
}

void update_contract_live(struct opstock *opstock)
{
	struct opchain *opchain = opstock->opchain;
	struct update update;
	char path[256];

	sprintf(path, "data/stocks/stockdb/options/%s/%s", opstock->stock->sym, opstock->option->contract);
	if (!opstock->OHLC && !opstock_csv_load(opstock, path))
		return;
	update.cmd = OPTION_LIVE|OPTION_1M;
}

void update_1d(char *ticker, char *contract)
{
	struct opchain *opchain;
	struct opstock *opstock;
	struct stock   *stock;
	struct Option  *cop, *pop;
	int nr_expiry, resume = 0, nr_days, nr_calls, nr_puts, x, y, z, updated = 0;

	load_option_stocks();
	for (x=0; x<nr_highcap_options; x++) {
		if (updated)
			updated = 0;
		if (!resume && ticker && strcmp(ticker, HIGHCAP_OPTIONS[x])) {
			if (do_resume)
				resume = 1;
			else
				continue;
		}
		stock = search_stocks(HIGHCAP_OPTIONS[x]);
		load_opchains(stock);
		if (verbose)
			printf("loading %s %p\n", ticker, stock->options);
		if (!stock->options)
			continue;
		opchain   = stock->options;
		nr_expiry = opchain->nr_expiry;
//		printf("loaded options %s exp: %d\n", stock->sym, nr_expiry);
		for (y=0; y<nr_expiry; y++) {
			if (opchain[y].corrupt)
				continue;
			nr_days  = opchain[y].nr_days-1;
			nr_calls = opchain[y].nr_calls;
			nr_puts  = opchain[y].nr_puts;
			cop      = opchain[y].call_options[nr_days];
			pop      = opchain[y].put_options[nr_days];
			opchain[y].call_opstocks = (struct opstock **)malloc(nr_calls * sizeof(struct opstock *));
			opchain[y].put_opstocks  = (struct opstock **)malloc(nr_puts  * sizeof(struct opstock *));
			for (z=0; z<nr_calls; z++) {
				opstock          = (struct opstock *)malloc(sizeof(*opstock));
				opstock->option  = cop++;
				opstock->stock   = stock;
				option_load_csv(stock->sym, opstock->option->contract, &opchain[y], z, OPTION_CALL);
				opchain[y].call_opstocks[z] = opstock;
				if (contract && !strcmp(contract, opstock->option->contract)) {
					update_contract_1d(opstock);
					updated = 1;
					break;
				}
			}
			if (updated)
				break;
			for (z=0; z<nr_puts; z++) {
				opstock          = (struct opstock *)malloc(sizeof(*opstock));
				opstock->option  = pop++;
				opstock->stock   = stock;
				option_load_csv(stock->sym, opstock->option->contract, &opchain[y], z, OPTION_PUT);
				opstock->csv     = opchain[y].csv_puts[z];
				opstock->csv_len = opchain[y].csv_puts_len[z];
				opstock->csv_nr_points = opchain[y].csv_put_points[z];
				opstock->nr_op   = nr_puts;
				opstock->opchain = &opchain[y];
				opchain[y].put_opstocks[z] = opstock;
				if (contract && !strcmp(contract, opstock->option->contract)) {
					update_contract_1d(opstock);
					updated = 1;
					break;
				}
			}
		}
	}
}

void update_contract_1d(struct opstock *opstock)
{
	struct opchain *opchain  = opstock->opchain;
	char           *ticker   = opstock->stock->sym;
	char           *contract = opstock->option->contract;
	struct ohlc    *OHLC;
	struct update   update;
	struct stat     sb;
	double         *opopen, *ophigh, *oplow, *opclose;
	time_t         *timestamps, timestamp_from, timestamp_to, timestamp, seekstamp;
	int            *volume, *nmap, csv_len = 1, nbytes, idx, x, nr_ohlc, overwrite = 0;
	int             nr_remote_points, last_nmap_index, nr_ohlc_left, fd;
	char            path[256];
	char            expbuf[64];
	char            csv[4 KB];

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", opstock->stock->sym, contract);
	stat(path, &sb);
	if (!opstock->OHLC && !opstock_csv_load(opstock, path))
		return;

	seekstamp       = opstock->OHLC[opstock->nr_ohlc-1].timestamp/1000;
	update.timefrom = seekstamp;
	update.timeto   = time(NULL);
	update.cmd      = OPTION_LIVE|OPTION_1D;
	if (!get_contract_ohlc(opstock->stock->sym, contract, &update))
		return;

	nr_remote_points = update.nr_points;
	last_nmap_index  = -1;
	nmap             = &update.nmap[0];
	timestamps       = &update.timestamps[0];
	for (x=0; x<nr_remote_points; x++) {
//		printf("%lu vs %lu\n", timestamps[nmap[x]], seekstamp);
		if (timestamps[nmap[x]] == seekstamp) {
			last_nmap_index = x;
			continue;
		}
		if (last_nmap_index != -1) {
			last_nmap_index = x;
			break;
		}
	}
	if (last_nmap_index == -1) {
		overwrite = 1;
		last_nmap_index = 0;
	}
	opopen         = update.open;
	ophigh         = update.high;
	oplow          = update.low;
	opclose        = update.close;
	volume         = update.volume;
	nr_ohlc_left   = nr_remote_points-last_nmap_index;
	if (nr_ohlc_left <= 0)
		return;
	if (overwrite)
		csv[0]     = '[';
	else
		csv[0]     = ',';
	for (x=0; x<nr_ohlc_left; x++) {
		idx       = nmap[last_nmap_index++];
		nbytes    = sprintf(csv+csv_len, "[%lu,%.2f,%.2f,%.2f,%.2f,%d],", timestamps[idx]*1000, opopen[x], ophigh[x], oplow[x], opclose[x], volume[x]);
		csv_len  += nbytes;
	}
	csv[csv_len-1] = ']';
	sprintf(path, "data/stocks/stockdb/options/%s/%s", opstock->stock->sym, contract);
	fd = open(path, O_RDWR);
	if (fd < 0)
		return;
	if (!overwrite)
		lseek(fd, -1, SEEK_END);
	write(fd, csv, csv_len);
	close(fd);
}

int get_contract_ohlc(char *ticker, char *contract, struct update *update)
{
	char              page[1096 KB];
	char              url[256];
	char              csv[256 KB];
	char              path[256];
	char              expbuf[64];
	double           *opopen, *ophigh, *oplow, *opclose;
	time_t           *timestamps, timestamp_from, timestamp_to;
	int              *volume, *nmap;
	char             *p, *p2, *tsp, *period;
	int               nr_timestamps, csv_len = 1, fd, nr_op, idx, nbytes, x, cmd;

	cmd            = update->cmd;
	timestamps     = update->timestamps;
	opopen         = update->open;
	ophigh         = update->high;
	oplow          = update->low;
	opclose        = update->close;
	volume         = update->volume;
	timestamp_from = update->timefrom;
	timestamp_to   = update->timeto;
	nmap           = update->nmap;

	if (cmd & OPTION_1D)
		period = YAHOO_OPTIONS_1D;
	else if (cmd & OPTION_1M)
		period = YAHOO_OPTIONS_1M;

	sprintf(url, period, contract, timestamp_from, timestamp_to);
	if (!curl_get(url, page)) {
		printf("failed to fetch: %s\n", contract);
		return 0;
	}

	sprintf(path, "data/stocks/stockdb/options/%s/%s", ticker, contract);
	printf(BOLDGREEN "%s" RESET "\n", url);
	p = strstr(page, "timestamp");
	if (!p)
		goto out_empty;
	tsp = p + 12;
	while ((p=strchr(tsp, ','))) {
		timestamps[nr_timestamps++] = strtoul(tsp, NULL, 10);
		if (*(p-1) == ']')
			break;
		tsp = p + 1;
	}

	p = strstr(tsp, "high");
	if (!p)
		goto out_empty;
	csvop(ophigh, p+7, NULL, NULL, OPTION_OHLC);

	p = strstr(tsp, "low");
	if (!p)
		return 0;
	csvop(oplow, p+6, NULL, NULL, OPTION_OHLC);
	
	p = strstr(tsp, "open");
	if (!p)
		return 0;
	csvop(opopen, p+7, NULL, NULL, OPTION_OHLC);

	p = strstr(tsp, "close");
	if (!p)
		return 0;
	nr_op = csvop(opclose, p+8, nmap, NULL, OPTION_OHLC);

	p = strstr(tsp, "volume");
	if (!p)
		return 0;
	csvop(NULL, p+9, NULL, volume, OPTION_VOLUME);

	update->nr_points = nr_op;
	if (cmd & OPTION_LIVE)
		return 1;

	*csv = '[';
	for (x=0; x<nr_op; x++) {
		idx       = nmap[x];
		nbytes    = sprintf(csv+csv_len, "[%lu,%.2f,%.2f,%.2f,%.2f,%d],", timestamps[idx]*1000, opopen[x], ophigh[x], oplow[x], opclose[x], volume[x]);
		csv_len  += nbytes;
		if (cmd & OPTION_PRINT)
			printf("[%lu,%.2f,%.2f,%.2f,%.2f,%d]" BOLDGREEN " [%s]" RESET "\n",  timestamps[idx]*1000, opopen[x], ophigh[x], oplow[x], opclose[x], volume[x], unix2str(timestamps[idx], expbuf));
	}
	if (cmd & OPTION_PRINT)
		return 1;

	*(csv+csv_len-1) = ']';
	printf("%s\n", csv);
	fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0644);
	write(fd, csv, csv_len);
	close(fd);
	return 1;
out_empty:
	fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0644);
	write(fd, "empty", 5);
	close(fd);
	return 1;
}

void get_opchain_history(char *offset, char *contract)
{
	struct stock   *stock;
	struct opchain *opchain;
	struct Option  *cop, *pop;
	struct update   update;
	char            expbuf[64];
	int x, y, z, start = 0, nr_expiry, nr_days, nr_calls, nr_puts, overwrite = contract ? 1 : 0;

	update.overwrite = overwrite;
	update.cmd       = 0;
	update.timefrom  = YEAR_2021_TIMESTAMP;
	update.timeto    = time(NULL);
	for (x=0; x<nr_highcap_options; x++) {
		if (offset && strcmp(offset, HIGHCAP_OPTIONS[x]) && !start)
			continue;
		start = 1;

		stock = search_stocks(HIGHCAP_OPTIONS[x]);
		load_opchains(stock);
		if (!stock->options)
			continue;
		opchain   = stock->options;
		nr_expiry = opchain->nr_expiry;
		for (y=0; y<nr_expiry; y++) {
			nr_days  = opchain[y].nr_days-1;
			nr_calls = opchain[y].nr_calls;
			nr_puts  = opchain[y].nr_puts;
			cop      = opchain[y].call_options[nr_days];
			pop      = opchain[y].put_options[nr_days];
			for (z=0; z<nr_calls; z++) {
				if (contract && strcmp(contract, cop->contract)) {
					cop++;
					continue;
				}
				if (!overwrite && !csv_exists(stock->sym, contract))
					continue;
				if (get_contract_ohlc(stock->sym, cop->contract, &update) != -1)
					os_sleep(1);
				if (contract)
					return;
				cop++;
			}
			for (z=0; z<nr_puts; z++) {
				if (contract && strcmp(contract, pop->contract)) {
					pop++;
					continue;
				}
				if (!overwrite && !csv_exists(stock->sym, contract))
					continue;
				if (get_contract_ohlc(stock->sym, pop->contract, &update) != -1)
					os_sleep(1);
				if (contract)
					return;
				pop++;
			}
		}
	}
}
