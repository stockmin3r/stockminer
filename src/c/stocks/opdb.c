#include <conf.h>
#include <stocks/options.h>

static char **OPTIONS;
static int    NR_OPTIONS;

struct opchain *search_opchain(struct opchain *opchain, char *expiry)
{
	int x;
	for (x=0; x<opchain->nr_expiry; x++) {
		if (!strcmp(opchain[x].expiry_date, expiry))
			return &opchain[x];
	}
	printf(BOLDRED "search_opchain: corrupted: %s" RESET "\n", expiry);
	return (NULL);
}

int expiry_present(time_t *expiry, int nr_expiry, time_t timestamp)
{
	int x;
	for (x=0; x<nr_expiry; x++) {
		if (expiry[x] == timestamp)
			return 1;
	}
	return 0;
}

void add_expiry(char *ticker, time_t timestamp)
{
	char path[256];

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	fs_appendfile(path, (void *)&timestamp, sizeof(time_t));
}

void remove_expiry(char *ticker, time_t timestamp)
{
	struct filemap filemap;
	char path[256];
	char *map;
	int fd, filesize;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	map = MAP_FILE_RW(path, &filemap);
	UNMAP_FILE(map, &filemap);
}

int load_expiry_file(char *ticker, int *expfd, time_t *saved_expiry)
{
	struct stat sb;
	char path[256];
	int fd;

	/* Read in current expiry file */
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	fd = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
	fstat(fd, &sb);
	read(fd, saved_expiry, sb.st_size);
	*expfd = fd;
	if (sb.st_size == 0)
		return 0;
	return sb.st_size/sizeof(time_t);
}

time_t get_last_update(char *ticker, char *expiry_date)
{
	struct stat sb;
	struct Option *opt;
	char path[256];
	char buf[256];
	int fd;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, expiry_date);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	fstat(fd, &sb);
	if (sb.st_size < sizeof(struct Option)+4)
		return 0;
	lseek(fd, (int64_t)-sizeof(struct Option), SEEK_END);
	read(fd, buf, sizeof(struct Option));
	close(fd);
	opt = (struct Option *)buf;
	return opt->timestamp;
}

int zombie_opchain(char *ticker, char *target, char **expiry, int nr_expiry)
{
	struct stat s1, s2;
	char path[256];
	char zombie_path[256];
	char op1[64 KB];
	char op2[64 KB];
	int x, is_zombie = 0;

	snprintf(zombie_path, sizeof(zombie_path)-1, "data/stocks/stockdb/options/%s/%s", ticker, target);
	if (stat(zombie_path, &s1) == -1)
		return 0;
	fs_readfile(zombie_path, op1, sizeof(op1));
	for (x=0; x<nr_expiry; x++) {
		if (!strcmp(expiry[x], target))
			continue;
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, expiry[x]);
		if (stat(path, &s2) == -1)
			continue;
		if (s1.st_size != s2.st_size)
			continue;
		fs_readfile(path, op2, sizeof(op2));
		if (!memcmp(op1, op2, s1.st_size)) {
			unlink(path);
			is_zombie = 1;
			continue;
		}
	}
	if (is_zombie) {
		unlink(zombie_path);
		return 1;
	}
	return 0;
}

void map_opchain(char *ticker, char *exp_year)
{
	struct Option *op;
	struct filemap filemap;
	char path[256];
	char *map;
	int filesize, fd, nr_days, nr_calls, nr_puts, nr_op, x;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, exp_year);
	map = MAP_FILE_RO(path, &filemap);
	nr_calls = *(unsigned short *)map;
	nr_puts  = *(unsigned short *)(map+2);
	nr_op    = nr_calls+nr_puts;

	nr_days  = (filemap.filesize-4)/(nr_op*sizeof(struct Option));
	op = (struct Option *)(map+4);
	for (x=0; x<nr_op; x++) {
		if (op->timestamp > 2616697055UL) {
			char *tp = (char *)&op->timestamp;
			*(time_t *)tp = (double)op->timestamp;
			printf("FAKE TIMESTAMP: %s %s %lu float: %.2f\n", ticker, exp_year, op->timestamp, (double)op->timestamp);
		}
		op++;
	}
	UNMAP_FILE(map, &filemap);
}

