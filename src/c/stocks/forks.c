#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

#define AIRFORK_CLASS_HYPER   0
#define AIRFORK_CLASS_SUPER   1
#define AIRFORK_CLASS_BIZ     2
#define AIRFORK_CLASS_ECO     3

/* Signal Status */
#define SIGNAL_TURBULENT 0
#define SIGNAL_LANDED    1
#define SIGNAL_TAKEOFF   2
#define SIGNAL_CLIMB     3
#define SIGNAL_CRUISE    4
#define SIGNAL_APPROACH  5

#define MONSTER_RANKS_50   50
#define MONSTER_RANKS_100 100
#define MONSTER_RANKS_150 150
#define MONSTER_RANKS_200 200

char          *AIRCLASS[]   = { "hyper", "super", "biz", "eco" };
int            fork_years[] = { 2018, 2019, 2020, 2021 };
double         avg_nr_positions;

struct stock **stock_hypersonic;
struct stock **stock_supersonic;
struct stock **stock_business;
struct stock **stock_economy;
struct stock **stock_crashed;

int max_hypersonic = 256;
int max_supersonic = 256;
int max_business   = 256;
int max_economy    = 256;
int max_crashed    = 256;

int nr_hypersonic;
int nr_supersonic;
int nr_business;
int nr_economy;
int nr_crashed;

double hypersonic_cutoff;
double supersonic_cutoff;
double business_cutoff;
double economy_cutoff;
double crashed_cutoff;

struct fq fork_quarters[NR_QUARTER_YEARS];

static __inline__ struct fquarter *fork_quarter(char *date, int date_format)
{
	int month, day, year;

	if (date_format == DATE_YMD) {
		if (!splitdate_YMD(date, &year, &month, &day))
			return NULL;
	} else if (date_format == DATE_MDY) {
		if (!splitdate_MDY(date, &year, &month, &day))
			return NULL;
	} else
		return NULL;

	if (month >= 1 && month <= 3)
		return &fork_quarters[year].fquarter[0];

	if (month >= 4 && month <= 6) {
		if (date_format == DATE_YMD && !strncmp(date, "2020-", 5)) {
//			printf("date: %s year: %d month: %d fq: %p\n", date, year, month, &fork_quarters[year].fquarter[1]);
		}
		return &fork_quarters[year].fquarter[1];
	}
	if (month >= 7 && month <= 9)
		return &fork_quarters[year].fquarter[2];
	if (month >= 10 && month <= 12)
		return &fork_quarters[year].fquarter[3];
	return NULL;
}

void rpc_airstocks_portfolio(struct rpc *rpc)
{
	int               fork_class = atoi(rpc->argv[1]);
	struct XLS       *XLS        = CURRENT_XLS;
	struct monster   *monster    = (struct monster *)XLS->MONSTER;
	struct portfolio *portfolios = monster->portfolios;

	switch (fork_class) {
		case AIRFORK_CLASS_SUPER:
			websocket_send(rpc->connection, portfolios->supersonic, portfolios->supersonic_size);
			break;
		case AIRFORK_CLASS_BIZ:
			websocket_send(rpc->connection, portfolios->business, portfolios->business_size);
			break;
		case AIRFORK_CLASS_ECO:
			websocket_send(rpc->connection, portfolios->economy, portfolios->economy_size);
			break;
	}
}

void rpc_airstocks_sigstat(struct rpc *rpc)
{
	struct stock *stock = search_stocks(rpc->argv[1]);
	struct sig   *sig;
	double        sigret;
	int           x, nr_signals, packet_len = 0;

	if (!stock || !stock->signals || !stock->nr_signals || !stock->mag2)
		return;

	nr_signals = stock->nr_signals;
	packet_len = snprintf(rpc->packet, 24, "sigstat %s ", stock->sym);
	for (x=0; x<nr_signals; x++) {
		sig = stock->signals[x];
		if (!sig)
			continue;
		if (sig->exit_price)
			sigret = sig->ret;
		else
			sigret = ((stock->current_price/sig->entry_price)-1)*100;
		packet_len += sprintf(rpc->packet+packet_len, "%s %s %s %.2f %.2f %.2f*", sig->ticker, sig->entry_date, sig->exit_date, sig->entry_price, sig->exit_price, sigret);
	}
	websocket_send(rpc->connection, rpc->packet, packet_len);
}

void rpc_airstocks_fork(struct rpc *rpc)
{
	int             port_id = atoi(rpc->argv[1]);
	struct forkmem *fork;
	struct XLS     *XLS = CURRENT_XLS;

	if (port_id < 0 || port_id > XLS->nr_forks || !XLS->forks)
		return;

	fork = &XLS->forks[port_id];
	if (!fork)
		return;
	websocket_send(rpc->connection, fork->boarding_passes, fork->boarding_passes_len);
}

/*
void stock_FlightClass_JSON(struct stock *stock, int type)
{
	char *sigtab;
	int nr_signals, sigtab_size, x;return;

	sigtab      = (char *)(MONSTER_TABLES+MONSTER_TABLE_SIZE);
	sigtab_size = sprintf(sigtab, "ctable FITAB-%s [", AIRCLASS[type]);
	nr_signals  = stock->nr_signals;
	for (x=0; x<nr_signals; x++) {
		struct sig *sig = stock->signals[x];
		// {"data":"Rank"},{"data":"Sym"},{"data":"ENDate"},{"data":"ENPrice"},{"data":"EXDate"},{"data":"EXPrice"},{"data":"RET"}
		sigtab_size += sprintf(sigtab+sigtab_size, "{\"Rank\":\"%d\",\"Sym\":\"%s\",\"ENDate\":\"%s\",\"ENPrice\":\"%.2f\",\"EXDate\":\"%s\",\"EXPrice\":\"%.2f\",\"RET\":\"%.2f\"},",
				sig->rank, sig->ticker, sig->entry_date, sig->entry_price, sig->exit_date, sig->exit_price, sig->ret);
		sig++;
	}
	sigtab[sigtab_size-1] = ']';
	sigtab[sigtab_size] = 0;
	printf(BOLDCYAN "sigtab_size: %d strlen: %d" RESET "\n", sigtab_size, (int)strlen(sigtab));
	memcpy(MONSTER_TABLES+MONSTER_TABLE_SIZE, sigtab, sigtab_size);
	MONSTER_TABLE_SIZE += sigtab_size;
}*/

void build_flight_classes(struct XLS *XLS)
{
	struct stock *stock;
	struct stock **HIGHCAPS;
	int x, nr_sigstocks = 0, nr_highcaps;
	double total_days = 0;

	stock_hypersonic = (struct stock **)zmalloc(sizeof(struct stock **) * max_hypersonic);
	stock_supersonic = (struct stock **)zmalloc(sizeof(struct stock **) * max_supersonic);
	stock_business   = (struct stock **)zmalloc(sizeof(struct stock **) * max_business);
	stock_economy    = (struct stock **)zmalloc(sizeof(struct stock **) * max_economy);
	stock_crashed    = (struct stock **)zmalloc(sizeof(struct stock **) * max_crashed);

	nr_highcaps = XLS->nr_highcaps;
	HIGHCAPS    = XLS->HIGHCAPS;
	for (x=0; x<nr_highcaps; x++) {
		stock   = HIGHCAPS[x];
		if (!stock->mag || !stock->mag2 || stock->mag->nr_entries != stock->nr_mag2_entries || !stock->nr_signals || !stock->sig_avgdays)
			continue;
		total_days += stock->sig_avgdays;
		nr_sigstocks++;
	}
	total_days /= nr_sigstocks;
	XLS->sigrank_avgdays = total_days;

	crashed_cutoff    = total_days;
	economy_cutoff    = 63.0;
	business_cutoff   = 63.0;
	supersonic_cutoff = 40.0;
	hypersonic_cutoff = 25.0;
	nr_hypersonic     = 0;
	nr_supersonic     = 0;
	nr_business       = 0;
	nr_economy        = 0;

	if (verbose)
		printf(BOLDWHITE "total_days: %.2f hyper: %.2f super: %.2f biz: %.2f eco: %.2f total: %.2f" RESET "\n", total_days, hypersonic_cutoff, supersonic_cutoff, business_cutoff, economy_cutoff, crashed_cutoff);
	for (x=0; x<nr_highcaps; x++) {
		stock = HIGHCAPS[x];
		if (!stock->mag || !stock->mag2 || stock->mag->nr_entries != stock->nr_mag2_entries || !stock->nr_signals)
			continue;
		if (stock->sig_avgdays <= hypersonic_cutoff) {
			if (nr_hypersonic >= max_hypersonic) {
				max_hypersonic += 32;
				stock_hypersonic = (struct stock **)realloc(stock_hypersonic, sizeof(struct stock *) * max_hypersonic);
			}
			stock_hypersonic[nr_hypersonic++] = stock;
			stock->airfork_stock_class = AIRFORK_CLASS_HYPER;
			continue;
		}

		if (stock->sig_avgdays <= supersonic_cutoff) {
			if (nr_supersonic >= max_supersonic) {
				max_supersonic += 32;
				stock_supersonic = (struct stock **)realloc(stock_supersonic, sizeof(struct stock *) * max_supersonic);
			}
			stock_supersonic[nr_supersonic++] = stock;
			stock->airfork_stock_class = AIRFORK_CLASS_SUPER;
			continue;
		}

		if (stock->sig_avgdays <= business_cutoff) {
			if (nr_business >= max_business) {
				max_business += 32;
				stock_business = (struct stock **)realloc(stock_business, sizeof(struct stock *) * max_business);
			}
			stock_business[nr_business++] = stock;
			stock->airfork_stock_class = AIRFORK_CLASS_BIZ;
			continue;
		}

		if (stock->sig_avgdays > economy_cutoff) {
			if (nr_economy >= max_economy) {
				max_economy += 32;
				stock_economy = (struct stock **)realloc(stock_economy, sizeof(struct stock *) * max_economy);
			}
			stock_economy[nr_economy++] = stock;
			stock->airfork_stock_class = AIRFORK_CLASS_ECO;
			continue;
		}
/*		if (stock->sig_avgdays <= crashed_cutoff) {
			if (nr_crashed >= max_crashed) {
				max_crashed += 32;
				stock_crashed = (struct stock **)realloc(stock_crashed, sizeof((struct stock *) * max_crashed);
			}
			stock_crashed[nr_crashed++] = stock;
			stock->airfork_stock_class = AIRFORK_CLASS_CRASHED;
			stock_FlightClass_JSON(stock, AIRFORK_CLASS_CRASHED);
			continue;
		}
*/
	}
}

