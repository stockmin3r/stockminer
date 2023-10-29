#include <conf.h>
#include <stocks/options.h>

void show_options(char *ticker, char *expiry_date)
{
	struct opchain *opchain;
	struct stock   *stock;
	struct Option  *opt;
	char            expbuf[32];
	int             nr_calls, nr_puts, nr_days, x, y, nr_expiry, openInterest = 0;

	stock = search_stocks(ticker);
	if (!load_opchains(stock))
		return;
	opchain   = stock->options;
	nr_expiry = opchain->nr_expiry;
	for (x=0; x<nr_expiry; x++) {
		if (expiry_date && strcmp(opchain[x].expiry_date, expiry_date))
			continue;
		// Iterate through each Option in the Option Chain
		nr_days  = opchain[x].nr_days-1;
		opt      = opchain[x].call_options[nr_days];
		nr_calls = opchain[x].nr_calls;
		nr_puts  = opchain[x].nr_puts;
		printf(BOLDBLUE "---------%s---------[%d, %d]" RESET "\n", opchain[x].expiry_date, nr_calls, nr_puts);
		for (y=0; y<nr_calls; y++) {
			printf(BOLDGREEN "%s strike: %.2f lastPrice: %.2f change: %.2f percentChange: %.2f volume: %d openInterest: %d bid: %.2f ask: %.2f expiry: %lu lastTrade: %lu impliedVolatility: %.2f " RESET BOLDYELLOW "time: %s" RESET "\n",
			opt->contract, opt->strike, opt->lastPrice, opt->change, opt->percentChange, opt->volume, opt->openInterest, opt->bid, opt->ask, opt->expiry, opt->lastTrade, opt->impliedVolatility, unix2str(opt->timestamp, expbuf));
			openInterest += opt->openInterest;
			opt++;
		}
		opt = opchain[x].put_options[nr_days];
		for (y=0; y<nr_puts; y++) {
			printf(BOLDRED "%s strike: %.2f lastPrice: %.2f change: %.2f percentChange: %.2f volume: %d openInterest: %d bid: %.2f ask: %.2f expiry: %lu lastTrade: %lu impliedVolatility: %.2f " RESET BOLDYELLOW "time: %s" RESET "\n",
			opt->contract, opt->strike, opt->lastPrice, opt->change, opt->percentChange, opt->volume, opt->openInterest, opt->bid, opt->ask, opt->expiry, opt->lastTrade, opt->impliedVolatility, unix2str(opt->timestamp, expbuf));
			openInterest += opt->openInterest;
			opt++;
		}
		printf(BOLDWHITE "openInterest total: %d average: %d" RESET "\n", openInterest, openInterest/(nr_calls+nr_puts));
	}
}

void show_expiry(char *ticker, int cmd)
{
	struct stat sb;
	char        path[256];
	char        expbuf[64];
	char       *exp_year;
	time_t      expiry[1024];
	int         nr_expiry, x;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	nr_expiry = (fs_readfile(path, (char *)expiry, sizeof(expiry)) / sizeof(time_t));
	for (x=0; x<nr_expiry; x++) {
		exp_year = unix2str(expiry[x], expbuf);
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, exp_year);
		if (stat(path, &sb) == -1)
	        printf(BOLDRED   "%s %s %lu" RESET "\n", ticker, unix2str(expiry[x], expbuf), expiry[x]);
		else if (cmd != OPSHOW_CORRUPT)
	        printf(BOLDGREEN "%s %s %lu" RESET "\n", ticker, unix2str(expiry[x], expbuf), expiry[x]);
	}
}

void verify_expiry(char *contract)
{
	int x;

	for (x=0; x<nr_highcap_options; x++)
		show_expiry(HIGHCAP_OPTIONS[x], OPSHOW_CORRUPT);
	for (x=0; x<nr_lowcap_options; x++)
		show_expiry(LOWCAP_OPTIONS[x], OPSHOW_CORRUPT);
}

