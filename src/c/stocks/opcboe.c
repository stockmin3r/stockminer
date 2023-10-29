#include <conf.h>
#include <stocks/options.h>

static __inline__ char *skip(char *ptr, int n)
{
	int x;
//	printf("SKIP START: %.10s\n", ptr-3);
	for (x=0; x<n; x++) {
		while (*ptr != ':') ptr++;
		ptr += 1;
//		printf("skip ptr: %.10s\n", ptr-5);
	}
	return (ptr+1);
}

char *expiry_date_path(char *expiry, char *ebuf)
{
	*(unsigned short *)(ebuf+2) = *(unsigned short *)expiry;
	*(unsigned short *)(ebuf+5) = *(unsigned short *)(expiry+2);
	*(unsigned short *)(ebuf+8) = *(unsigned short *)(expiry+4);
	return (ebuf);
}

double strike_price(char *contract)
{
	char buf[12];

	*(unsigned long *)buf = *(unsigned long *)contract;
	buf[8] = 0;

	/* 10k */
	if (*(contract) != '0') {
		buf[5] = 0;
		return strtod(buf, NULL);
	}

	/* 1000-9999 */
	if (*(contract) == '0' && *(contract+1) != '0') {
		buf[5] = 0;
		return strtod(buf+1, NULL);
	}

	/* 100-999 */
	if (*(contract+1) == '0' && *(contract+2) != '0') {
		buf[5] = 0;
		return strtod(buf+2, NULL);
	}
	
	/* 10-99 */
	if (*(contract+2) == '0' && *(contract+3) != '0') {
		buf[5] = 0;
		return strtod(buf+3, NULL);
	}

	/* 1-9 */
	if (*(contract+3) == '0' && *(contract+4) != '0') {
		buf[6] = 0;
		return strtod(buf+4, NULL);
	}

	/* 0.1-0.99 */
	if (*(contract+4) == '0') {
		buf[4] = '.';
		return strtod(buf+3, NULL);
	}
	return 0.0;
}

void add_option(struct Option *opt, char *remap, int idx, int nr_days)
{
	int x;

	for (x=0; x<nr_days; x++) {
//		memmove(
	}
}

/* There is/are New Option(s) that don't exist in the filemap */
int fix_added(struct Option *options, char *map, char *remap, int remap_size, int nr_new, int nr_old)
{
//	struct Option *new_opt, *old_opt;
//	int x, y;

/*	for (x=0; x<nr_old; x++) {
		if (strcmp(new_opt->contract, old_opt->contract)) {

	}
	for (x=0; x<nr_new; x++) {
		new_opt = &options[x];
		old_opt = (struct Option *)map;
		for (y=0; y<nr_old; y++) {
			if (strcmp(new_opt->contract, old_opt->contract)) {
				add_option(new_opt, remap, x, nr_days);
				remap_size += (nr_days*sizeof(*new_opt));
			}
			old_opt++;
		}
	}*/
	return (remap_size);
}

/* There are extra options in map that aren't in the new options */
int fix_removed(struct Option *options, char *map, char *remap, int remap_size, int nr_new, int nr_old)
{
//	struct Option *new_opt, *old_opt;
//	int x;

/*	old_opt = (struct Option *)map;
	for (x=0; x<nr_old; x++) {
		for (y=0; y<nr_new; y++) {
			if (strcmp(new_opt->contract, old_opt->contract))
				remove_option(old_opt);
			new_opt++;
		}
		old_opt++;
	}
	*/
	return (remap_size);
}

void cboe_refit(struct Option *c_options, struct Option *p_options, struct filemap *filemap, int nr_days, int nr_cops, int nr_pops, int f_calls, int f_puts)
{
	char *map, *remap;
	int remap_size, filesize, opchain_fd;

	if (nr_cops != nr_pops)
		goto reset;

	filesize          = ((f_calls+f_puts)*nr_days)+2;
	filemap->filesize = filesize;
	opchain_fd        = filemap->fd;

	map = MAP_FILEMAP_RW(filemap);
	if (!map)
		goto reset;
	remap = (char *)malloc(filesize*2);
	memcpy(remap, map, filesize);
	remap_size = filesize;

	/* Call Mismatch */
	if (nr_cops != f_calls || nr_pops != f_puts) {
		if (abs(nr_cops-f_calls) > 3)
			goto reset;
		if (nr_cops > f_calls)
			remap_size = fix_added  (c_options, map, remap, remap_size, nr_cops, f_calls);
		else
			remap_size = fix_removed(c_options, map, remap, remap_size, nr_cops, f_calls);
	}
	UNMAP_FILE(map, filemap);
	ftruncate(opchain_fd, 0);
	write(opchain_fd, (void *)&nr_cops, 2);
	write(opchain_fd, (void *)&nr_pops, 2);
	write(opchain_fd, remap, remap_size);
	close(opchain_fd);
	return;
reset:
	ftruncate(opchain_fd, 0);
	write(opchain_fd, (void *)&nr_cops, 2);
	write(opchain_fd, (void *)&nr_pops, 2);
}