char *filtered_tickers[] = { "CAE", "CNF", "ELAN", "GRPN", "IMOS", "JHX", "LPI", "TRMD", "VHI" };

int is_filtered(char *ticker)
{
	int x;

	for (x=0; x<sizeof(filtered_tickers)/8; x++)
		if (!strcmp(ticker, filtered_tickers[x]))
			return 1;
	return 0;
}

void zero_signals(struct monster *monster)
{
	/* 2020 */
	fork_quarters[0].fquarter[0].nr_signals_63d  = 0;
	fork_quarters[0].fquarter[0].nr_signals_128d = 0;
	fork_quarters[0].fquarter[0].nr_signals_dead = 0;

	fork_quarters[0].fquarter[0].end   = "3/31/2020";
	fork_quarters[0].fquarter[1].start = "4/1/2020";
	fork_quarters[0].fquarter[1].end   = "6/30/2020";
	fork_quarters[0].fquarter[2].start = "7/1/2020";
	fork_quarters[0].fquarter[2].end   = "9/30/2020";
	fork_quarters[0].fquarter[3].start = "10/1/2020";
	fork_quarters[0].fquarter[3].end   = "12/31/2020";
	/* 2021 */
	fork_quarters[1].fquarter[0].start = "1/1/2021";
	fork_quarters[1].fquarter[0].end   = "3/31/2021";
	fork_quarters[1].fquarter[1].start = "4/1/2021";
	fork_quarters[1].fquarter[1].end   = "6/30/2021";
	fork_quarters[1].fquarter[2].start = "7/1/2021";
	fork_quarters[1].fquarter[2].end   = "9/30/2021";

}

void monster_signal_completed(struct fquarter *fquarter, int nr_days, int rank, char *date)
{
	switch (rank) {
		case MONSTER_RANKS_50:
			if (!strncmp(date, "2020-", 5)) {
				int m = atoi(date+5);
				if (verbose && m >= 7 && m <= 9)
					printf(BOLDGREEN "monster_signal_completed: %s nr_days: %d ranks: %d" RESET "\n", date, nr_days, rank);
			}
			fquarter->nr_signals_rank50++;
			if (nr_days <= 63) {
				fquarter->nr_signals_rank50_63d++;
				fquarter->nr_signals_rank50_128d++;
			} else if (nr_days > 63 && nr_days <= 128)
				fquarter->nr_signals_rank50_128d++;
			else
				fquarter->nr_signals_rank50_dead++;
			break;
		case MONSTER_RANKS_100:
			if (!strncmp(date, "2020-", 5)) {
				int m = atoi(date+5);
				if (verbose && m >= 7 && m <= 9)
					printf(BOLDBLUE "monster_signal_completed: %s nr_days: %d ranks: %d" RESET "\n", date, nr_days, rank);
			}
			fquarter->nr_signals_rank100++;
			if (nr_days <= 63) {
				fquarter->nr_signals_rank100_63d++;
				fquarter->nr_signals_rank100_128d++;
			} else if (nr_days > 63 && nr_days <= 128)
				fquarter->nr_signals_rank100_128d++;
			else
				fquarter->nr_signals_rank100_dead++;
			break;
		case MONSTER_RANKS_150:
			if (!strncmp(date, "2020-", 5)) {
				int m = atoi(date+5);
				if (verbose && m >= 7 && m <= 9)
					printf(BOLDWHITE "monster_signal_completed: %s nr_days: %d ranks: %d" RESET "\n", date, nr_days, rank);
			}
			fquarter->nr_signals_rank150++;
			if (nr_days <= 63) {
				fquarter->nr_signals_rank150_63d++;
				fquarter->nr_signals_rank150_128d++;
			} else if (nr_days > 63 && nr_days <= 128)
				fquarter->nr_signals_rank150_128d++;
			else
				fquarter->nr_signals_rank150_dead++;
			break;
		case MONSTER_RANKS_200:
			if (!strncmp(date, "2020-", 5)) {
				int m = atoi(date+5);
				if (verbose && m >= 7 && m <= 9)
					printf(BOLDYELLOW "monster_signal_completed: %s nr_days: %d ranks: %d" RESET "\n", date, nr_days, rank);
			}
			fquarter->nr_signals_rank200++;
			if (nr_days <= 63) {
				fquarter->nr_signals_rank200_63d++;
				fquarter->nr_signals_rank200_128d++;
			} else if (nr_days > 63 && nr_days <= 128)
				fquarter->nr_signals_rank200_128d++;
			else
				fquarter->nr_signals_rank200_dead++;
			break;
	}
}

void monster_signal_pending(struct fquarter *fquarter, int nr_days, int rank)
{
	switch (rank) {
		case MONSTER_RANKS_50:
			fquarter->nr_pending_rank50++;
			break;
		case MONSTER_RANKS_100:
			fquarter->nr_pending_rank100++;
			break;
		case MONSTER_RANKS_150:
			fquarter->nr_pending_rank150++;
			break;
		case MONSTER_RANKS_200:
			fquarter->nr_pending_rank200++;
			break;
	}
}

void store_signals(struct sig *signals, int nr_signals)
{
	fs_newfile("db/signals.db", (void *)signals, nr_signals*sizeof(*signals));
}

