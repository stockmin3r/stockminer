#include <conf.h>
#include <extern.h>

void apc_ohlc(struct connection *connection, char **argv)
{
	struct stock *stock = search_stocks(argv[0]);

	if (!stock)
		return;

	for (int x=0; x<stock->nr_ohlc; x++) {
		struct ohlc *ohlc = &stock->ohlc[x];
		printf("Timestmap: %lu Open: %.2f Close: %.2f\n", ohlc->timestamp, ohlc->open, ohlc->close);
	}
}

void apc_yahoo_fetch(struct connection *connection, char **argv)
{
	struct stock *stock = search_stocks(argv[0]);
	char buf[1024 KB];
	char url[256];

	if (!stock)
		return;
	sprintf(url, YAHOO_OHLC, stock->sym);
	for (int x=0; x<3; x++) {
		if (!curl_get(url, buf))
			break;
	}
	printf("%s\n", buf);
}

void apc_stock_debug(struct connection *connection, char **argv)
{
	char msg[256];
	char *ticker, *p;

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

void apc_live(struct connection *connection, char **argv)
{
	struct stock *stock = search_stocks(argv[0]);

	if (!stock)
		return;

	printf(BOLDGREEN "Open: %.2f High: %.2f Low: %.2f Close: %.2f" RESET "\n", stock->current_open, stock->current_high, stock->current_low, stock->current_close);
	for (int x=0; x<stock->nr_ohlc; x++) {
		struct ohlc *ohlc = &stock->ohlc[x];
		printf(BOLDWHITE "[%d] Open: %.2f High: %.2f Low: %.2f Close: %.2f Volume: %llu" RESET "\n", x, ohlc->open, ohlc->high, ohlc->low, ohlc->close, ohlc->volume);
	}
	printf("nr_ohlc: %d\n", stock->nr_ohlc);
}

void apc_signals(struct connection *connection, char **argv)
{
	struct stock  *stock = search_stocks(argv[0]);
	struct action *action;
	struct mag    *mag;
	char          *date;

	if (!stock || !(mag=stock->mag))
		return;

	for (int x=0; x<mag->nr_action1; x++) {
		action = &mag->action1[x];
		if (action->entry <= 0 || action->entry >= mag->nr_entries) {
			printf(BOLDRED "ENTRY SIGNAL ERROR: %d 5days: %d 5pc: %.2f 10days: %d 10pc: %.2f" RESET "\n",mag->nr_entries, action->five_days, action->five_pt, action->ten_days, action->ten_pt);
			continue;
		}
		date = mag->date[action->entry];
		printf("[%s] {Action 1} 5pt: %.2f (%d days) 10pt: %.2f (%d days) (STATUS: %d, ENTRY: %d)\n", date, action->five_pt, action->five_days, action->ten_pt, action->ten_days, action->status, action->entry);
	}
	for (int x=0; x<mag->nr_action4; x++) {
		action = &mag->action4[x];
		if (action->entry <= 0 || action->entry >= mag->nr_entries) {
			printf(BOLDRED "ENTRY SIGNAL ERROR: %d 5days: %d 5pc: %.2f 10days: %d 10pc: %.2f" RESET "\n",mag->nr_entries, action->five_days, action->five_pt, action->ten_days, action->ten_pt);
			continue;
		}
		date = mag->date[action->entry];
		printf("[%s] {Action 4} 5pt: %.2f (%d days) 10pt: %.2f (%d days) (STATUS: %d, ENTRY: %d)\n", date, action->five_pt, action->five_days, action->ten_pt, action->ten_days, action->status, action->entry);
	}
}

void apc_print_stock(struct connection *connection, char **argv)
{
	struct stock *stock = search_stocks(argv[0]);
	char         *buf   = malloc(4096);
	int           len;

	if (!stock || !buf)
		return;

	len = snprintf(buf, sizeof(buf)-1, "%p (ID: %d) current_price: %.2f pr_percent: %.2f vol: %llu prior_close: %.2f (15m: %.2f, 5m: %.2f 1m: %.2f)\n",
				  stock, stock->id, stock->current_price, stock->pr_percent, stock->current_volume, stock->prior_close,
				  stock->price_15m, stock->price_5m, stock->price_1min);
	printf("%s\n", buf);
	for (int x=0; x<stock->nr_signals; x++) {
		struct sig *sig = stock->signals[x];
		printf("sig_entry: %s sig_exit: %s nr_days: %d\n", sig->entry_date, sig->exit_date, sig->nr_days);
	}
	printf("thread_id: %d update: %d nr_ohlc: %d sig_avgdays: %.2f\n", stock->thread_id, stock->update, stock->nr_ohlc, stock->sig_avgdays);
//	printf("rank 2020-11-30: %d\n", date_to_rank(stock, "2020-11-30"));
//	printf("rank 2021-02-04: %d\n", date_to_rank(stock, "2021-02-04"));
//	return (buf);
}

void apc_candle(struct connection *connection, char **argv)
{
	char         *ticker = argv[0];
	char         *cname  = argv[1];
	struct stock *stock  = search_stocks(ticker);
	struct mag   *mag;
	int           len, nr_candles;

	if (!stock || !(mag=stock->mag))
		return;

	nr_candles = mag->nr_candles;
	for (int x=0; x<nr_candles; x++) {
		struct CANDLE *C = &mag->candles[x];
		if (!C->name || strcmp(cname, C->name))
			continue;
		printf("%s %-24s entry: %d type: %d\n", C->date, C->name, C->entry, C->type);
	}

	for (int x=0; x<NR_CANDLES; x++) {
		if (!strcmp(cname,  CTYPES[x].name)) {
			struct candle *candle = stock->candles[x];
			if (candle->type == CANDLE_BULL)
				printf("[%s] day1_bull: %.2f day3_bull: %.2f day7_bull: %.2f day21_bull: %.2f\n", CTYPES[x].name, candle->day1_bull, candle->day3_bull, candle->day7_bull, candle->day21_bull);
			else if (candle->type == CANDLE_BEAR)
				printf("[%s] day1_bear: %.2f day3_bear: %.2f day7_bear: %.2f day21_bear: %.2f\n", CTYPES[x].name, candle->day1_bear, candle->day3_bear, candle->day7_bear, candle->day21_bear);
		}
	}
}

void apc_ebuild_stocks(struct connection *connection, char **argv)
{
	struct stock *stock = search_stocks(argv[0]);

	if (!stock)
		return;
	build_earnings(stock);
}