void opchain_fix(char *ticker)
{
	struct stat    sb;
	struct dirmap  dirmap;
	struct update  update;
	struct Option *op;
	struct Option  options[256];
	char           path[1024];
	char           buf[8];
	char           expbuf[32];
	char          *map, *filename;
	char          *expiry[256];
	time_t         saved_expiry[2048];
	time_t         timestamp;
	int            count = 0, fd, expfd, expected_total, actual_total, x, filesize, nr_expiry = 0;
	unsigned short nr_calls, nr_puts, nr_saved_expiry;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s", ticker);
	if (!fs_opendir(path, &dirmap))
		return;

	nr_saved_expiry = load_expiry_file(ticker, &expfd, saved_expiry);
	update.nr_saved_expiry = 0;

	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (!strncmp(filename, "202", 3))
			expiry[nr_expiry++] = strdup(filename);
	}
	fs_closedir(&dirmap);
	close(expfd);

	for (x=0; x<nr_expiry; x++) {
		timestamp = ny_time(expiry[x]);
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, expiry[x]);
		printf("%s\n", path);
		fd = open(path, O_RDONLY);
		if (fd < 0)
			continue;
		fstat(fd, &sb);
		read(fd, buf, 4);
		close(fd);
		nr_calls = *(unsigned short *)buf;
		nr_puts  = *(unsigned short *)(buf+2);
		expected_total = nr_calls+nr_puts;
		actual_total   = (sb.st_size-4)/sizeof(struct Option);
/*		if (zombie_opchain(ticker, expiry[x], expiry, nr_expiry)) {
			printf("zombie opchain: %s" RESET "\n", expiry[x]);
			refetch_opchain(ticker, timestamp);
			continue;
		}*/
		if (!expiry_present(saved_expiry, nr_saved_expiry, timestamp)) {
//			add_expiry(ticker, timestamp);
			printf(BOLDGREEN "added timestamp to: %s %lu %s" RESET "\n", ticker, timestamp, expiry[x]);
		}
		if (expected_total != actual_total) {
			printf(BOLDRED "[%s] corrupt expiry: nr_calls: %d nr_puts: %d expected: %d actual: %d" RESET "\n", ticker, (int)nr_calls, (int)nr_puts, expected_total, actual_total);
//			refetch_opchain(ticker, timestamp);
		}
	}
}

void opchains_fix(char *ticker)
{
	int x;

	if (ticker) {
		opchain_fix(ticker);
		return;
	}
	for (x=0; x<nr_highcap_options; x++)
		opchain_fix(HIGHCAP_OPTIONS[x]);
	for (x=0; x<nr_lowcap_options; x++)
		opchain_fix(LOWCAP_OPTIONS[x]);
}

void opchain_fixoi(struct stock *stock, char *expiry_date)
{
	struct opchain *opchain;
	int nr_expiry, openInterest, x;

	opchain = stock->options;
	if (!opchain)
		return;
	nr_expiry = opchain->nr_expiry;

	for (x=0; x<nr_expiry; x++) {
		if (expiry_date && strcmp(opchain[x].expiry_date, expiry_date))
			continue;
		openInterest = opchain[x].openInterest_avg;
		if (openInterest < 10 && (opchain[x].nr_calls > 10 || opchain[x].nr_puts > 10)) {
			if (openInterest <= 10) {
				printf(BOLDWHITE "REFETCH: %-5s [%s] %.2f avg openInterest: %-4d avg volume: %-4d" RESET "\n", stock->sym, opchain[x].expiry_date,  opchain[x].csv_total_points, opchain[x].openInterest_avg, opchain[x].volume_avg);
//				refetch_opchain(stock->sym, opchain[x].expiry);
//				fs_sleep(5);
				continue;
			}
		}
	}
}

void opchains_fixoi(char *ticker, char *expiry_date, struct server *server)
{
	int x;

	init_options(server);
	if (ticker) {
		opchain_fixoi(search_stocks(ticker), expiry_date);
		return;
	}
	for (x=0; x<nr_highcap_options; x++)
		opchain_fixoi(search_stocks(HIGHCAP_OPTIONS[x]), NULL);
	for (x=0; x<nr_lowcap_options; x++)
		opchain_fixoi(search_stocks(LOWCAP_OPTIONS[x]), NULL);
}