// cat python/recursion/2020.csv | tr "," "-" | sort -n -t "-" -k3 -k4 -k5|gawk -F"-" '{print $1","$2","$3"-"$4"-"$5","$6","$7"-"$8"-"$9","$10}'| sed 's/--//g' | sed 's/Rank.*,,/Rank,Ticker,Entry Date,Entry Price,Exit Date,Exit Price/g'|grep -v ,,,, > /dev/shm/2.csv
// table #3
void init_signals(struct XLS *XLS)
{
	struct stock    *stock;
	struct stock   **HIGHCAPS;
	struct monster  *monster;
	struct mag      *mag;
	struct mag2     *m2;
	struct sig      *sig, *signals, **stock_signals;
	char            *entry_date, *exit_date, *signals_csv;
	int              x, z, start_day, entry_day, nr_days, nr_entries, nr_stock_signals, max_signals, rank;
	int              signals_csv_len = 0, nr_highcaps;
	int64_t          nr_signals = 0;
	double           entry_price, exit_price, pc, sigcompleted = 0, close_pc, sig_avgdays;

	if (!XLS->config->production)
		return;
	nr_highcaps = XLS->nr_highcaps;
	HIGHCAPS    = XLS->HIGHCAPS;
	monster     = (struct monster *)XLS->MONSTER;
	if ((signals=(struct sig *)fs_mallocfile("db/signals.db", &nr_signals))) {
		printf(BOLDCYAN "LOADING EXISTING SIGNALS.DB monster: %p" RESET "\n", monster);
		nr_signals /= sizeof(struct sig);
		max_signals = 4;
		for (x=0; x<nr_signals; x++) {
			sig = &signals[x];
			sig->stock = stock = search_stocks_XLS(XLS, sig->ticker);
			if (!sig->stock)
				continue;
			stock->nr_signals++;
			stock->sig_avgdays += sig->nr_days;
			if (!stock->signals)
				stock->signals  = (struct sig **)zmalloc(sizeof(struct sig *) * max_signals);
			if (stock->nr_signals >= max_signals) {
				max_signals   += 4;
				stock->signals = (struct sig **)realloc(stock->signals, sizeof(struct sig *) * max_signals);
			}
			stock->signals[stock->nr_signals-1] = sig;
		}
		for (x=0; x<nr_highcaps; x++) {
			stock = HIGHCAPS[x];
			if (!stock || !stock->nr_signals || !stock->mag || !stock->mag2 ||is_index(stock->sym) || is_fund(stock->sym) || is_filtered(stock->sym) || !stock->rank)
				continue;
			stock->sig_avgdays /= stock->nr_signals;
		}
		monster->signals    = signals;
		monster->nr_signals = nr_signals;
		build_flight_classes(XLS);
		return;
	}

	// update_EOD() PATH AFTER SIGNALS.DB IS UNLINKED
	printf(BOLDCYAN "CREATING NEW SIGNALS.DB" RESET "\n");
	signals_csv     = (char *)malloc(1024 KB);
	signals_csv_len = sprintf(signals_csv, "%s\n","Rank,Ticker,Entry Date,Entry Price,Exit Date,Exit Price");
	signals         = (struct sig *)malloc(sizeof(struct sig) * 4096);
	sig             = signals;
	zero_signals(monster);

	for (x=0; x<nr_highcaps; x++) {
		stock = HIGHCAPS[x];
		if (!stock || !stock->mag || !stock->mag2 ||is_index(stock->sym) || is_fund(stock->sym) || is_filtered(stock->sym) || !stock->rank)
			continue;
		if (stock->nr_mag2_entries != stock->mag->nr_entries)
			continue;
		mag              = stock->mag;
		start_day        = mag->year_2020;
		nr_entries       = mag->nr_entries;
		nr_stock_signals = 0;
		max_signals      = 4;
		stock_signals    = (struct sig **)zmalloc(sizeof(struct sig *) * max_signals);
		sig_avgdays      = 0.0;
		if (start_day < 0)
			continue;
		for (entry_day=start_day; entry_day<nr_entries; entry_day++) {
			rank = date_to_rank(stock, mag->date[entry_day]);
			m2 = &stock->mag2[entry_day];
			/* Check if the stock Signalled on this day */
			if (rank > 200 || m2->sig == 0.0)
				continue;
			/* Scan ahead until 10% is reached */
			entry_date      = mag->date[entry_day];
			entry_price     = mag->close[entry_day];
			nr_days         = nr_entries-entry_day;
			sig->stock      = stock;
			sig->rank       = rank;
			sig->entry      = entry_day;
			*(unsigned long  *)(sig->ticker)       = *(unsigned long  *)(stock->sym);
			*(unsigned long  *)(sig->entry_date)   = *(unsigned long  *)(entry_date);
			*(unsigned short *)(sig->entry_date+8) = *(unsigned short *)(entry_date+8);
			nr_signals     += 1;
			sigcompleted    = 0;
			for (z=0; z<nr_days; z++) {
				exit_price = mag->close[entry_day+z+1];
				pc         = ((exit_price/entry_price)-1)*100.0;
				if (pc >= 10.0) {
					exit_date        = mag->date[entry_day+z+1];
					signals_csv_len += sprintf(signals_csv+signals_csv_len, "%d,%s,%s,%.2f,%s,%.2f\n", rank, stock->sym, mag->date[entry_day], entry_price, exit_date, exit_price);
					*(unsigned long  *)(sig->exit_date)   = *(unsigned long  *)(exit_date);
					*(unsigned short *)(sig->exit_date+8) = *(unsigned short *)(exit_date+8);
					sig->entry_price = entry_price;
					sig->exit_price  = exit_price;
					sig->ret         = ((exit_price/entry_price)-1)*100.0;
					sigcompleted     = 1;
					sig->nr_days     = z;
					sig_avgdays     += z;
					stock_signals[nr_stock_signals++] = sig;
					if (nr_stock_signals >= max_signals) {
						max_signals += 4;
						stock_signals = (struct sig **)realloc(stock_signals, sizeof(struct sig *) * max_signals);
					}
					if (verbose && (!strcmp(stock->sym, "FTNT") || !strcmp(stock->sym, "AAPL") || !strcmp(stock->sym, "ENPH") || !strcmp(stock->sym, "ACMR") || !strcmp(stock->sym, "CVNA") || !strcmp(stock->sym, "DAVA")))
						printf(BOLDGREEN "%s nr_days: %d curavg: %.2f" RESET "\n", stock->sym, z,(sig_avgdays/nr_stock_signals));
					sig->status = SIGNAL_LANDED;
					sig++;
					/* Rank Saftey */
					if (rank <= 50)
						monster_signal_completed(fork_quarter(entry_date, DATE_YMD), z, MONSTER_RANKS_50, entry_date);
					else if (rank > 51 && rank <= 100)
						monster_signal_completed(fork_quarter(entry_date, DATE_YMD), z, MONSTER_RANKS_100,entry_date);
					else if (rank > 101 && rank <= 150)
						monster_signal_completed(fork_quarter(entry_date, DATE_YMD), z, MONSTER_RANKS_150,entry_date);
					else if (rank > 151 && rank <= 200)
						monster_signal_completed(fork_quarter(entry_date, DATE_YMD), z, MONSTER_RANKS_200,entry_date);
					break;
				}
			}
			if (!sigcompleted) {
				signals_csv_len += sprintf(signals_csv+signals_csv_len, "%d,%s,%s,%.2f,,\n", rank, stock->sym, mag->date[entry_day], entry_price);
				exit_date        = "Pending";
				*(unsigned long  *)(sig->exit_date) = *(unsigned long  *)(exit_date);
				sig->entry_price = entry_price;
				sig->exit_price  = 0.0;
				sig->ret         = 0.0;
				sig->nr_days     = nr_entries-entry_day;
				sig_avgdays     += sig->nr_days;
				stock_signals[nr_stock_signals++] = sig;
				if (nr_stock_signals >= max_signals) {
					max_signals += 4;
					stock_signals = (struct sig **)realloc(stock_signals, sizeof(struct sig *) * max_signals);
				}
				close_pc = ((entry_price/stock->current_price)-1)*100.0;
				if (close_pc < -5.0)
					sig->status = SIGNAL_TURBULENT;
				else if (close_pc >= 0.0 && close_pc < 2.0)
					sig->status = SIGNAL_TAKEOFF;
				else if (close_pc >= 2.0 && close_pc < 5.0)
					sig->status = SIGNAL_CLIMB;
				else if (close_pc >= 5.0 && close_pc < 8.0)
					sig->status = SIGNAL_CRUISE;
				else if (close_pc >= 8.0)
					sig->status = SIGNAL_APPROACH;
				if (verbose && (!strcmp(stock->sym, "FTNT") || !strcmp(stock->sym, "AAPL") || !strcmp(stock->sym, "ENPH") || !strcmp(stock->sym, "ACMR") || !strcmp(stock->sym, "CVNA") || !strcmp(stock->sym, "DAVA")))
					printf(BOLDRED "%s nr_days: %d curavg: %.2f" RESET "\n", stock->sym, z,(sig_avgdays/nr_stock_signals));

				/* Rank Saftey */
				if (rank <= 50)
					monster_signal_pending(fork_quarter(entry_date, DATE_YMD), z, MONSTER_RANKS_50);
				else if (rank > 51 && rank <= 100)
					monster_signal_pending(fork_quarter(entry_date, DATE_YMD), z, MONSTER_RANKS_100);
				else if (rank > 101 && rank <= 150)
					monster_signal_pending(fork_quarter(entry_date, DATE_YMD), z, MONSTER_RANKS_150);
				else if (rank > 151 && rank <= 200)
					monster_signal_pending(fork_quarter(entry_date, DATE_YMD), z, MONSTER_RANKS_200);
				sig++;
			}
		}
		stock->signals    = stock_signals;
		stock->nr_signals = nr_stock_signals;
		if (nr_stock_signals)
			stock->sig_avgdays = (sig_avgdays/nr_stock_signals);
	}
	// EOD PATH
	fs_newfile("python/recursion/2020.csv", signals_csv, signals_csv_len);
	monster->nr_signals = nr_signals;
	monster->signals    = signals;
	build_flight_classes(XLS);
	build_flight_info(monster);
	store_signals(signals,nr_signals);
}

// table 3
void build_flight_info(struct monster *monster)
{
	struct sig *sig = monster->signals;
	int x, max_table_size = 3200 KB, nr_signals = monster->nr_signals;

	monster->table3 = (char *)malloc(3200 KB);
	strcpy(monster->table3, "ctable FDT [");
	monster->table3_size = 12;
	for (x=0; x<nr_signals; x++) {
		// {"data":"Rank"},{"data":"Sym"},{"data":"ENDate"},{"data":"ENPrice"},{"data":"EXDate"},{"data":"EXPrice"},{"data":"RET"}
		monster->table3_size += sprintf(monster->table3+monster->table3_size, "{\"Rank\":\"%d\",\"Sym\":\"%s\",\"ENDate\":\"%s\",\"ENPrice\":\"%.2f\",\"EXDate\":\"%s\",\"EXPrice\":\"%.2f\",\"RET\":\"%.2f\",\"STAT\":\"%d\",\"CLASS\":\"%d\"},",
				sig->rank, sig->ticker, sig->entry_date, sig->entry_price, sig->exit_date, sig->exit_price, sig->ret, sig->status, sig->stock->airfork_stock_class);
		sig++;
		if (monster->table3_size + 1024 >= max_table_size) {
			max_table_size += 2048;
			monster->table3 = (char *)realloc(monster->table3, max_table_size);
		}
	}
	monster->table3[monster->table3_size-1] = ']';
	monster->table3[monster->table3_size++] = '@';
}

// old flight saftey table
void old_flight_saftey_table(struct monster *monster)
{
	struct fquarter  *fquarter;
	int x, y;

	monster->table5 = (char *)malloc(64 KB);
	strcpy(monster->table5, "@qf QF [");
	monster->table5_size = 8;
	for (x=0; x<NR_QUARTER_YEARS; x++) {
		struct fq *fq = &fork_quarters[x];
		for (y=0; y<4; y++) {
			fquarter = &fq->fquarter[y];
			if (!fquarter->start)
				continue;

			if (fquarter->nr_APY_SP500_5pc)
				fquarter->APY_SP500_5pc = ((double)fquarter->nr_APY_SP500_5pc/(double)fquarter->total)*100.0;
			if (fquarter->nr_APY_SP500)
				fquarter->APY_SP500     = ((double)fquarter->nr_APY_SP500/(double)fquarter->total)*100.0;
			if (fquarter->nr_APY_10pc)
				fquarter->APY_10pc      = ((double)fquarter->nr_APY_10pc/(double)fquarter->total)*100.0;

			if (fquarter->nr_RTD_SP500_5pc)
				fquarter->RTD_SP500_5pc = ((double)fquarter->nr_RTD_SP500_5pc/(double)fquarter->total)*100.0;
			if (fquarter->nr_RTD_SP500)
				fquarter->RTD_SP500     = ((double)fquarter->nr_RTD_SP500/(double)fquarter->total)*100.0;
			if (fquarter->nr_RTD_10pc)
				fquarter->RTD_10pc      = ((double)fquarter->nr_RTD_10pc/(double)fquarter->total)*100.0;

			monster->table5_size += sprintf(monster->table5+monster->table5_size, "{\"YEAR\":\"%d\",\"Q\":\"Q%d\",\"S\":\"%s\",\"E\":\"%s\",\"T\":\"%d\","
										   "\"APY1\":\"%d\",\"APY2\":\"%d\",\"APY3\":\"%d\","
										   "\"RTD1\":\"%d\",\"RTD2\":\"%d\",\"RTD3\":\"%d\","
										   "\"APY4\":\"%.2f\",\"APY5\":\"%.2f\",\"APY6\":\"%.2f\","
				                           "\"RTD4\":\"%.2f\",\"RTD5\":\"%.2f\",\"RTD6\":\"%.2f\"},",
				                           fork_years[x], y+1, fquarter->start, fquarter->end, fquarter->total,
										   fquarter->nr_APY_SP500_5pc, fquarter->nr_APY_SP500, fquarter->nr_APY_10pc,
				                           fquarter->nr_RTD_SP500_5pc, fquarter->nr_RTD_SP500, fquarter->nr_RTD_10pc,
				                           fquarter->APY_SP500_5pc,    fquarter->APY_SP500,    fquarter->APY_10pc,
										   fquarter->RTD_SP500_5pc,    fquarter->RTD_SP500,    fquarter->RTD_10pc);
		}
	}
}