/* show CSV points from hard drive */
void show_csv_points(char *ticker, char *contract)
{
	struct opchain *opchain;
	struct stock   *stock;
	struct Option  *cop, *pop;
	struct ohlc     ohlc[2];
	char            expbuf[32];
	char            path[256];
	char            csvbuf[64 KB];
	int             nr_calls, nr_puts, nr_days, x, y, z, nr_expiry, nr_ohlc;

	stock = search_stocks(ticker);
	if (!load_opchains(stock))
		return;
	opchain   = stock->options;
	nr_expiry = opchain->nr_expiry;
	for (x=0; x<nr_expiry; x++) {
		nr_days  = opchain[x].nr_days-1;
		cop      = opchain[x].call_options[nr_days];
		pop      = opchain[x].put_options[nr_days];
		nr_calls = opchain[x].nr_calls;
		nr_puts  = opchain[x].nr_puts;
		for (y=0; y<nr_calls; y++) {
			if (contract && strcmp(contract, cop->contract)) {
				cop++;
				continue;
			}
			snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, cop->contract);
			if (!fs_readfile_str(path, csvbuf, sizeof(csvbuf)))
				continue;
			cop++;
			if (!csv_load(csvbuf, &ohlc[0])) {
				printf("csv load failed\n");
				continue;
			}
			for (z=0; z<2; z++)
				printf(BOLDGREEN "[%s %s]" RESET "[%s, %.2f, %.2f, %.2f, %.2f, %llu]\n", ticker, cop->contract, unix2str(ohlc[z].timestamp/1000, expbuf), ohlc[z].open, ohlc[z].high, ohlc[z].low, ohlc[z].close, ohlc[z].volume);
		}
		for (y=0; y<nr_puts; y++) {
			if (contract && strcmp(contract, pop->contract)) {
				pop++;
				continue;
			}
			snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, pop->contract);
			fs_readfile_str(path, csvbuf, sizeof(csvbuf));
			pop++;
			if (!csv_load(csvbuf, &ohlc[0])) {
				printf("csv load failed\n");
				continue;
			}
			for (z=0; z<2; z++)
				printf(BOLDGREEN "[%s %s]" RESET "[%s, %.2f, %.2f, %.2f, %.2f, %llu]\n", ticker, pop->contract, unix2str(ohlc[z].timestamp/1000, expbuf), ohlc[z].open, ohlc[z].high, ohlc[z].low, ohlc[z].close, ohlc[z].volume);
		}
	}
}

/* show CSV points from Yahoo remotely */
void show_contract_1d(char *contract, int range)
{
	struct tm     *local_tm, ltm;
	struct update  update;
	char           ticker[8];
	char           path[256];
	char          *p;
	int            x = 0;
	time_t         utc_time, timenow_ny;

	utc_time   = time(NULL);
	local_tm   = localtime_r(&utc_time, &ltm);
	timenow_ny = mktime(local_tm);

	p = contract;
	while (*p != '_' && p-contract < 8) {
		ticker[x++] = *p;
		p++;
	}
	ticker[x] = 0;
	if (!range) {
		update.timefrom = YEAR_2021_TIMESTAMP;
		update.timeto   = timenow_ny;
		update.cmd      = OPTION_1D|OPTION_PRINT;
		get_contract_ohlc(ticker, contract, &update);
	} else {
		update.timefrom = timenow_ny-((60*7)*3600);
		update.timeto   = timenow_ny;
		update.cmd      = OPTION_1D|OPTION_PRINT;
		get_contract_ohlc(ticker, contract, &update);
	}
}

void opchain_query(char *ticker, char *expyear, int cmd)
{
	struct stat    sb;
	char           path[256];
	char           buf[8];
	time_t         expiry[4096];
	int            nr_expiry, fd, expected_total, actual_total;
	unsigned short nr_calls, nr_puts;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	nr_expiry = (fs_readfile(path, (char *)expiry, sizeof(expiry)) / 8);
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, expyear);
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("opchain %s %s does not exist" RESET "\n", ticker, expyear);
		return;
	}
	read(fd, buf, 4);
	fstat(fd, &sb);
	nr_calls       = *(unsigned short *)buf;
	nr_puts        = *(unsigned short *)(buf+2);
	expected_total = nr_calls+nr_puts;
	actual_total   = (sb.st_size-4)/sizeof(struct Option);
	if (actual_total != expected_total)
		printf(BOLDWHITE "[%-5s]" RESET BOLDRED   " [%s] nr_calls: %-3d nr_puts: %-3d (expected total: " RESET BOLDWHITE "%-4d" RESET BOLDRED   ") actual total: " BOLDWHITE "%-4d" RESET "\n", ticker, expyear, nr_calls, nr_puts, expected_total, actual_total);
	else if (cmd != OPSHOW_CORRUPT)
		printf(BOLDWHITE "[%-5s]" RESET BOLDGREEN " [%s] nr_calls: %-3d nr_puts: %-3d (expected total: " RESET BOLDWHITE "%-4d" RESET BOLDGREEN ") actual total: " BOLDWHITE "%-4d" RESET "\n", ticker, expyear, nr_calls, nr_puts, expected_total, actual_total);
	close(fd);
}