/* requires:
 *   - cmd
 *   - page
 *   - ticker
 *   - expiry_path (data/stocks/stockdb/options/TSLA/2021-03-19
 *   - option
 */
int extract_options(struct update *update)
{
	struct opconf  opconf;
	struct Option *opt;
	struct stat    sb;
	char          *chain, *p, *p2, *expiry_path, *ticker, *page, *puts;
	int            nr_call_options, nr_put_options, total_options = 0, nr_days, fd;
	int            liveconf[2], cmd, no_volume, no_oi;
	time_t         timenow;

	cmd         = update->cmd;
	page        = update->page;
	ticker      = update->ticker;
	expiry_path = update->expiry_path;
	opt         = update->option;

	if (!get_nr_options(page, &nr_call_options, &nr_put_options, &puts)) {
		printf(BOLDRED "get_nr_options(%s) failed" RESET "\n", ticker);
		return 0;
	}
	chain = strstr(page, "contractSymbol");
	if (!chain) {
		printf(BOLDRED "FAKE OR EMPTY OPCHAIN: %s" RESET "\n", expiry_path);
		return 0;
	}
	memset(opt, 0, sizeof(*opt));
	opconf.nr_calls = nr_call_options;
	opconf.nr_puts  = nr_put_options;
	printf("nr calls: %d nr_puts: %d\n", nr_call_options, nr_put_options);

	timenow = get_timestamp();
	if (cmd == OPCHAIN_CREATE) {
		if (stat(expiry_path, &sb) == 0)
			return 0;
		fd = open(expiry_path, O_RDWR|O_CREAT, 0644);
		write(fd, &opconf, sizeof(opconf));
	} else if (cmd == OPCHAIN_UPDATE) {
		fd = open(expiry_path, O_RDWR|O_APPEND, 0644);
	} else if (cmd == OPCHAIN_OVERWRITE) {
		fd = open(expiry_path, O_RDWR|O_CREAT|O_TRUNC, 0644);
		opconf.nr_calls = nr_call_options;
		opconf.nr_puts  = nr_put_options;
		write(fd, &opconf, sizeof(opconf));
	} else if (cmd == OPCHAIN_LIVE) {
		/* OPTION_LIVE */
		if (stat(expiry_path, &sb) == -1) {
			fd = open(expiry_path, O_RDWR|O_CREAT, 0644);
			if (fd < 0)
				return 0;
			memset(&opconf, 0, sizeof(opconf));
			liveconf[0] = nr_call_options;
			liveconf[1] = nr_put_options;
			write(fd, &opconf, sizeof(opconf));
		} else {
			fd = open(expiry_path, O_RDWR|O_CREAT, 0644);
			lseek(fd, -(nr_call_options+nr_put_options)*sizeof(struct Option), SEEK_END);
		}
	}

	while ((p=strstr(chain, "actSym"))) {
		no_volume = 0;
		no_oi     = 0;
		p += 12;
		p2 = strchr(p, '"');
		if (!p2)
			break;
		*p2++ = 0;
		if (*p == '\0')
			break;
		strncpy(opt->contract, p, 23);
		opt->strike = strtod(p2+10, NULL);

		p = strstr(p2+11, "lastPrice");
		if (!p)
			break;
		opt->lastPrice = strtod(p+11, NULL);
		p += 11;

		p = strchr(p, ':');
		if (!p)
			break;
		opt->change = strtod(p+1, NULL);
		p = strchr(p+2, ':');
		if (!p)
			break;
		opt->percentChange = strtod(p+1, NULL);

		p = strchr(p+2, ':');
		if (!p)
			break;
		if (*(p-2) == 'e' && *(p-3) == 'm') {
			opt->volume = strtoul(p+1, NULL, 10);
			p = strchr(p+1, ':');
			if (!p)
				break;
		} else {
			no_volume = 1;
		}
		if (*(p-2) == 't') {
			opt->openInterest = strtoul(p+1, NULL, 10);
			p = strchr(p+2, ':');
			if (!p)
				break;
		} else {
			no_oi = 1;
		}
		if (no_volume && no_oi) {
			goto expir;
		} else {
			opt->bid = strtod(p+1, NULL);
			p = strchr(p+2, ':');
			if (!p)
				break;
			opt->ask = strtod(p+1, NULL);
		}
expir:
		p = strstr(p, "expir");
		if (!p)
			break;
		opt->expiry = strtoul(p+12, NULL, 10);

		p += 18;
		p = strchr(p+2, ':');
		if (!p)
			break;
		opt->lastTrade = strtoul(p+1, NULL, 10);

		p = strchr(p+10, ':');
		if (!p)
			break;
		opt->impliedVolatility = strtod(p+1, NULL);
		opt->timestamp = ((market == NO_MARKET||market==PRE_MARKET) ? QDATESTAMP[0] : QDATESTAMP[1]);
		chain = p + 5;
//		printf("%s strike: %.2f lastPrice: %.2f change: %.2f percentChange: %.2f volume: %lu openInterest: %d bid: %.2f ask: %.2f expiry: %lu lastTrade: %lu impliedVolatility: %.2f inTheMoney: %d\n",
//		opt->contract, opt->strike, opt->lastPrice, opt->change, opt->percentChange, opt->volume, opt->openInterest, opt->bid, opt->ask, opt->expiry, opt->lastTrade, opt->impliedVolatility, opt->inTheMoney);
		write(fd, opt, sizeof(*opt));
		if (cmd == OPCHAIN_LIVE)
			opt++;
		total_options += 1;
	}
	if (total_options != (nr_call_options+nr_put_options)) {
		printf(BOLDRED "CORRUPTION: total_options: %d nr_calls: %d nr_puts: %d" RESET "\n", total_options, nr_call_options, nr_put_options);
		close(fd);
		unlink(expiry_path);
		return 0;
	}
	close(fd);
	return 1;
}