void flight_saftey_table(struct monster *monster)
{
	int nr_signals_total,   nr_pending_total,   nr_signals_total_63d,   nr_signals_total_128d,   nr_signals_total_dead;
	int nr_signals_rank50,  nr_pending_rank50,  nr_signals_rank50_63d,  nr_signals_rank50_128d,  nr_signals_rank50_dead;
	int nr_signals_rank100, nr_pending_rank100, nr_signals_rank100_63d, nr_signals_rank100_128d, nr_signals_rank100_dead;
	int nr_signals_rank150, nr_pending_rank150, nr_signals_rank150_63d, nr_signals_rank150_128d, nr_signals_rank150_dead;
	int nr_signals_rank200, nr_pending_rank200, nr_signals_rank200_63d, nr_signals_rank200_128d, nr_signals_rank200_dead;
	int x, y, max_table_size = 64 KB;

	monster->table5 = (char *)malloc(64 KB);
	strcpy(monster->table5, "@qf QF [");
	monster->table5_size = 8;
	for (x=0; x<NR_QUARTER_YEARS; x++) {
		struct fq *fq = &fork_quarters[x];
		for (y=0; y<4; y++) {
			struct fquarter *fquarter = &fq->fquarter[y];
			/* Ranks 0..50 */
			nr_signals_rank50       = fquarter->nr_signals_rank50;
			nr_pending_rank50       = fquarter->nr_pending_rank50;
			nr_signals_rank50_63d   = fquarter->nr_signals_rank50_63d;
			nr_signals_rank50_128d  = fquarter->nr_signals_rank50_128d;
			nr_signals_rank50_dead  = fquarter->nr_signals_rank50_dead;
			/* Ranks 51..100 */
			nr_signals_rank100      = fquarter->nr_signals_rank100;
			nr_pending_rank100      = fquarter->nr_pending_rank100;
			nr_signals_rank100_63d  = fquarter->nr_signals_rank100_63d;
			nr_signals_rank100_128d = fquarter->nr_signals_rank100_128d;
			nr_signals_rank100_dead = fquarter->nr_signals_rank100_dead;
			/* Ranks 101..150 */
			nr_signals_rank150      = fquarter->nr_signals_rank150;
			nr_pending_rank150      = fquarter->nr_pending_rank150;
			nr_signals_rank150_63d  = fquarter->nr_signals_rank150_63d;
			nr_signals_rank150_128d = fquarter->nr_signals_rank150_128d;
			nr_signals_rank150_dead = fquarter->nr_signals_rank150_dead;
			/* Ranks 151..200 */
			nr_signals_rank200      = fquarter->nr_signals_rank200;
			nr_pending_rank200      = fquarter->nr_pending_rank200;
			nr_signals_rank200_63d  = fquarter->nr_signals_rank200_63d;
			nr_signals_rank200_128d = fquarter->nr_signals_rank200_128d;
			nr_signals_rank200_dead = fquarter->nr_signals_rank200_dead;

			nr_signals_total        = nr_signals_rank50      + nr_signals_rank100      + nr_signals_rank150      + nr_signals_rank200;
			nr_pending_total        = nr_pending_rank50      + nr_pending_rank100      + nr_pending_rank150      + nr_pending_rank200;
			nr_signals_total_63d    = nr_signals_rank50_63d  + nr_signals_rank100_63d  + nr_signals_rank150_63d  + nr_signals_rank200_63d;
			nr_signals_total_128d   = nr_signals_rank50_128d + nr_signals_rank100_128d + nr_signals_rank150_128d + nr_signals_rank200_128d;
			nr_signals_total_dead   = nr_signals_rank50_dead + nr_signals_rank100_dead + nr_signals_rank150_dead + nr_signals_rank200_dead;

			monster->table5_size += snprintf(monster->table5+monster->table5_size, 2048,
			"{\"T_1\":\"%d\",\"P_1\":\"%d\",\"63d_1\":\"%d\",\"128d_1\":\"%d\",\"dead_1\":\"%d\","
			 "\"T_2\":\"%d\",\"P_2\":\"%d\",\"63d_2\":\"%d\",\"128d_2\":\"%d\",\"dead_2\":\"%d\","
			 "\"T_3\":\"%d\",\"P_3\":\"%d\",\"63d_3\":\"%d\",\"128d_3\":\"%d\",\"dead_3\":\"%d\","
			 "\"T_4\":\"%d\",\"P_4\":\"%d\",\"63d_4\":\"%d\",\"128d_4\":\"%d\",\"dead_4\":\"%d\","
			 "\"T_5\":\"%d\",\"P_5\":\"%d\",\"63d_5\":\"%d\",\"128d_5\":\"%d\",\"dead_5\":\"%d\"},",
			nr_signals_total,   nr_pending_total,   nr_signals_total_63d,   nr_signals_total_128d,   nr_signals_total_dead,
			nr_signals_rank50,  nr_pending_rank50,  nr_signals_rank50_63d,  nr_signals_rank50_128d,  nr_signals_rank50_dead,
			nr_signals_rank100, nr_pending_rank100, nr_signals_rank100_63d, nr_signals_rank100_128d, nr_signals_rank100_dead,
			nr_signals_rank150, nr_pending_rank150, nr_signals_rank150_63d, nr_signals_rank150_128d, nr_signals_rank150_dead,
			nr_signals_rank200, nr_pending_rank200, nr_signals_rank200_63d, nr_signals_rank200_128d, nr_signals_rank200_dead);			
			if (monster->table5_size + 1024 >= max_table_size) {
				max_table_size += 2048;
				monster->table5 = (char *)realloc(monster->table5, max_table_size);
			}
		} // for (y<4)
	} // for (x<2)
	*(char *)(monster->table5+monster->table5_size-1) = ']';
}