int extract_options_cboe(struct update *update, char *opchain)
{
	struct Option   opt;
	struct Option   p_options[1024];
	struct Option   c_options[1024];
	struct stat     sb;
	struct tm       ltm;
	struct filemap  filemap;
	char           *option, *p, *p2, *p3, *ticker, *page;
	char            opchain_path[256];
	char            current_expiry[12] = {0};
	char            ebuf[12];
	int             nr_days, cmd, no_volume, no_oi, optype, expiry_fd, opchain_fd, nr_opchains;
	unsigned short  nr_cops = 0, nr_pops = 0, file_call_options, file_put_options;
	time_t          opchains[256];
	time_t          timestamp, expiry;

	cmd         = update->cmd;
	page        = update->page;
	ticker      = update->ticker;
//	opt         = update->option;
	ebuf[0]  = '2';
	ebuf[1]  = '0';
	ebuf[4]  = '-';
	ebuf[7]  = '-';
	ebuf[10] = 0;
	option = strstr(page, "options");
	if (!option)
		return 0;
/*
* {"iv": 1.251,                    "high":   0.0, "theta":   -0.0128, "prev_day_close": 0.00499999988824129, "open": 0.0, 
*  "open_interest": 202.0,         "low":    0.0, "ask_size": 2008.0, "bid_size": 0.0, "theo":   0.0058, "percent_change": 0.0,
*  "option": "SPY210601P00290000", "bid":    0.0, "volume":   0.0,    "rho":  -0.0016, "delta": -0.0005, "ask": 0.01,
*  "tick":   "no_change",          "change": 0.0, "last_trade_price": 0.01, "vega": 0.0006, "last_trade_time": "2021-05-21T13:02:06", "gamma": 0.0}
*/
	p = option + 18;
	timestamp = time(NULL);
	while ((p2=strchr(option, '}'))) {
		memset(&opt, 0, sizeof(opt));
		opt.timestamp = timestamp;

		/* iv */
		opt.impliedVolatility = strtod(p, NULL);
		/* high */
		p = skip(p, 2);

		/* theta */
		opt.theta = strtod(p, NULL);

		/* prev_day_close */
		/* open */
		p = skip(p, 3);

		/* open_interest */
		opt.openInterest = atoi(p);

		/* low */
		/* ask_size */
		/* bid_size */
		/* theo */
		p = skip(p, 5);

		/* percent_change */
		opt.percentChange = strtod(p, NULL);
		p = skip(p, 1);

		/* option */
		p3 = strchr(p+8, '\"');
		*p3 = 0;
		strncpy(opt.contract, p+1, sizeof(opt.contract)-1);
		p = skip(p, 1);

		/* bid */
		opt.bid = strtod(p, NULL);
		p = skip(p, 1);

		/* volume */
		opt.volume = strtoul(p, NULL, 10);
		p = skip(p, 1);

		/* rho */
		opt.rho = strtod(p, NULL);
		p = skip(p, 1);

		/* delta */
		opt.delta = strtod(p, NULL);
		p = skip(p, 1);

		/* ask */
		opt.ask = strtod(p, NULL);
		p = skip(p, 3);

		/* tick */
		/* change */

		/* last_trade_price */
		opt.lastPrice = strtod(p, NULL);
		p = skip(p, 1);

		/* vega */
		opt.vega = strtod(p, NULL);
		p = skip(p, 1);

		/* last_trade_time */
		if (*p != 'n') {
			*(p+20) = 0;
			memset(&ltm, 0, sizeof(ltm));
			strptime(p+1, "%Y-%m-%dT%H:%M:%S", &ltm);
			ltm.tm_isdst  = -1;
			opt.lastTrade = mktime(&ltm);
			p = p + 20;
			p = skip(p, 1);
		} else {
			opt.lastTrade = 0;
			p = skip(p, 1);
		}
		/* gamma */
		opt.gamma = strtod(p, NULL);
		p = opt.contract;
		while (!isdigit(*p)) p++;
		opt.strike = strike_price(p+7);
//		printf("[%s] STRIKE: %.2f DELTA: %.4f RHO: %.4f ASK: %.2f LAST: %.2f VEGA: %.4f GAMMA: %.4f lastTrade: %lu\n", opt.contract, opt.strike, opt.delta, opt.rho, opt.ask, opt.lastPrice, opt.vega, opt.gamma, opt.lastTrade);

		/* New Opchain */
		if (*(unsigned int *)current_expiry != 0 && (*(unsigned int *)p != *(unsigned int *)current_expiry || *(unsigned short *)(p+4) != *(unsigned short *)(current_expiry+4))) {
			if (!opchain || (opchain && !strcmp(current_expiry, opchain))) {
				sprintf(opchain_path, "data/stocks/data/stocks/stockdb/options/%s/%s", ticker,expiry_date_path(current_expiry, ebuf));
				printf("opchain path: %s ebuf: %s\n", opchain_path, ebuf);
				opchain_fd = open(opchain_path, O_RDWR|O_CREAT|O_APPEND, 0644);
				fstat(opchain_fd, &sb);
				if (!sb.st_size) {
					write(opchain_fd, (void *)&nr_cops, 2);
					write(opchain_fd, (void *)&nr_pops, 2);
				} else {
					read(opchain_fd, (void *)&file_call_options, 2);
					read(opchain_fd, (void *)&file_put_options,  2);

					/* OPTION MISMATCH */
					if (nr_cops != file_call_options || nr_pops != file_put_options) {
						nr_days = (sb.st_size-4)/(file_call_options+file_put_options);
						filemap.fd = opchain_fd;
						cboe_refit(&c_options[0], &p_options[0], &filemap, nr_days, nr_cops, nr_pops, file_call_options, file_put_options);
						printf(BOLDRED   "(%d,%d) (%d,%d)" RESET "\n", nr_cops, file_call_options, nr_pops, file_put_options);					
					} else
						printf(BOLDGREEN "(%d,%d) (%d,%d)" RESET "\n", nr_cops, file_call_options, nr_pops, file_put_options);
				}
				/* Dump all Calls & Puts */
				write(opchain_fd, c_options, nr_cops*sizeof(struct Option));
				write(opchain_fd, p_options, nr_pops*sizeof(struct Option));
				close(opchain_fd);

				/* exp file */
				nr_opchains = load_expiry_file(ticker, &expiry_fd, opchains);
				expiry      = str2unix(ebuf);
				if (!expiry_present(opchains, nr_opchains, expiry))
					write(expiry_fd, (void *)&expiry, 8);
				close(expiry_fd);
			}
			/* Reset */
			nr_cops = 0;
			nr_pops = 0;
			/* set New Opchain Expiry */
			*(unsigned int   *)(current_expiry)   = *(unsigned int   *)(p);
			*(unsigned int   *)(current_expiry+4) = *(unsigned short *)(p+4);
		} else {
			*(unsigned int   *)(current_expiry)   = *(unsigned int   *)(p);
			*(unsigned short *)(current_expiry+4) = *(unsigned short *)(p+4);
		}

		/* Add parsed option to the list */
		if (*(char *)(p+6) == 'C')
			memcpy(&c_options[nr_cops++], &opt, sizeof(opt));
		else
			memcpy(&p_options[nr_pops++], &opt, sizeof(opt));

		/* next option */
		if (*(p2+1) == ']')
			break;
		p = p2+10;
		option = p;
	}
	return 0;
}