void load_option_stocks()
{
	char  buf[8 KB];
	char *line, *p;
	int   count = 0;

	fs_readfile_str((char *)"data/stocks/OPTIONS_HIGHCAPS.TXT", buf, sizeof(buf));
	nr_highcap_options = cstring_line_count(buf);
	HIGHCAP_OPTIONS    = (char **)malloc(sizeof(char *) * nr_highcap_options);
	if (!HIGHCAP_OPTIONS)
		return;

	OPTIONS = (char **)malloc(sizeof(char *) * 4096);
	if (!OPTIONS)
		return;

	line = buf;
	while ((p=strchr(line, '\n'))) {
		*p++ = 0;
		HIGHCAP_OPTIONS[count++] = strdup(line);
		OPTIONS[NR_OPTIONS++]    = HIGHCAP_OPTIONS[count-1];
		line = p;
	}

	if (!fs_readfile_str("data/stocks/OPTIONS_LOWCAPS.TXT", buf, sizeof(buf)))
		return;
	nr_lowcap_options = cstring_line_count(buf);
	LOWCAP_OPTIONS = (char **)malloc(sizeof(void *) * nr_lowcap_options);
	if (!LOWCAP_OPTIONS)
		return;

	line  = buf;
	count = 0;
	while ((p=strchr(line, '\n'))) {
		*p++ = 0;
		LOWCAP_OPTIONS[count++] = strdup(line);
		OPTIONS[NR_OPTIONS++]   = LOWCAP_OPTIONS[count-1];
		line = p;
	}
}