void load_forks(struct XLS *XLS)
{
	struct forkmem   *fork;
	struct forkmem   *forks;
	struct monster   *monster;
	struct fquarter  *fquarter;
	struct portfolio *portfolios;
	char             *list[128];
	char             *buf, *ports, *port, *p, *p2, *end, *forktab, *RTD, *APY;
	char              RTDbuf[64];
	char              APYbuf[64];
	char              buy_date[16]  = {0};
	char              sell_date[16] = {0};
	int               nr_items, x, y, z, fork_size, pos, max_size, nr_forks = 0;
	int               max_hypersonic_size, max_supersonic_size, max_business_size, max_economy_size, boarding_passes_max;
	time_t            unix_start_date, unix_end_date;
	double            capital, GSPC_start, GSPC_end, GSPC_RTD, GSPC_APY, nr_days = 0.0, sigret;

	unlink(FORKS_DB);
	monster    = (struct monster *)XLS->MONSTER;
	portfolios = (struct portfolio *)zmalloc(sizeof(*portfolios));
	*(char *)(portfolios->hypersonic = (char *)malloc(1024 KB)) = 0;
	*(char *)(portfolios->business   = (char *)malloc(1024 KB)) = 0;
	*(char *)(portfolios->economy    = (char *)malloc(1024 KB)) = 0;
	*(char *)(portfolios->supersonic = (char *)malloc(1024 KB)) = 0;
	strcpy(portfolios->hypersonic, "ctable PORTS [");
	portfolios->hypersonic_size = 14;
	strcpy(portfolios->supersonic, "ports 1 [");
	portfolios->supersonic_size = 9;
	strcpy(portfolios->business,   "ports 2 [");
	portfolios->business_size   = 9;
	strcpy(portfolios->economy,    "ports 3 [");
	portfolios->economy_size    = 9;

 	max_hypersonic_size = 1024 KB;
 	max_supersonic_size = 1024 KB;
 	max_business_size   = 1024 KB;
 	max_economy_size    = 1024 KB;

	buf = fs_mallocfile_str("python/recursion/2020.txt", NULL);
	if (!buf)
		return;

	port     = ports = buf;
	while ((p=strstr(port, "Port"))) {
		nr_forks++;
		port = p + 10;
	}

	XLS->forks    = forks = (struct forkmem *)zmalloc(sizeof(struct forkmem) * nr_forks);
	XLS->nr_forks = nr_forks;
	ports = buf;
	for (z=0; z<nr_forks; z++) {
		fork = &forks[z];
		port=strstr(ports, "Port");
		if (!port)
			break;
		fork->port_id = atoi(port+14);
		fork->avgdays = 0.0;
		nr_days = 0.0;

		/* Tickers */
		p = strstr(port, "ti");
		if (!p)
			continue;
		if (*(p+8) == '[') {
			nr_items = fork_list(p+10, &list[0], &end);
			if (!nr_items)
				continue;
			fork->tickers = (char **)zmalloc(sizeof(char *) * nr_items);
			for (x=0; x<nr_items; x++)
				fork->tickers[x] = strdup(list[x]);
			fork->nr_positions = nr_items;
		} else {
			fork->tickers      = (char **)malloc(sizeof(char *));
			p2 = strchr(p+8, '\n');
			if (!p2)
				continue;
			*p2++ = 0;
			fork->tickers[0]   = strdup(p+8);
			fork->nr_positions = 1;
			end = p2;
		}

		/* qty */
		p = end;
		p = strstr(p, "qty");
		if (!p)
			continue;
		if (*(p+5) == '[') {
			nr_items = fork_list(p+7, &list[0], &end);
			if (!nr_items)
				continue;
			fork->qty = (int *)zmalloc(sizeof(int *) * nr_items);
			for (x=0; x<nr_items; x++)
				fork->qty[x] = atoi(list[x]);
		} else {
			fork->qty        = (int *)malloc(sizeof(int));
			fork->qty[0]     = atoi(p+5);
			end = p + 6;
		}

		/* buy_amount */
		p = end;
		p = strstr(p, "buy am");
		if (!p)
			continue;
		if (*(p+9) == '[') {
			nr_items = fork_list(p+11, &list[0], &end);
			if (!nr_items)
				continue;
			fork->buy_amount = (double *)zmalloc(sizeof(double *) * nr_items);
			for (x=0; x<nr_items; x++)
				fork->buy_amount[x] = strtod(list[x], NULL);
		} else {
			fork->buy_amount        = (double *)malloc(sizeof(double));
			fork->buy_amount[0]     = strtod(p+9, NULL);
			end = p + 10;
		}

		/* sell_amount */
		p = end;
		p = strstr(p, "sell a");
		if (!p)
			continue;
		if (*(p+10) == '[') {
			nr_items = fork_list(p+12, &list[0], &end);
			if (!nr_items)
				continue;
			fork->sell_amount = (double *)zmalloc(sizeof(double *) * nr_items);
			for (x=0; x<nr_items; x++)
				fork->sell_amount[x] = strtod(list[x], NULL);
		} else {
			fork->sell_amount    = (double *)malloc(sizeof(double));
			fork->sell_amount[0] = strtod(p+10, NULL);
		}

		/* buy_date */
		p = end;
		p = strstr(p, "buy d");
		if (!p)
			continue;
		if (*(p+10) == '[') {
			nr_items = fork_list(p+12, &list[0], &end);
			if (!nr_items)
				continue;
			fork->buy_date = (char **)zmalloc(sizeof(char *) * nr_items);
			for (x=0; x<nr_items; x++) {
				if (verbose) printf("[%d] buy_date[%d]  = %s list[x]: %s\n",nr_items, x, fork->buy_date[x], list[x]);
				fork->buy_date[x] = strdup(list[x]);
			}
		} else {
			fork->buy_date     = (char **)malloc(sizeof(char *));
			fork->buy_date[0]  = (char  *)malloc(12);
			*(fork->buy_date[0]+8) = 0;
			memcpy(fork->buy_date[0], p+10, 8);
			end = p + 8;
		}

		/* sell_date */
		p = end;
		p = strstr(p, "sell d");
		if (!p)
			continue;
		if (*(p+11) == '[') {
			nr_items = fork_list(p+13, &list[0], &end);
			if (!nr_items)
				continue;
			fork->sell_date = (char **)zmalloc(sizeof(char *) * nr_items);
			if (!fork->nr_days)
				fork->nr_days = (int *)zmalloc(sizeof(int) * nr_items);
			for (x=0; x<nr_items; x++) {
				struct stock *stock = search_stocks(fork->tickers[x]);
				if (!stock || !stock->mag) {
					fork->nr_positions = 0;
					break;
				}
				if (*list[x] == 'i') {
					fork->sell_date[x] = "In-Flight";
					fork->nr_days[x]   = count_market_days(stock->mag, MDY2YMD(fork->buy_date[x], buy_date), QDATE[0]);
				} else {
					fork->sell_date[x] = strdup(list[x]);
					fork->nr_days[x]   = count_market_days(stock->mag, MDY2YMD(fork->buy_date[x], buy_date), MDY2YMD(fork->sell_date[x], sell_date));
				}
			}
			if (!fork->nr_positions)
				continue;
		} else {
			fork->sell_date     = (char **)malloc(sizeof(char *));
			fork->sell_date[0]  = (char  *)malloc(2);
			fork->sell_date[0]  = "-";
			end = p + 2;
		}

		/* cash */
		p = end;
		p = strstr(p, "cash");
		if (!p)
			continue;
		if (*(p+6) == '[') {
			nr_items = fork_list(p+8, &list[0], &end);
			if (!nr_items)
				continue;
			fork->cash = (double *)zmalloc(sizeof(double) * nr_items);
			for (x=0; x<nr_items; x++)
				fork->cash[x] = strtod(list[x], NULL);
		} else {
			fork->cash     = (double *)zmalloc(sizeof(double));
			fork->cash[0]  = strtod(p+6, NULL);
			end = p+7;
		}

		/* capital */
		p = end;
		p = strstr(p, "cap");
		if (!p)
			continue;
		if (*(p+9) == '[') {
			nr_items = fork_list2(p+10, &list[0], &end);
			if (!nr_items)
				continue;
			fork->capital = (double *)zmalloc(sizeof(double *) * nr_items);
			if (verbose)
				printf(BOLDRED "capital nr_items: %d" RESET "\n", nr_items);
			for (x=0; x<nr_items; x++)
				if (*list[x] == 'n')
					fork->capital[x] = 0.0;
				else {
					fork->capital[x] = strtod(list[x], NULL);
					if (verbose)
						printf("list[%d]: %s CAPITAL: %.2f\n", x, list[x], fork->capital[x]);
				}
		} else {
			fork->capital    = (double *)zmalloc(sizeof(double *));
			if (*(p+9) == 'n')
				fork->capital[0] = 0.0;
			else
				fork->capital[0] = strtod(p+9, NULL);
			end = p + 10;
		}

		/* returns */
		p = end;
		p = strstr(p, "ret");
		if (!p)
			continue;
		if (*(p+9) == '[') {
			p += 9;
			fork->RTD = strtod(p+1, NULL);
			p = strchr(p, ',');
			if (!p)
				continue;
			fork->APY = strtod(p+1, NULL);
			if (*(p+1) == '\'')
				fork->APY = 0.0;
			else
				fork->APY = strtod(p+1, NULL);
		} else {
			fork->RTD = 0.0;
			fork->APY = 0.0;
		}
		ports = p + 10;
		/* Calculate Portfolio Class */
		if (fork->nr_days) {
			for (x=0; x<fork->nr_positions; x++)
				nr_days += fork->nr_days[x];
			fork->avgdays = (nr_days/fork->nr_positions);
			if (fork->avgdays <= 30.0) {
				fork->fork_class = AIRFORK_CLASS_HYPER;
				portfolios->nr_hypersonic_ports++;
			} else if (fork->avgdays >= 31.0 && fork->avgdays < 64.0) {
				fork->fork_class = AIRFORK_CLASS_SUPER;
				portfolios->nr_supersonic_ports++;
			} else if (fork->avgdays > 64.0 && fork->avgdays < 80.0) {
				fork->fork_class = AIRFORK_CLASS_BIZ;
				portfolios->nr_business_ports++;
			} else if (fork->avgdays >= 80.0) {
				fork->fork_class = AIRFORK_CLASS_ECO;
				portfolios->nr_economy_ports++;
			}
		}

		/* table #4.1 */
		fork->fork_table = forktab = (char *)malloc(32 KB);
		strcpy(forktab, "fork <tr>");
		fork_size = 9;
		fork_size += sprintf(forktab+fork_size, "<td>Stock</td>");

		for (x=0; x<fork->nr_positions; x++)
			fork_size += sprintf(forktab+fork_size, "<td>%s</td>",   fork->tickers[x]);
		strcpy(forktab+fork_size, "</tr><tr>");
		fork_size += 9;
		fork_size += sprintf(forktab+fork_size, "<td>Qty</td>");
		for (x=0; x<fork->nr_positions; x++)
			fork_size += sprintf(forktab+fork_size, "<td>%d</td>",   fork->qty[x]);
		strcpy(forktab+fork_size, "</tr><tr>");
		fork_size += 9;
		fork_size += sprintf(forktab+fork_size, "<td>BuyAmount</td>");
		for (x=0; x<fork->nr_positions; x++)
			fork_size += sprintf(forktab+fork_size, "<td>%.2f</td>", fork->buy_amount[x]);
		strcpy(forktab+fork_size, "</tr><tr>");
		fork_size += 9;
		fork_size += sprintf(forktab+fork_size, "<td>SellAmount</td>");
		for (x=0; x<fork->nr_positions; x++)
			fork_size += sprintf(forktab+fork_size, "<td>%.2f</td>", fork->sell_amount[x]);
		strcpy(forktab+fork_size, "</tr><tr>");
		fork_size += 9;
		fork_size += sprintf(forktab+fork_size, "<td>BuyDate</td>");
		for (x=0; x<fork->nr_positions; x++)
			fork_size += sprintf(forktab+fork_size, "<td>%s</td>", fork->buy_date[x]);
		strcpy(forktab+fork_size, "</tr><tr>");
		fork_size += 9;
		fork_size += sprintf(forktab+fork_size, "<td>SellDate</td>");
		for (x=0; x<fork->nr_positions; x++)
			fork_size += sprintf(forktab+fork_size, "<td>%s</td>", fork->sell_date[x]);
		strcpy(forktab+fork_size, "</tr><tr>");
		fork_size += 9;
		fork_size += sprintf(forktab+fork_size, "<td>Cash</td>");
		for (x=0; x<fork->nr_positions; x++)
			fork_size += sprintf(forktab+fork_size, "<td>%.2f</td>", fork->cash[x]);
		strcpy(forktab+fork_size, "</tr>");
		fork->fork_tsize = fork_size + 5;

		/* Table #4.2 */
		for (x=0; x<fork->nr_positions; x++) {
			capital = fork->capital[fork->nr_positions-1-x];
			if (!capital)
				continue;
			break;
		}
		if (!fork->RTD)
			RTD = "";
		else {
			sprintf(RTDbuf, "%.2f", fork->RTD);
			RTD = RTDbuf;
		}
		if (!fork->APY)
			APY = "";
		else {
			sprintf(APYbuf, "%.2f", fork->APY);
			APY = APYbuf;
		}
 		pos = fork->nr_positions-1;
		switch (fork->fork_class) {
			case AIRFORK_CLASS_HYPER:
				portfolios->hypersonic_size += snprintf(portfolios->hypersonic+portfolios->hypersonic_size, 256,
											  "{\"PID\":\"%d\",\"Capital\":\"%.2f\",\"RTD\":\"%s\",\"APY\":\"%s\",\"LP\":\"%s\",\"T\":\"%s\",\"S\":\"%d\"},",
											  fork->port_id, capital, RTD, APY, fork->buy_date[pos], fork->tickers[pos], fork->qty[pos]);
				if (portfolios->hypersonic_size + 1024 >= max_hypersonic_size) {
					max_hypersonic_size += 64 KB;
					portfolios->hypersonic = (char *)realloc(portfolios->hypersonic, max_hypersonic_size);
				}
				break;
			case AIRFORK_CLASS_SUPER:
				portfolios->supersonic_size += snprintf(portfolios->supersonic+portfolios->supersonic_size, 256,
											  "{\"PID\":\"%d\",\"Capital\":\"%.2f\",\"RTD\":\"%s\",\"APY\":\"%s\",\"LP\":\"%s\",\"T\":\"%s\",\"S\":\"%d\"},",
											  fork->port_id, capital, RTD, APY, fork->buy_date[pos], fork->tickers[pos], fork->qty[pos]);
				if (portfolios->supersonic_size + 1024 >= max_supersonic_size) {
					max_supersonic_size += 64 KB;
					portfolios->supersonic = (char *)realloc(portfolios->supersonic, max_supersonic_size);
				}
				break;
			case AIRFORK_CLASS_BIZ:
				portfolios->business_size += snprintf(portfolios->business+portfolios->business_size, 256,
											  "{\"PID\":\"%d\",\"Capital\":\"%.2f\",\"RTD\":\"%s\",\"APY\":\"%s\",\"LP\":\"%s\",\"T\":\"%s\",\"S\":\"%d\"},",
											  fork->port_id, capital, RTD, APY, fork->buy_date[pos], fork->tickers[pos], fork->qty[pos]);
				if (portfolios->business_size + 1024 >= max_business_size) {
					max_business_size += 64 KB;
					portfolios->business = (char *)realloc(portfolios->business, max_business_size);
				}
				break;
			case AIRFORK_CLASS_ECO:
				portfolios->economy_size += sprintf(portfolios->economy+portfolios->economy_size,
											  "{\"PID\":\"%d\",\"Capital\":\"%.2f\",\"RTD\":\"%s\",\"APY\":\"%s\",\"LP\":\"%s\",\"T\":\"%s\",\"S\":\"%d\"},",
											  fork->port_id, capital, RTD, APY, fork->buy_date[pos], fork->tickers[pos], fork->qty[pos]);
				if (portfolios->economy_size + 1024 >= max_economy_size) {
					max_economy_size += 64 KB;
					portfolios->economy = (char *)realloc(portfolios->economy, max_economy_size);
				}
				break;
		}

		fork->boarding_passes = (char *)malloc(256);
		strcpy(fork->boarding_passes, "sigstat ");
		fork->boarding_passes_len = 8;
		boarding_passes_max = 256;
		for (x=0; x<fork->nr_positions; x++) {
			struct stock *stock = search_stocks(fork->tickers[x]);
			if (!stock)
				break;
			if (!fork->buy_amount[x])
				continue;
			if (fork->sell_amount[x])
				sigret = fork->sell_amount[x];
			else
				sigret = ((stock->current_price/fork->buy_amount[x])-1)*100;
			if (fork->boarding_passes_len + 64 >= boarding_passes_max) {
				boarding_passes_max  += 64;
				fork->boarding_passes = (char *)realloc(fork->boarding_passes, boarding_passes_max);
			}
//			if (!fork->sell_date[x])
//				status = 
			fork->boarding_passes_len += snprintf(fork->boarding_passes+fork->boarding_passes_len, 63,
										 		 "%s %s %s %.2f %.2f*", fork->tickers[x], fork->buy_date[x], fork->sell_date[x], fork->buy_amount[x], sigret);
		}
		fork->boarding_passes[--fork->boarding_passes_len] = 0;

		/* Table #5 */
		fquarter = fork_quarter(fork->buy_date[0], DATE_MDY);
		if (!fquarter)
			continue;
		fquarter->total++;

		GSPC_start = price_by_date(XLS->GSPC, fork->buy_date[0]);
		GSPC_end   = XLS->GSPC->mag->close[XLS->GSPC->mag->nr_entries-1];
		GSPC_RTD   = ((GSPC_end/GSPC_start)-1);
//		printf("buy_date: %s GSPC_START: %.2f END: %.2f RTD: %.2f\n", fork->buy_date[0], GSPC_start, GSPC_end, GSPC_RTD);

		unix_start_date = str2unix2(fork->buy_date[0]);
		unix_end_date   = get_timestamp();
		nr_days         = (double)(unix_end_date-unix_start_date)/86400.0;

		//((current date price / start date price - 1) + 1)^(365.25/(current date - start date))-1
		if (nr_days < 365) {
			/* RTD */
//			printf("[%s] fork->RTD: %.2f GSPC_RTD: %.2f DIFF: %.2f\n", fork->buy_date[0], fork->RTD, GSPC_RTD, fork->RTD-GSPC_RTD);
			if (fork->RTD > GSPC_RTD)
				fquarter->nr_RTD_SP500++;
			if (fork->RTD - GSPC_RTD >= 0.05)
				fquarter->nr_RTD_SP500_5pc++;
			if (fork->RTD - GSPC_RTD >= 0.1)
				fquarter->nr_RTD_10pc++;
		} else {
			/* APY */
			GSPC_APY = pow(GSPC_RTD+1, 365.25/nr_days)-1;
//			printf("signal %s %s RTD: %.2f APY: %.2f\n", fork->tickers[0], fork->buy_date[0], fork->RTD, fork->APY);
//			printf("GSPC_START: %.2f GSPC_END: %.2f GSPC_RTD: %.2f\n", GSPC_start, GSPC_end, GSPC_RTD);
//			printf(BOLDGREEN "GSPC_APY: (%.2f^(365.25/%.2f))-1=%.2f" RESET "\n", GSPC_RTD, nr_days, GSPC_APY);
			if (fork->APY >= GSPC_APY)
				fquarter->nr_APY_SP500++;
			if (fork->APY - GSPC_APY >= 0.05)
				fquarter->nr_APY_SP500_5pc++;
			if (fork->APY - GSPC_APY >= 0.1)
				fquarter->nr_APY_10pc++;
		}
//		printf("[%d] buy date: %s gspc: %.2f GSPC_APY: %.2f\n", fork->port_id, fork->buy_date[0], GSPC_start, GSPC_APY);
		/* Chart #4.1 */
		// that was the final capital value on the y axis and the portfolio number on the x axis
	}

//	sort_forks(forks, nr_forks);

	/* *********************
	 * MASTER FORK TABLE #5
	 **********************/
	flight_saftey_table(monster);

	/* ALLFORKS Chart #4.1 - forkcat {x/y series} [1d close series] */
	// that was the final capital value on the y axis and the portfolio number on the x axis
	portfolios->hyper_port_scatter    = NULL;
	portfolios->super_port_scatter    = NULL;
	portfolios->business_port_scatter = NULL;
	portfolios->economy_port_scatter  = NULL;
	for (x=0; x<nr_forks; x++) {
		fork = &forks[x];
		if (!fork)
			continue;
		if (fork->nr_positions <= 1)
			continue;
		capital = fork->capital[fork->nr_positions-2];
		if (!capital)
			continue;
		switch (fork->fork_class) {
			case AIRFORK_CLASS_HYPER:
				if (!portfolios->hyper_port_scatter) {
					portfolios->hyper_port_scatter      = (char *)zmalloc(64 KB);
					portfolios->hyper_port_scatter_max  = 64 KB;
					portfolios->hyper_port_scatter_size = 14;
					strcpy(portfolios->hyper_port_scatter, "]@portscatter 0 [");
				}
				portfolios->hyper_port_scatter_size    += sprintf(portfolios->hyper_port_scatter+portfolios->hyper_port_scatter_size, "{\"x\":%d,\"y\":%.2f},", fork->port_id, capital);
				if (portfolios->hyper_port_scatter_size + 1024 > portfolios->hyper_port_scatter_max) {
					portfolios->hyper_port_scatter_max += 4 KB;
					portfolios->hyper_port_scatter      = (char *)realloc(portfolios->hyper_port_scatter, portfolios->hyper_port_scatter_max);
				}
				break;
			case AIRFORK_CLASS_SUPER:
				if (!portfolios->super_port_scatter) {
					portfolios->super_port_scatter      = (char *)zmalloc(64 KB);
					portfolios->super_port_scatter_max  = 64 KB;
					portfolios->super_port_scatter_size = 14;
					strcpy(portfolios->super_port_scatter, "]@portscatter 1 [");
				}
				portfolios->super_port_scatter_size    += sprintf(portfolios->super_port_scatter+portfolios->super_port_scatter_size, "{\"x\":%d,\"y\":%.2f},", fork->port_id, capital);
				if (portfolios->super_port_scatter_size + 1024 > portfolios->super_port_scatter_max) {
					portfolios->super_port_scatter_max += 4 KB;
					portfolios->super_port_scatter      = (char *)realloc(portfolios->super_port_scatter, portfolios->super_port_scatter_max);
				}
				break;
			case AIRFORK_CLASS_BIZ:
				if (!portfolios->business_port_scatter) {
					portfolios->business_port_scatter      = (char *)zmalloc(64 KB);
					portfolios->business_port_scatter_max  = 64 KB;
					portfolios->business_port_scatter_size = 14;
					strcpy(portfolios->business_port_scatter, "]@portscatter 2 [");
				}
				portfolios->business_port_scatter_size    += sprintf(portfolios->business_port_scatter+portfolios->business_port_scatter_size, "{\"x\":%d,\"y\":%.2f},", fork->port_id, capital);
				if (portfolios->business_port_scatter_size + 1024 > portfolios->business_port_scatter_max) {
					portfolios->business_port_scatter_max += 4 KB;
					portfolios->business_port_scatter      = (char *)realloc(portfolios->business_port_scatter, portfolios->business_port_scatter_max);
				}
				break;
			case AIRFORK_CLASS_ECO:
				if (!portfolios->economy_port_scatter) {
					portfolios->economy_port_scatter       = (char *)zmalloc(64 KB);
					portfolios->economy_port_scatter_max   = 64 KB;
					portfolios->economy_port_scatter_size  = 14;
					strcpy(portfolios->economy_port_scatter, "]@portscatter 3 [");
				}
				portfolios->economy_port_scatter_size     += sprintf(portfolios->economy_port_scatter+portfolios->economy_port_scatter_size, "{\"x\":%d,\"y\":%.2f},", fork->port_id, capital);
				if (portfolios->economy_port_scatter_size  + 1024 > portfolios->economy_port_scatter_max) {
					portfolios->economy_port_scatter_max  += 4 KB;
					portfolios->economy_port_scatter       = (char *)realloc(portfolios->economy_port_scatter, portfolios->economy_port_scatter_max);
				}
				break;
		}
	}

	if (portfolios->hyper_port_scatter)
		portfolios->hyper_port_scatter   [portfolios->hyper_port_scatter_size-1]    = ']';
	if (portfolios->super_port_scatter)
		portfolios->super_port_scatter   [portfolios->super_port_scatter_size-1]    = ']';
	if (portfolios->business_port_scatter)
		portfolios->business_port_scatter[portfolios->business_port_scatter_size-1] = ']';
	if (portfolios->economy_port_scatter)
		portfolios->economy_port_scatter [portfolios->economy_port_scatter_size-1]  = ']';

	if (portfolios->hypersonic_size+portfolios->hyper_port_scatter_size >= max_hypersonic_size)
		portfolios->hypersonic = (char *)realloc(portfolios->hypersonic, portfolios->hypersonic_size+portfolios->hyper_port_scatter_size+256);

	if (portfolios->supersonic_size+portfolios->super_port_scatter_size >= max_supersonic_size)
		portfolios->supersonic = (char *)realloc(portfolios->supersonic, portfolios->supersonic_size+portfolios->super_port_scatter_size+256);

	if (portfolios->business_size+portfolios->business_port_scatter_size >= max_business_size)
		portfolios->business   = (char *)realloc(portfolios->business, portfolios->business_size+portfolios->business_port_scatter_size+256);

	if (portfolios->economy_size+portfolios->economy_port_scatter_size >= max_economy_size)
		portfolios->economy    = (char *)realloc(portfolios->economy, portfolios->economy_size+portfolios->economy_port_scatter_size+256);

	if (portfolios->hyper_port_scatter)
		memcpy(portfolios->hypersonic+portfolios->hypersonic_size-1, portfolios->hyper_port_scatter,    portfolios->hyper_port_scatter_size);	
	if (portfolios->super_port_scatter)
		memcpy(portfolios->supersonic+portfolios->supersonic_size-1, portfolios->super_port_scatter,    portfolios->super_port_scatter_size);
	if (portfolios->business_port_scatter)
		memcpy(portfolios->business  +portfolios->business_size-1,   portfolios->business_port_scatter, portfolios->business_port_scatter_size);
	if (portfolios->economy_port_scatter)
		memcpy(portfolios->economy   +portfolios->economy_size-1,    portfolios->economy_port_scatter,  portfolios->economy_port_scatter_size);

	portfolios->hypersonic_size += portfolios->hyper_port_scatter_size    - 1;
	portfolios->supersonic_size += portfolios->super_port_scatter_size    - 1;
	portfolios->business_size   += portfolios->business_port_scatter_size - 1;
	portfolios->economy_size    += portfolios->economy_port_scatter_size  - 1;

	/* ONEFORK Chart #4.2 */
	for (x=0; x<nr_forks; x++) {
		fork = &forks[x];
		if (!fork)
			continue;
		if (fork->nr_positions <= 1)
			continue;

		if (fork->avgdays > 0 && fork->avgdays < 10000) {
			portfolios->avgdays += fork->avgdays;
//			printf("%d portfolios_avgdays: %.2f avgdays: %.2f\n", fork->port_id, portfolios->avgdays, fork->avgdays);
		}
		fork->fork_scatter  = (char *)malloc(4 KB);
		if (!fork->fork_scatter)
			continue;
		strcpy(fork->fork_scatter, "forkscat [");
		fork->fork_scatter_size = 10;
		max_size = 4 KB;
		for (y=0; y<fork->nr_positions; y++) {
			fork->fork_scatter_size += sprintf(fork->fork_scatter+fork->fork_scatter_size, "[%lu,%.2f],", str2unix(fork->buy_date[y])*1000, fork->capital[y]);
			if (fork->fork_scatter_size + 1024 > max_size) {
				max_size = fork->fork_scatter_size + 1 KB;
				fork->fork_scatter = (char *)realloc(fork->fork_scatter, max_size);
			}
		}
		fork->fork_scatter[fork->fork_scatter_size-1] = ']';
		fork->fork_scatter[fork->fork_scatter_size++] = '@';
	}
	portfolios->avgdays /= nr_forks;
	if (portfolios->supersonic)
		*(portfolios->supersonic + portfolios->supersonic_size-1) = ']';
	if (portfolios->business)
		*(portfolios->business   + portfolios->business_size-1)   = ']';
	if (portfolios->economy)
		*(portfolios->economy    + portfolios->economy_size-1)    = ']';

	// Set Current Portfolios
	monster->portfolios = portfolios;
	// forkmem -> forkdb
	store_forkdb("db/forks.db", forks, nr_forks);
}