void stock_query(char *ticker, int cmd)
{
	char *oplist[1024];
	int   nr_opchains, x;

	nr_opchains = opchain_list(ticker, oplist);

	for (x=0; x<nr_opchains; x++)
		opchain_query(ticker, oplist[x], cmd);
}

void db_query(int cmd)
{
	int x;

	for (x=0; x<nr_highcap_options; x++)
		stock_query(HIGHCAP_OPTIONS[x], cmd);
	printf(BOLDBLUE "-------------------- LOWCAPS --------------------" RESET "\n");
	for (x=0; x<nr_lowcap_options; x++)
		stock_query(LOWCAP_OPTIONS[x], cmd);
}

void show_corrupt(char *ticker)
{
	if (ticker) {
		stock_query(ticker, OPSHOW_CORRUPT);
		return;
	}
	db_query(OPSHOW_CORRUPT);
}

void option_query(char *contract)
{
	struct opstock *opstock;
	struct Option  *opt;
	char expbuf[32];

	opstock = search_contract(NULL, contract);
	if (!opstock) {
		printf(BOLDRED "%s not found" RESET "\n", contract);
		return;
	}
	opt = opstock->option;
	printf(BOLDGREEN "%s strike: %.2f lastPrice: %.2f change: %.2f percentChange: %.2f volume: %d openInterest: %d bid: %.2f ask: %.2f expiry: %lu lastTrade: %lu impliedVolatility: %.2f time: %s " RESET "\n",
	opt->contract, opt->strike, opt->lastPrice, opt->change, opt->percentChange, opt->volume, opt->openInterest, opt->bid, opt->ask, opt->expiry, opt->lastTrade, opt->impliedVolatility, unix2str(opt->timestamp, expbuf));
}

int opchain_present(char *ticker, char *expiry)
{
	struct stat sb;
	char path[256];

	sprintf(path, "data/stocks/stockdb/options/%s/%s", ticker, expiry);
	if (stat(path, &sb) == 0)
		return 1;
	return 0;
}

void opchain_missing(char *ticker)
{
	time_t  timestamps[1024];
	char   *expiry;
	char    expbuf[64];
	int     nr_expiry, expfd, x;

	nr_expiry = load_expiry_file(ticker, &expfd, timestamps);
	for (x=0; x<nr_expiry; x++) {
		expiry = unix2str(timestamps[x], expbuf);
		if (!opchain_present(ticker, expiry))
			printf(BOLDRED "opchain missing: " RESET BOLDWHITE "%s %s" RESET "\n", ticker, expiry);
	}
}

void show_missing(char *ticker)
{
	int x;

	if (ticker) {
		opchain_missing(ticker);
		return;
	}
	for (x=0; x<nr_highcap_options; x++)
		opchain_missing(HIGHCAP_OPTIONS[x]);
	for (x=0; x<nr_lowcap_options; x++)
		opchain_missing(LOWCAP_OPTIONS[x]);
}

void opchain_timestamp(char *ticker)
{
	struct stock   *stock;
	struct Option  *op;
	struct opchain *opchain;
	char            expbuf[64];
	int             x;

	stock = search_stocks(ticker);
	opchain = stock->options;

	for (x=0; x<opchain->nr_expiry; x++) {
		if (opchain[x].corrupt)
			continue;
		if (opchain[x].nr_calls)
			op = opchain[x].call_options[0];
		else
			op = opchain[x].put_options[0];
		printf(BOLDBLUE "[%s %s] " RESET BOLDYELLOW "[%s %lu]" RESET "\n", ticker, opchain[x].expiry_date, unix2str2(op->timestamp, expbuf), op->timestamp);
	}
}

void show_chainstamp(char *ticker, struct server *server)
{
	int x;

	init_options(server);
	if (ticker) {
		opchain_timestamp(ticker);
		return;
	}
	for (x=0; x<nr_highcap_options; x++)
		opchain_timestamp(HIGHCAP_OPTIONS[x]);
	for (x=0; x<nr_lowcap_options; x++)
		opchain_timestamp(LOWCAP_OPTIONS[x]);
}

int opchain_getdents(char *ticker, char **expiry)
{
	struct dirmap dirmap;
	char          path[256];
	char         *filename;
	int           nr_expiry = 0;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s", ticker);
	if (!fs_opendir(path, &dirmap))
		return 0;
	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (!strncmp(filename, "202", 3))
			expiry[nr_expiry++] = strdup(filename);
	}
	fs_closedir(&dirmap);
	return (nr_expiry);
}