int load_opchains(struct stock *stock)
{
	struct stat     sb;
	struct opconf   opconf;
	struct opchain *opchain;
	struct Option  *cop, *pop;
	char            path[256];
	char            expbuf[64];
	char            cname[32];
	char           *ticker = stock->sym;
	time_t          expiry[2048];
	int x, y, z, fd, nbytes, nr_days, nr_calls, nr_puts, nr_expiry;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	nr_expiry = fs_readfile(path, (char *)expiry, sizeof(expiry)) / sizeof(time_t);
	if (nr_expiry <= 0)
		return 0;
	opchain   = (struct opchain *)zmalloc(sizeof(struct opchain) * nr_expiry);
	if (!opchain)
		return 0;
	opchain->nr_expiry = nr_expiry;

	for (x=0; x<nr_expiry; x++) {
//		printf("loading: %s %s %lu opchain: %p nr_expiry: %d\n", stock->sym, unix2str(expiry[x], expbuf), expiry[x], &opchain[x], nr_expiry);
		snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, unix2str(expiry[x], expbuf));
		fd = open(path, O_RDONLY);
		if (fd < 0) {
			printf("failed path: %s\n", path);
			return 0;
		}
		read(fd, &opconf, sizeof(opconf));
		fstat(fd, &sb);
		if (sb.st_size == 4) {
			printf(BOLDRED "corrupt opchain: %s" RESET "\n", path);
			close(fd);
			unlink(path);
			opchain[x].corrupt = 1;
			continue;
		}
		opchain[x].nr_calls = nr_calls = opconf.nr_calls;
		opchain[x].nr_puts  = nr_puts  = opconf.nr_puts;
		opchain[x].nr_days  = nr_days  = (sb.st_size-4)/((nr_calls+nr_puts)*sizeof(struct Option));

		strcpy(opchain[x].expiry_date, expbuf);
		opchain[x].expiry = expiry[x];
		opchain[x].ticker = ticker;

		opchain[x].call_options    = (struct Option **)malloc(sizeof(struct Option *)  * nr_days);
		opchain[x].put_options     = (struct Option **)malloc(sizeof(struct Option *)  * nr_days);
		for (y=0; y<nr_days; y++) {
			nr_calls = opconf.nr_calls;
			nr_puts  = opconf.nr_puts;
			opchain[x].cnames          = (char **)        malloc(sizeof(char *)        * nr_calls);
			opchain[x].pnames          = (char **)        malloc(sizeof(char *)        * nr_puts);
			opchain[x].call_options[y] = (struct Option *)malloc(sizeof(struct Option) * nr_calls);
			opchain[x].put_options[y]  = (struct Option *)malloc(sizeof(struct Option) * nr_puts);
			if (nr_calls) {
				read(fd, opchain[x].call_options[y], nr_calls * sizeof(struct Option));
				cop = opchain[x].call_options[y];
				opchain[x].last_update = cop->timestamp;
			} if (nr_puts) {
				read(fd, opchain[x].put_options[y],  nr_puts  * sizeof(struct Option));
				pop = opchain[x].put_options[y];
				opchain[x].last_update = pop->timestamp;
			}
			for (z=0; z<nr_calls; z++) {
				nbytes = snprintf(cname, sizeof(cname)-1, "%s_%s_%.2f_C", ticker, unix2str3(cop->expiry, expbuf), cop->strike);
				if (nbytes >= 24) {
					printf(BOLDRED "[-] load_opchains(): calls %s %s %s" RESET "\n", ticker, cop->contract, cname);
					return 0;
				}
				opchain[x].cnames[z] = strdup(cname);
				cop++;
			}
			for (z=0; z<nr_puts; z++) {
				nbytes = snprintf(cname, sizeof(cname)-1, "%s_%s_%.2f_P", ticker, unix2str3(pop->expiry, expbuf), pop->strike);
				if (nbytes >= 24) {
					printf(BOLDRED "[-] load_opchains(): puts %s %s %s" RESET "\n", ticker, pop->contract, cname);
					return 0;
				}
				opchain[x].pnames[z] = strdup(cname);
				pop++;
			}
		}
		close(fd);
	}
	stock->options = opchain;
	return 1;
}

