#include <conf.h>
#include <extern.h>

// will be replaced when a task scheduler is written

static void init_stock(struct stock *stock, int stock_id)
{
	struct price *price;

	stock->id               = stock_id;
	stock->candles          = (struct candle **)zmalloc(sizeof(struct candle *) * (NR_CANDLES));
	stock->ohlc             = (struct ohlc    *)zmalloc(sizeof(struct ohlc)     * (TRADE_MINUTES+100));
	stock->price            = price = (struct price *)zmalloc(sizeof(struct price));
	price->price_1m         = (char *)malloc(96 KB);
	price->price_mini       = (char *)malloc(32 KB);
	*price->price_1m        = 0;
	*price->price_mini      = 0;
	stock->candle_json      = (char *)malloc(28 KB);
	stock->candle_json_max  = 28 KB;
	stock->current_tick     = &stock->tickbuf;
	stock->sparetick        = &stock->tickbuf2;
	stock->options_url_size = snprintf(stock->options_url, sizeof(stock->options_url)-1, "/api/global/delayed_quotes/options/%s.json", stock->sym);
	set_index(stock);
	load_fundamentals(stock);
}

void alloc_stock_threads(struct XLS *XLS)
{
	struct thread *thread;
	int            nr_threads, stocks_per_thread;
	int            rem, x, y, stock_id = 0;

	stocks_per_thread    = 30;
	rem                  = XLS->nr_stocks%30;
	nr_threads           = XLS->nr_stocks/30;
	XLS->stock_threads   = (struct thread *)zmalloc(sizeof(struct thread) * (nr_threads+1));
	printf("alloc_stock_threads(XLS): total threads: %d stocks_per_thread: %d rem: %d\n", nr_threads, stocks_per_thread, rem);
	for (x=0; x<nr_threads; x++) {
		thread         = &XLS->stock_threads[x];
		thread->stocks = &XLS->STOCKS_PTR[x*stocks_per_thread];
		thread->XLS    = XLS;
		thread->id     = x;
		thread->stocks_per_thread = stocks_per_thread;
		for (y=0; y<stocks_per_thread; y++) {
			init_stock(thread->stocks[y], stock_id);
			thread->stocks[y]->thread_id = x;
			if (Server.DEBUG_STOCK && !strcmp(Server.DEBUG_STOCK, thread->stocks[y]->sym))
				printf(BOLDYELLOW "%s is in thread group: %d" RESET "\n", thread->stocks[y]->sym, x);
			thread->nr_stocks++;
			stock_id++;
		}
	}
	if (!rem)
		XLS->nr_stock_threads = nr_threads;

	thread                    = &XLS->stock_threads[x];
	thread->stocks            = &XLS->STOCKS_PTR[x*stocks_per_thread];
	thread->XLS               = XLS;
	thread->stocks_per_thread = rem;
	for (x=0; x<rem; x++) {
		init_stock(thread->stocks[x], stock_id);
		thread->stocks[x]->thread_id = x;
		if (Server.DEBUG_STOCK && !strcmp(Server.DEBUG_STOCK, thread->stocks[x]->sym))
			printf(BOLDYELLOW "%s is in thread group: %d" RESET "\n", thread->stocks[x]->sym, nr_threads-1);
		thread->nr_stocks++;
		stock_id++;
	}
	XLS->nr_stock_threads = (nr_threads+1);
}

void create_stock_threads(struct XLS *XLS)
{
	int x;

	alloc_stock_threads(XLS);
	for (x=0; x<XLS->nr_stock_threads; x++)
		thread_create(stock_thread, &XLS->stock_threads[x]);

	// synchronize until all stocks have been loaded from this market
	while (Server.stock_boot != XLS->nr_stock_threads) os_usleep(100000);
}