void save_cboe_options(char *ticker, char *json)
{
	char path[256];
	char timestr[32];
	time_t timestamp;

	timestamp = time(NULL);
//	if (server_type != SERVER_PRODUCTION)
//		timestamp -= UTCOFF;
	sprintf(path, "data/stocks/data/stocks/stockdb/options/%s/%s", ticker, unix2str(timestamp, timestr));
	printf("saving cboe options json to: %s\n", path);
	fs_newfile(path, json, strlen(json));
}

int cboe_query(struct stock *stock, char *opchain)
{
	struct update    update;
	char             page[8192 KB];
	char             url[512];

	if (!stock)
		return 0;
	snprintf(url, sizeof(url)-1, "https://cdn.cboe.com/api/global/delayed_quotes/options/%s.json", stock->sym);
	if (!curl_get(url, page))
		return 0;

	save_cboe_options(stock->sym, page);
	return 1;
	update.page   = page;
	update.ticker = stock->sym;
	extract_options_cboe(&update, opchain);
	return 1;
}

void op_query(char *ticker)
{
	int x;

	load_option_stocks();
	for (x=0; x<nr_highcap_options; x++) {
		if (ticker && strcmp(ticker, HIGHCAP_OPTIONS[x]))
			continue;
		cboe_query(search_stocks(HIGHCAP_OPTIONS[x]), NULL);
		ticker = NULL;
	}
	for (x=0; x<nr_lowcap_options; x++) {
		if (ticker && strcmp(ticker, LOWCAP_OPTIONS[x]))
			continue;
		cboe_query(search_stocks(LOWCAP_OPTIONS[x]), NULL);
		ticker = NULL;
	}
}

void op_query_ticker(char *ticker)
{
	load_option_stocks();
	cboe_query(search_stocks(ticker), NULL);
}

void op_query_opchain(char *ticker, char *opchain)
{
	load_option_stocks();
	cboe_query(search_stocks(ticker), opchain);
}