void init_options(struct server *server)
{
	struct stock   *stock;
	struct opchain *opchain;
	struct opstock *opstock;
	struct Option  *cop, *pop, *op;
	char            expbuf[12];
	unsigned char   nr_ranked;
	int             x, y, z, nr_expiry, nr_calls, nr_puts, nr_days;
	double          csv_nr_points;

	if (!server->production)
		return;

	load_option_stocks();
	for (x=0; x<nr_highcap_options; x++) {
		stock = search_stocks(HIGHCAP_OPTIONS[x]);
		load_opchains(stock);
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
			opchain[y].call_opstocks = (struct opstock **)malloc(sizeof(struct opstock *) * nr_calls);
			opchain[y].put_opstocks  = (struct opstock **)malloc(sizeof(struct opstock *) * nr_puts);
			nr_global_calls += nr_calls;
			nr_global_puts  += nr_puts;
			csv_nr_points = 0;
			for (z=0; z<nr_calls; z++) {
				opstock         = (struct opstock *)malloc(sizeof(*opstock));
				opstock->option = cop++;
				opstock->stock  = stock;
				opstock->name   = opchain[y].cnames[z];
				sort_options_oi_calls(opstock);
				sort_options_vol_calls(opstock);
				option_load_csv(stock->sym, opstock->option->contract, &opchain[y], z, OPTION_CALL);
				opstock->csv     = opchain[y].csv_calls[z];
				opstock->csv_len = opchain[y].csv_calls_len[z];
				opstock->csv_nr_points = opchain[y].csv_call_points[z];
				csv_nr_points += ((double)opstock->csv_nr_points/53.0)*100.0;
				opstock->opmath  = opchain->opmath;
				opstock->expiry  = y;
				opstock->optype  = OPTION_CALL;
				opstock->nr_op   = nr_calls;
				opstock->opchain = &opchain[y];
				opchain[y].call_opstocks[z] = opstock;
				if (((double)opstock->csv_nr_points/53.0)*100.0 == 100.0) {
					nr_ranked = opchain[y].nr_ranked_calls;
					if (nr_ranked < 8) {
						opchain[y].ranked_calls[nr_ranked] = opstock;
						opchain[y].nr_ranked_calls = (nr_ranked+1);
//						printf("%-24s vol: %-4d OHLC: %.2f\n", opstock->option->contract, opstock->option->volume, ((double)opstock->csv_nr_points/53.0)*100.0);
					}
				}
				/* Stats */
				opchain[y].openInterest_avg += opstock->option->openInterest;
				opchain[y].volume_avg       += opstock->option->volume;
				if (*opstock->option->contract == '\0')
					printf(BOLDRED "FAKE OPTION: %s" RESET "\n", stock->sym);
			}
			for (z=0; z<nr_puts; z++) {
				opstock          = (struct opstock *)malloc(sizeof(*opstock));
				opstock->option  = pop++;
				opstock->stock   = stock;
				opstock->name    = opchain[y].pnames[z];
				sort_options_oi_puts(opstock);
				sort_options_vol_puts(opstock);
				option_load_csv(stock->sym, opstock->option->contract, &opchain[y], z, OPTION_PUT);
				opstock->csv     = opchain[y].csv_puts[z];
				opstock->csv_len = opchain[y].csv_puts_len[z];
				opstock->csv_nr_points = opchain[y].csv_put_points[z];
				csv_nr_points += ((double)opstock->csv_nr_points/53.0)*100.0;
				opstock->opmath  = opchain->opmath;
				opstock->expiry  = y;
				opstock->optype  = OPTION_PUT;
				opstock->nr_op   = nr_puts;
				opstock->opchain = &opchain[y];
				opchain[y].put_opstocks[z] = opstock;
				/* Stast */
				opchain[y].openInterest_avg += opstock->option->openInterest;
				opchain[y].volume_avg       += opstock->option->volume;
				if (((double)opstock->csv_nr_points/53.0)*100.0 == 100.0) {
					nr_ranked = opchain[y].nr_ranked_puts;
					if (nr_ranked < 8) {
						opchain[y].ranked_puts[nr_ranked] = opstock;
						opchain[y].nr_ranked_puts = (nr_ranked+1);
//						printf("%-24s vol: %-4d OHLC: %.2f\n", opstock->option->contract, opstock->option->volume, ((double)opstock->csv_nr_points/53.0)*100);
					}
				}
				if (*opstock->option->contract == '\0')
					printf(BOLDRED "FAKE OPTION: %s" RESET "\n", stock->sym);

			}
			if (nr_calls+nr_puts) {
				opchain[y].csv_total_points  = csv_nr_points/(double)(nr_calls+nr_puts);
				opchain[y].openInterest_avg /= (nr_calls+nr_puts);
			}
			if (!strcmp(stock->sym, "TSLA"))
				printf("%-5s [%s] %.2f avg openInterest: %-4d avg volume: %-4d\n", stock->sym, opchain[y].expiry_date,  opchain[y].csv_total_points, opchain[y].openInterest_avg, opchain[y].volume_avg);

			if (opchain[y].csv_total_points >= 50.0) {
				oprank_add_opchain(&opchain[y]);
				printf("%-5s [%s] %.2f avg openInterest: %-4d avg volume: %-4d\n", stock->sym, opchain[y].expiry_date,  opchain[y].csv_total_points, opchain[y].openInterest_avg, opchain[y].volume_avg);
			}
		}
	}