/* FORKDB -> XLS->forks (FORKMEM) */
int load_forkdb(struct XLS *XLS)
{
	struct forkdb   *forkdb;
	struct forkmem  *forkmem, *FORKMEM;
	char            *forkfile, *forkptr; 
	char             timestr[16] = {0};
	int              nr_positions, min_pos, nr_extra, nr_forks, x, y;
	int              nr_qwords, nr_ints, nr_shorts, total_qwords, total_ints, total_shorts, min_qwords, min_ints, min_shorts;
	int64_t          forksize;

	forkfile = fs_mallocfile("db/forks.db", &forksize);
	if (!forkfile) {
		printf(BOLDRED "forks.db missing" RESET "\n");
		return 0;
	}
	XLS->nr_forks = nr_forks = *(unsigned int *)forkfile;
	if (nr_forks <= 0 || nr_forks > 512 KB) {
		printf(BOLDRED "load_forkdb: NR_FORKS: %d" RESET "\n", nr_forks);
		free(forkfile);
		return 0;
	}

	XLS->forks = (struct forkmem *)zmalloc(nr_forks * sizeof(struct forkmem));
	FORKMEM    = XLS->forks;
	forkdb     = (struct forkdb  *)(forkfile+8);
	forkmem    = (struct forkmem *)(&FORKMEM[0]); // forkmem is a pointer into the XLS->forks array
	for (x=0; x<nr_forks; x++) {
		nr_positions =  forkdb->nr_positions;
		if (nr_positions < 0 || nr_positions > 128)
			continue;

		/* Set non-array members */
		forkmem->nr_positions        = nr_positions;
		forkmem->RTD                 = forkdb->RTD;
		forkmem->APY                 = forkdb->APY;
		forkmem->avgdays             = forkdb->avgdays;
		forkmem->fork_class          = forkdb->fork_class;
		forkmem->port_id             = forkdb->port_id;
		forkmem->fork_tsize          = forkdb->fork_tsize;
		forkmem->fork_scatter_size   = forkdb->fork_scatter_size;
		forkmem->boarding_passes_len = forkdb->boarding_passes_len;

		total_qwords = nr_positions * 8;
		total_ints   = nr_positions * 4;
		total_shorts = nr_positions * 2;
		min_pos      = MIN(nr_positions, NR_EXP);
		min_qwords   = MIN(nr_positions, NR_EXP) * 8;
		min_ints     = MIN(nr_positions, NR_EXP) * 4;
		min_shorts   = MIN(nr_positions, NR_EXP) * 2;

		// buy_amount
		forkmem->buy_amount  = (double *)malloc(total_qwords);
		memcpy(forkmem->buy_amount,  &forkdb->buy_amount[0], min_qwords);
		// sell_amount
		forkmem->sell_amount = (double *)malloc(total_qwords);
		memcpy(forkmem->sell_amount, &forkdb->sell_amount[0], min_qwords);
		// cash
		forkmem->cash        = (double *)malloc(total_qwords);
		memcpy(forkmem->cash, &forkdb->cash[0], min_qwords);
		// capital
		forkmem->capital     = (double *)malloc(total_qwords);
		memcpy(forkmem->capital, &forkdb->capital[0], min_qwords);
		// buy_date + sell_date
		forkmem->buy_date  = (char **)malloc(total_qwords);
		forkmem->sell_date = (char **)malloc(total_qwords);
		for (y=0; y<min_pos; y++) {
			*(uint64_t *)timestr = *(uint64_t *)&forkdb->buy_date[y];
			forkmem->buy_date[y] = strdup(timestr);
			*(uint64_t *)timestr = *(uint64_t *)&forkdb->sell_date[y];
			forkmem->sell_date[y] = strdup(timestr);
//			printf("buy_date: %s sell_date: %s\n", forkmem->buy_date[y], forkmem->sell_date[y]);
		}
		// nr_days
		forkmem->nr_days = (int *)malloc(total_ints);
		memcpy(forkmem->nr_days, &forkdb->nr_days[0], min_ints);
		// qty
		forkmem->qty     = (int *)malloc(total_ints);
		memcpy(forkmem->qty, &forkdb->qty[0], min_ints);
		// tickers
		forkmem->tickers = (char **)malloc(total_qwords);
//		for (y=0; y<min_pos; y++)
//			forkmem->tickers[y] = stock_sym(forkdb->tickers[y]);

//		for (y=0; y<MIN(nr_positions, NR_EXP); y++)
//			printf("forkdb[%d].nr_days[%d] = %x\n", x, y, forkdb->nr_days[y]);
		if (nr_positions > NR_EXP) {
			nr_extra      = (nr_positions-NR_EXP);
			nr_qwords     = nr_extra*sizeof(uint64_t);
			nr_ints       = nr_extra*sizeof(int);
			nr_shorts     = nr_extra*sizeof(short);
			forkptr       = (char *)((char *)forkdb+sizeof(struct forkdb));  // move to the end of this forkdb struct where the rest of the array data is
			nr_positions -= NR_EXP;

			// buy_amount
			memcpy(&forkmem->buy_amount[NR_EXP], (int *)forkptr, nr_qwords);
			forkptr += nr_qwords;
			// sell_amount
			memcpy(&forkmem->sell_amount[NR_EXP], (int *)forkptr, nr_qwords);
			forkptr += nr_qwords;
			// cash
			memcpy(&forkmem->cash[NR_EXP],      (void *)forkptr, nr_qwords);
			forkptr += nr_qwords;
			// capital
			memcpy(&forkmem->capital[NR_EXP],   (void *)forkptr, nr_qwords);
			forkptr += nr_qwords;
			// buy_date
			for (y=0; y<nr_extra; y++) {
				*(uint64_t *)timestr   = *(uint64_t *)forkptr;
				forkmem->buy_date[NR_EXP+y] = strdup(timestr);
				forkptr += 8;
			}
			for (y=0; y<nr_extra; y++) {
				*(uint64_t *)timestr    = *(uint64_t *)forkptr;
				forkmem->sell_date[NR_EXP+y] = strdup(timestr);
				forkptr += 8;
			}
			// nr_days
			memcpy(&forkmem->nr_days[NR_EXP],   (void *)forkptr, nr_ints);
			forkptr += nr_ints;
			// qty
			memcpy(&forkmem->qty[NR_EXP],       (void *)forkptr, nr_ints);
			forkptr += nr_ints;
			// tickers
//			for (y=0; y<nr_extra; y++) {
//				forkmem->tickers[NR_EXP+y] = stock_sym(*(unsigned short *)forkptr);
//				forkptr += sizeof(unsigned short);
//			}
		} else {
			forkptr = (char *)((char *)forkdb+sizeof(struct forkdb));
		}
		// fork_table
		forkmem->fork_table = (char *)malloc(forkmem->fork_tsize+1);
		memcpy(forkmem->fork_table, forkptr, forkmem->fork_tsize);
		forkmem->fork_table[forkmem->fork_tsize] = 0;
		forkptr += forkmem->fork_tsize;
		// fork_scatter_table
		forkmem->fork_scatter = (char *)malloc(forkmem->fork_scatter_size+1);
		memcpy(forkmem->fork_scatter, forkptr, forkmem->fork_scatter_size);
		forkmem->fork_scatter[forkmem->fork_scatter_size] = 0;
		forkptr += forkmem->fork_scatter_size;
		// boarding_passes
		forkmem->boarding_passes = (char *)malloc(forkmem->boarding_passes_len+1);
		memcpy(forkmem->boarding_passes, forkptr, forkmem->boarding_passes_len);
		forkmem->boarding_passes[forkmem->boarding_passes_len] = 0;
		forkptr += forkmem->boarding_passes_len;

		// Next Fork
		forkmem++;
		forkdb = (struct forkdb *)forkptr;
	}
//	store_forkdb("db/forks2.db", FORKMEM, nr_forks);
	return (1);
}