void opchain_days(char *ticker, char *exp_year)
{
	struct filemap filemap;
	char path[256];
	char expbuf[32];
	char *map;
	char *expiry[1024];
	int x, nr_calls, nr_puts, nr_ops, filesize, nr_expiry, nr_days;

	nr_expiry = opchain_getdents(ticker, expiry);
	for (x=0; x<nr_expiry; x++) {
		if (exp_year && strcmp(expiry[x], exp_year))
			continue;
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, expiry[x]);
		map = MAP_FILE_RO(path, &filemap);
		if (!map)
			continue;
		nr_calls = *(unsigned short *)map;
		nr_puts  = *(unsigned short *)(map+2);
		nr_ops   = nr_calls+nr_puts;
		nr_days  = (filemap.filesize-4)/(nr_ops*sizeof(struct Option));
		printf("%s %s: %d days, lastUpdate: %s filesize: %d\n", ticker, expiry[x], nr_days, unix2str(get_last_update(ticker, expiry[x]), expbuf), (int)filemap.filesize);
		UNMAP_FILE(map, &filemap);
	}
}

void show_days(char *ticker, char *exp_year)
{
	int x;

	if (ticker) {
		opchain_days(ticker, exp_year);
		return;
	}
	for (x=0; x<nr_highcap_options; x++)
		opchain_days(HIGHCAP_OPTIONS[x], exp_year);
	for (x=0; x<nr_lowcap_options; x++)
		opchain_days(LOWCAP_OPTIONS[x], exp_year);
}

void opchain_stat(struct Option *op, int nr_options, int *OI, int *VOL)
{
	int x, openInterest_avg = 0, volume_avg = 0;

	for (x=0; x<nr_options; x++) {
		openInterest_avg += op->openInterest;
		volume_avg       += op->volume;
		op++;
	}
	if (openInterest_avg)
		*OI  = openInterest_avg/nr_options;
	else
		*OI  = 0;
	if (volume_avg)
		*VOL = volume_avg/nr_options;
	else
		*VOL = 0;
}

void opchain_stats(char *ticker, char *exp_year)
{
	struct Option  *op;
	struct filemap  filemap;
	char            path[256];
	char            expbuf[32];
	char           *map;
	char           *expiry[1024];
	int             x, y, nr_calls, nr_puts, nr_ops, filesize, nr_expiry, nr_days;
	int             openInterest = 0, volume = 0;

	nr_expiry = opchain_getdents(ticker, expiry);
	for (x=0; x<nr_expiry; x++) {
		if (exp_year && strcmp(expiry[x], exp_year))
			continue;
		sprintf(path, "data/stocks/stockdb/options/%s/%s", ticker, expiry[x]);
		map = MAP_FILE_RO(path, &filemap);
		if (!map)
			continue;
		nr_calls = *(unsigned short *)map;
		nr_puts  = *(unsigned short *)(map+2);
		nr_ops   = nr_calls+nr_puts;
		nr_days  = (filemap.filesize-4)/(nr_ops*sizeof(struct Option));
		op       = (struct Option *)(map+4);
		if (nr_days > 1) {
			for (y=0; y<nr_days; y++) {
				opchain_stat(op, nr_ops, &openInterest, &volume);
				printf(BOLDWHITE "%-5s" RESET BOLDBLUE " [%s] [%d] " RESET BOLDYELLOW "%s" RESET BOLDWHITE " OpenInterest: " RESET BOLDBLUE "%-6d " RESET BOLDWHITE "Vol: %d" RESET "\n", ticker, expiry[x], nr_days, unix2str(op->timestamp, expbuf), openInterest, volume);
				op          += nr_ops;
			}
		} else {
			opchain_stat(op, nr_ops, &openInterest, &volume);
			printf(BOLDWHITE "%-5s [%s] [%d] " RESET BOLDYELLOW "%s" RESET BOLDWHITE " OpenInterest: %-6d Vol: %d" RESET "\n", ticker, expiry[x], nr_days, unix2str(op->timestamp, expbuf), openInterest, volume);
		}
		UNMAP_FILE(map, &filemap);
	}
}

void show_stats(char *ticker, char *exp_year)
{
	int x;

	if (ticker) {
		opchain_stats(ticker, exp_year);
		return;
	}
	for (x=0; x<nr_highcap_options; x++)
		opchain_stats(HIGHCAP_OPTIONS[x], exp_year);
	for (x=0; x<nr_lowcap_options; x++)
		opchain_stats(LOWCAP_OPTIONS[x], exp_year);
}