//	printf("loaded all options\n");
	update_json_options_oi_calls();
	update_json_options_vol_calls();
	update_json_options_oi_puts();
	update_json_options_vol_puts();
	printf(BOLDCYAN "Total Calls: %d Total Puts: %d" RESET "\n", nr_global_calls, nr_global_puts);
}

void rank_opchains()
{
	struct opchain *opchain;
	int x;

	for (x=0; x<nr_ranked_opchains; x++) {
		opchain = ranked_opchains[x];
	}
}

int get_nr_options(char *page, int *nr_call_options, int *nr_put_options, char **puts)
{
	char *p;
	int nr_options = 0;

	if (!page)
		return 0;

	p = strstr(page, "calls\":");
	if (!p) {
		*nr_call_options = 0;
		*nr_put_options  = 0;
		return 0;
	}
	if (*(p+8) != ']') {
		while ((p=strchr(p, '}')) != NULL) {
			nr_options++;
			if (*(p+1) == ']')
				break;
			p = p + 32;
		}
	}
	*nr_call_options = nr_options;

	nr_options = 0;
	p += 4;
	if (*(p+7) == ']') {
		*nr_put_options = 0;
		return 1;
	}
	*puts = p;
	while ((p=strchr(p, '}'))) {
		nr_options++;
		if (*(p+1) == ']')
			break;
		p = p + 32;
	}

	*nr_put_options = nr_options;
	return 1;
}

int opchain_list(char *ticker, char **opchains)
{
	char   path[256];
	char   expbuf[64];
	time_t expiry[1024];
	int    nr_opchains = 0, nr_expiry, x;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	nr_expiry = fs_readfile(path, (char *)expiry, sizeof(expiry))/sizeof(time_t);
	for (x=0; x<nr_expiry; x++)
		opchains[nr_opchains++] = strdup(unix2str(expiry[x], expbuf));
	return (nr_opchains);
}

int backchain_list(char *ticker, char **opchains, char *backup_day)
{
	char   path[256];
	char   expbuf[64];
	time_t expiry[1024];
	int    nr_opchains = 0, nr_expiry, x;

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/backup/%s/%s.exp", ticker, backup_day, ticker);
	nr_expiry = fs_readfile(path, (char *)expiry, sizeof(expiry)) / sizeof(time_t);
	for (x=0; x<nr_expiry; x++)
		opchains[nr_opchains++] = strdup(unix2str(expiry[x], expbuf));
	return (nr_opchains);
}

int opchain_exists(char *ticker, char *expiry)
{
	struct stat sb;
	char        path[256];

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/%s", ticker, expiry);
	if (!stat(path, &sb))
		return 1;
	return 0;
}

void add_broken_opchain(char *ticker, time_t timestamp)
{
	char     buf[256];
	char     expbuf[64] = {0};
	char    *expiry;
	int64_t  nbytes;

	expiry = unix2str(timestamp, expbuf);
	nbytes = snprintf(buf, sizeof(buf)-1, "%s %s\n", ticker, expiry);
	fs_appendfile("data/stocks/BROKEN_OPCHAINS", (void *)buf, nbytes);
}

void backup_expiry_file(char *ticker, char *backup_day, char *src_path)
{
	char path[256];
	char dst_path[256];

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/backup", ticker);
	fs_mkdir(path, 0755);

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/backup/%s", ticker, backup_day);
	fs_mkdir(path, 0755);

	snprintf(dst_path, sizeof(dst_path)-1, "data/stocks/stockdb/options/%s/backup/%s/%s.exp", ticker, backup_day, ticker);
	fs_copy_file(src_path, dst_path);
	printf(BOLDGREEN "backed up expiry file: %s" RESET "\n", dst_path);
}