void store_forkdb(char *forkpath, struct forkmem *FORKMEM, int nr_forks)
{
	char           *FORKDB = (char *)zmalloc(3074 KB);
	struct forkdb  *forkdb;
	struct forkmem *forkmem;
	int             forkdb_size = 0, nr_positions, nr_qwords, nr_ints, nr_shorts, min_pos, nr_extra, x, y;
	int             forkdb_max_size = 3074 KB;

	/* FORKDB HEADER */
	*(uint64_t *)FORKDB = nr_forks;
	forkdb_size += 8;

	for (x=0; x<nr_forks; x++) {
		if (forkdb_size + 32 KB >= forkdb_max_size) {
			forkdb_max_size += 256 KB;
			FORKDB = (char *)realloc(FORKDB, forkdb_max_size);
			if (!FORKDB)
				exit(-1);
		}

		forkdb       = (struct forkdb *)(FORKDB+forkdb_size);
		forkmem      = &FORKMEM[x];
		nr_positions = forkmem->nr_positions;

		/* Set non-array members */
		forkdb->nr_positions        = nr_positions;
		forkdb->RTD                 = forkmem->RTD;
		forkdb->APY                 = forkmem->APY;
		forkdb->avgdays             = forkmem->avgdays;
		forkdb->fork_class          = forkmem->fork_class;
		forkdb->port_id             = forkmem->port_id;
		forkdb->fork_tsize          = forkmem->fork_tsize;
		forkdb->fork_scatter_size   = forkmem->fork_scatter_size;
		forkdb->boarding_passes_len = forkmem->boarding_passes_len;

		min_pos   = MIN(nr_positions, NR_EXP);
		nr_qwords = min_pos * 8;
		nr_ints   = min_pos * 4;
		nr_shorts = min_pos * 2;
		// buy_amount
		memcpy(&forkdb->buy_amount[0],  &forkmem->buy_amount[0],  nr_qwords);
		// sell_amount
		memcpy(&forkdb->sell_amount[0], &forkmem->sell_amount[0], nr_qwords);
		// capital
		memcpy(&forkdb->capital[0],     &forkmem->capital[0],     nr_qwords);
		// buy_date + sell_date
		for (y=0; y<min_pos; y++) {
			*(uint64_t *)&forkdb->buy_date[y]  = *(uint64_t *)forkmem->buy_date[y];
			*(uint64_t *)&forkdb->sell_date[y] = *(uint64_t *)forkmem->sell_date[y];
		}
		// nr_days
		if (forkmem->nr_days)
			memcpy(&forkdb->nr_days[0], &forkmem->nr_days[0],     nr_ints);
		// qty
		memcpy(&forkdb->qty[0],         &forkmem->qty[0],         nr_ints);
		// tickers
		for (y=0; y<min_pos; y++)
			*(unsigned short *)(&forkdb->tickers[y]) = stock_id(forkmem->tickers[y]);

		forkdb_size += sizeof(*forkdb);
		if (nr_positions > NR_EXP) {
			nr_extra  = (nr_positions-NR_EXP);
			nr_qwords = (nr_positions-NR_EXP)*sizeof(unsigned long);
			nr_ints   = (nr_positions-NR_EXP)*sizeof(int);

			// buy_amount  (double)
			memcpy(FORKDB+forkdb_size, &forkmem->buy_amount[NR_EXP],  nr_qwords); forkdb_size += nr_qwords;
			// sell_amount (double)
			memcpy(FORKDB+forkdb_size, &forkmem->sell_amount[NR_EXP], nr_qwords); forkdb_size += nr_qwords;
			// cash        (double)
			memcpy(FORKDB+forkdb_size, &forkmem->cash[NR_EXP],        nr_qwords); forkdb_size += nr_qwords;
			// capital     (double)
			memcpy(FORKDB+forkdb_size, &forkmem->capital[NR_EXP],     nr_qwords); forkdb_size += nr_qwords;
			// buy_date
			for (y=0; y<nr_extra; y++) {
				*(uint64_t *)(FORKDB+forkdb_size) = *(uint64_t *)forkmem->buy_date[NR_EXP+y];
				forkdb_size += 8;
			}
			// sell_date
			for (y=0; y<nr_extra; y++) {
				*(uint64_t *)(FORKDB+forkdb_size) = *(uint64_t *)forkmem->sell_date[NR_EXP+y];
				forkdb_size += 8;
			}
			// nr_days     (int)
			if (forkmem->nr_days)
				memcpy(FORKDB+forkdb_size, &forkmem->nr_days[NR_EXP], nr_ints);   forkdb_size += nr_ints;
			// qty         (int)
			memcpy(FORKDB+forkdb_size, &forkmem->qty[NR_EXP],         nr_ints);   forkdb_size += nr_ints;
			// tickers     (short)
			for (y=0; y<nr_extra; y++) {
				*(unsigned short *)(FORKDB+forkdb_size) = stock_id(forkmem->tickers[NR_EXP+y]);
				forkdb_size += sizeof(unsigned short);
			}
//			memcpy(FORKDB+forkdb_size, &forkmem->tickers[NR_EXP],     nr_shorts); forkdb_size += nr_shorts;
		}
		// Fork Table
		memcpy(FORKDB+forkdb_size, forkmem->fork_table,      forkmem->fork_tsize);
		forkdb_size += forkmem->fork_tsize;
		// Fork Scatter Chart
		memcpy(FORKDB+forkdb_size, forkmem->fork_scatter,    forkmem->fork_scatter_size);
		forkdb_size += forkmem->fork_scatter_size;
		// Boarding Passes
		memcpy(FORKDB+forkdb_size, forkmem->boarding_passes, forkmem->boarding_passes_len);
		forkdb_size += forkmem->boarding_passes_len;
	}
	fs_writefile(forkpath, FORKDB, forkdb_size);
}