void *stock_thread(void *args)
{
	struct thread *thread = (struct thread *)args;
	struct stock  *stock;
	struct server *config;
	struct XLS    *XLS;
	int            x, stocks_per_thread;

	stocks_per_thread = thread->stocks_per_thread;
	XLS               = thread->XLS;
	config            = XLS->config;
	for (x=0; x<stocks_per_thread; x++) {
		stock = thread->stocks[x];
		init_algo(XLS, stock);
		if (XLS->config->production) {
			load_mag2(stock);
			load_mag3(stock);
			load_mag4(stock);
		}
	}

	mutex_lock(&config->stock_lock);
	config->stock_boot++;
	mutex_unlock(&config->stock_lock);

	for (x=0; x<stocks_per_thread; x++) {
		stock = thread->stocks[x];
		update_allday_price(XLS, stock);
	}

	while (1) {
		while (market == NO_MARKET)
			os_sleep(4);
		if (thread->stop)
			return NULL;
		for (x=0; x<stocks_per_thread; x++) {
			stock = thread->stocks[x];
			/* Current Price */
			if (config->DEBUG_STOCK && !strcmp(stock->sym, config->DEBUG_STOCK))
				printf(BOLDBLUE "stock_thread(): [%s] pr_percent: %.2f current_price: %.2f update: %d" RESET "\n", stock->sym, stock->pr_percent, stock->current_price, stock->update);
			if (!stock->update) {
				if (stock->nr_data_retries++ < 1)
					update_allday_price(XLS, stock);
				continue;
			}
			update_current_price(stock);
		}
		thread->workload++;
		sleep(3);
		if (thread->stop)
			return NULL;
	}
	return NULL;
}

void stock_data_status(char *ticker, int hour, int failed)
{
	struct filemap    filemap;
	struct tradetime *tradetime;
	char              tbuf[sizeof(struct tradetime)] = {0};
	char              path[256];

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.tradetime", ticker);
	tradetime = (struct tradetime *)MAP_FILE_RW(path, &filemap);
	if (!tradetime) {
		tradetime = (struct tradetime *)tbuf;
		fs_writefile(path, (char *)tradetime, sizeof(*tradetime));
		tradetime = (struct tradetime *)MAP_FILE_RW(path, &filemap);
		if (!tradetime)
			return;
	}

	switch (hour) {
		case 4: if (failed) tradetime->time_4AM--; else tradetime->time_4AM++; break;
		case 5: if (failed) tradetime->time_5AM--; else tradetime->time_5AM++; break;
		case 6: if (failed) tradetime->time_6AM--; else tradetime->time_6AM++; break;
		case 7: if (failed) tradetime->time_7AM--; else tradetime->time_7AM++; break;
		case 8: if (failed) tradetime->time_8AM--; else tradetime->time_8AM++; break;
		case 9: if (failed) tradetime->time_9AM--; else tradetime->time_9AM++; break;
	}
	UNMAP_FILE((char *)tradetime, &filemap);
}

void update_threads(struct thread *threads, int nr_threads, int hour)
{
	int x, y;

	for (x=0; x<nr_threads; x++) {
		struct thread *thread = &threads[x];
		int nr_stocks         = thread->stocks_per_thread;
		for (y=0; y<nr_stocks; y++) {
			struct stock *stock = thread->stocks[y];
			if (!stock->update) {
				if (update_allday_price(CURRENT_XLS, stock)) {
					stock_data_status(stock->sym, hour, 0); // success
					printf(BOLDGREEN "new stock update: %s hour: %d" RESET "\n", stock->sym, hour);
				} else {
					stock_data_status(stock->sym, hour, 1); // failed
				}
			}
		}
	}
}

void *market_update_thread(void *args)
{
	struct XLS *XLS = CURRENT_XLS;
	uint64_t hour   = (uint64_t)args;

	update_threads(XLS->stock_threads, XLS->nr_stock_threads, hour);
	printf(BOLDGREEN "nr_working_stocks: %d" RESET "\n", Server.nr_working_stocks);
	printf(BOLDRED   "nr_failed_stocks:  %d" RESET "\n", Server.nr_failed_stocks);
	return NULL;
}

void end_threads(struct thread *threads, int nr_threads)
{
	int x, y;

	for (x=0; x<nr_threads; x++) {
		struct thread *thread = &threads[x];
		int nr_stocks         = thread->stocks_per_thread;
		for (y=0; y<nr_stocks; y++) {
			struct stock *stock = thread->stocks[y];
			if (!stock || !stock->price)
				continue;
			stock->price->stale = 1;
		}
	}
}

void *market_end(void *args)
{
	struct XLS *XLS = CURRENT_XLS;
	end_threads(XLS->stock_threads,  XLS->nr_stock_threads);
	Server.nr_working_stocks = 0;
	Server.nr_failed_stocks  = 0;
	printf(BOLDWHITE "SETTING END THREADS STALE" RESET "\n");
	return NULL;
}