/*
 * backup_day: 2021-03-26
 * src_path:   data/stocks/stockdb/options/TSLA/2021-03-19
 * dst_path:   data/stocks/stockdb/options/TSLA/backup/2021-03-26/2021-03-19
 */
void backup_opchain(char *ticker, char *backup_day, char *exp_year)
{
	char src_path[256];
	char dst_path[256];

	snprintf(src_path, sizeof(src_path)-1, "data/stocks/stockdb/options/%s/%s", ticker, exp_year);
	snprintf(dst_path, sizeof(dst_path)-1, "data/stocks/stockdb/options/%s/backup/%s/%s", ticker, backup_day, exp_year);
	fs_copy_file(src_path, dst_path);
}

void backup_stock(char *ticker)
{
	char  *opchains[1024];
	char   path[256];
	char  *backup_day;
	int    x, nr_opchains;

	backup_day = QDATE[0];
	printf("backup_day: %s\n", backup_day);

	nr_opchains = opchain_list(ticker, opchains);
	if (!nr_opchains) {
		printf(BOLDRED "failed to backup: %s - no opchains" RESET "\n", ticker);
		return;
	}
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	backup_expiry_file(ticker, backup_day, path);
	for (x=0; x<nr_opchains; x++)
		backup_opchain(ticker, backup_day, opchains[x]);
}

void backup_stocks(char *ticker)
{
	int x;

	if (ticker) {
		backup_stock(ticker);
		return;
	}

	for (x=0; x<nr_highcap_options; x++)
		backup_stock(HIGHCAP_OPTIONS[x]);
	for (x=0; x<nr_lowcap_options; x++)
		backup_stock(LOWCAP_OPTIONS[x]);
}

void backup_restore_stock(char *ticker, char *backup_day, char *exp_year)
{
	struct stat  sb;
	char        *opchains[1024];
	char         src_path[256];
	char         dst_path[256];
	char         path[256];
	int          x, nr_opchains;

	if (!backup_day)
		backup_day = QDATE[0];
	printf("backup_day: %s\n", backup_day);
	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/options/%s/backup/%s", ticker, backup_day);
	if (stat(path, &sb) == -1) {
		printf(BOLDRED "path does not exist: %s" RESET "\n", path);
		return;
	}

	nr_opchains = backchain_list(ticker, opchains, backup_day);
	if (!nr_opchains) {
		printf(BOLDRED "failed to backup: %s - no opchains" RESET "\n", ticker);
		return;
	}

	snprintf(src_path, sizeof(src_path)-1, "data/stocks/stockdb/options/%s/backup/%s/%s.exp", ticker, backup_day, ticker);
	snprintf(dst_path, sizeof(dst_path)-1, "data/stocks/stockdb/options/%s.exp", ticker);
	fs_copy_file(src_path, dst_path);

	for (x=0; x<nr_opchains; x++) {
		if (exp_year && strcmp(opchains[x], exp_year))
			continue;
		snprintf(src_path, sizeof(src_path)-1, "data/stocks/stockdb/options/%s/backup/%s/%s", ticker, backup_day, opchains[x]);
		snprintf(dst_path, sizeof(dst_path)-1, "data/stocks/stockdb/options/%s/%s", ticker, opchains[x]);
		fs_copy_file(src_path, dst_path);
		printf(BOLDGREEN "restored: %s" RESET "\n", dst_path);
	}
}

void backup_restore(char *ticker, char *backup_day, char *exp_year)
{
	int x;

	if (ticker) {
		backup_restore_stock(ticker, backup_day, exp_year);
		return;
	}

	for (x=0; x<nr_highcap_options; x++)
		backup_restore_stock(HIGHCAP_OPTIONS[x], backup_day, exp_year);
	for (x=0; x<nr_lowcap_options; x++)
		backup_restore_stock(LOWCAP_OPTIONS[x], backup_day, exp_year);
}