void update_forks_EOD()
{
	load_forks(CURRENT_XLS);
	generate_monster_db(CURRENT_XLS);
	printf("update_forks_EOD(): OK\n");
}

void init_forks(struct XLS *XLS)
{
	/* 2020 */
	fork_quarters[0].fquarter[0].start = "1/1/2020";
	fork_quarters[0].fquarter[0].end   = "3/31/2020";
	fork_quarters[0].fquarter[1].start = "4/1/2020";
	fork_quarters[0].fquarter[1].end   = "6/30/2020";
	fork_quarters[0].fquarter[2].start = "7/1/2020";
	fork_quarters[0].fquarter[2].end   = "9/30/2020";
	fork_quarters[0].fquarter[3].start = "10/1/2020";
	fork_quarters[0].fquarter[3].end   = "12/31/2020";
	/* 2021 */
	fork_quarters[1].fquarter[0].start = "1/1/2021";
	fork_quarters[1].fquarter[0].end   = "3/31/2021";
	fork_quarters[1].fquarter[1].start = "4/1/2021";
	fork_quarters[1].fquarter[1].end   = "6/30/2021";
	fork_quarters[1].fquarter[2].start = "7/1/2021";
	fork_quarters[1].fquarter[2].end   = "9/30/2021";
	/* 2022 */
	fork_quarters[2].fquarter[0].start = "1/1/2022";
	fork_quarters[2].fquarter[0].end   = "3/31/2022";
	fork_quarters[2].fquarter[1].start = "4/1/2022";
	fork_quarters[2].fquarter[1].end   = "6/30/2022";
	fork_quarters[2].fquarter[2].start = "7/1/2022";
	fork_quarters[2].fquarter[2].end   = "9/30/2022";

	if (!XLS->config->production)
		return;

	if (!load_forkdb(XLS))
		load_forks(XLS);
}
