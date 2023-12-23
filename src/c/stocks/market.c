#include <conf.h>
#include <extern.h>

#define MARKET_WEEKDAYS 1
#define MARKET_24_7     2

/* ------------
 * Market Hours
 * ------------
 * US-Stocks:
 *   - pre market starts at  4:00 EST | 9:00 UTC
 *   - day market starts at  9:30 EST | 14:30 UTC
 *   - day market ends   at 16:00 EST | 21:00 UTC
 *   - afh market ends   at 20:00 EST | 01:00 UTC
 * Crypto: (24/7)
 *   -    market starts at 00:00 UTC
 *   -    market ends   at 00:00 UTC (next day)
 */
struct market markets[] =
{
	{ US, STOCK_TYPE_STOCK,  MARKET_NASDAQ,  9,  0, 14, 30, 21, 0, 1, 0, MARKET_WEEKDAYS, 12,  960 },
	{ US, STOCK_TYPE_STOCK,  MARKET_NYSE,   12,  0, 14, 30, 21, 0, 1, 0, MARKET_WEEKDAYS,  9,  720 },
	{ WW, STOCK_TYPE_CRYPTO, MARKET_CRYPTO,  0,  0,  0,  0,  0, 0, 0, 0, MARKET_24_7,     24, 1440 }
};

void market_set_EOD(time_t current_utc_timestamp, int country_id, char *prev_day, char *curr_day, char *next_day)
{
	for (int x=0; x<sizeof(markets)/sizeof(struct market); x++) {
		struct market *market = &markets[x];
		if (market->country_id == country_id) {
			time_t prev_eod_timestamp  = str2utc(prev_day) + ((market->afh_start_hour*3600)+(market->afh_start_min*60));
			time_t curr_eod_timestamp  = str2utc(curr_day) + ((market->afh_start_hour*3600)+(market->afh_start_min*60));
			time_t next_eod_timestamp  = str2utc(next_day) + ((market->afh_start_hour*3600)+(market->afh_start_min*60));

			printf("market trading period: %d vs %d\n", market->trading_period, MARKET_24_7);	
			if (market->trading_period != MARKET_24_7) {
				// If the current time has passed today's EOD for this market then adjust the prev and curr EOD timestamps
				printf("curutc: %lu next_eod: %lu\n", current_utc_timestamp, next_eod_timestamp);
				if (current_utc_timestamp >= next_eod_timestamp) {
					market->prev_eod = strdup(curr_day);
					market->next_eod = strdup(next_day);
					market->prev_eod_timestamp = curr_eod_timestamp;
					market->next_eod_timestamp = next_eod_timestamp;
					printf(BOLDGREEN "current: %lu prev_day timestamp: %lu next_day_timestamp: %lu" RESET "\n", current_utc_timestamp, prev_eod_timestamp, next_eod_timestamp);
				} else {
					market->prev_eod = strdup(prev_day);
					market->next_eod = strdup(curr_day);
					market->prev_eod_timestamp = prev_eod_timestamp;
					market->next_eod_timestamp = curr_eod_timestamp;
				}
			} else {
				market->prev_eod = strdup(prev_day);
				market->next_eod = strdup(curr_day);
				market->prev_eod_timestamp = prev_eod_timestamp;
				market->next_eod_timestamp = curr_eod_timestamp;
			}
		}
	}
}

struct market *search_market(struct stock *stock)
{
	for (int x=0; x<sizeof(markets)/sizeof(struct market); x++) {
		struct market *market = &markets[x];
		if (market->country_id == stock->country_id)
			return (market);
	}
	return (NULL);
}

time_t market_prev_eod_unix(struct stock *stock)
{
	struct market *market = search_market(stock);

	if (!market)
		return 0;
	return market->prev_eod_timestamp;
}

time_t market_next_eod_unix(struct stock *stock)
{
	struct market *market = search_market(stock);

	if (!market)
		return 0;
	return market->next_eod_timestamp;
}

char *market_prev_eod_str(struct stock *stock)
{
	struct market *market = search_market(stock);

	if (!market)
		return NULL;
	return market->prev_eod;
}

int market_get_nr_minutes(struct stock *stock)
{
	struct market *market = search_market(stock);

	if (!market)
		return 0;
	return market->nr_trading_minutes;
}

int market_get_nr_hours(struct stock *stock)
{
	struct market *market = search_market(stock);

	if (!market)
		return 0;
	return market->nr_trading_hours;
}

void market_update_status(struct market *market)
{
	time_t    epoch;
	struct tm utc_tm;
	int       chour, cmin;
	int       pre_start_hour, day_start_hour, afh_start_hour;

	time(&epoch);
	gmtime_r(&epoch, &utc_tm);

	if (market->trading_period == MARKET_WEEKDAYS && (utc_tm.tm_wday == 0 || utc_tm.tm_wday == 6)) {
		market->status = NO_MARKET;
		return;
	}

	chour = utc_tm.tm_hour;
	cmin  = utc_tm.tm_min;
	if (chour >= market->pre_start_hour) {
		// [1] If the Current Hour is equal to the Market's PreMarket Starting Hour AND the Current Minute is Greater or Equal to the PreMarket Starting Minute
		if ((chour == market->pre_start_hour) && ((cmin >= market->pre_start_min))) {
			market->status = PRE_MARKET;
			return;
		}

		// [2] If the Current Hour is between the Market's PreMarket Starting Hour AND the Market's DayMarket Opening Hour
		//  + The Current minute is Greater or Equal to the PreMarket's Starting Minute 
		if (chour < market->day_start_hour && cmin >= market->pre_start_min) {
			market->status = PRE_MARKET;
			return;
		}

		// [3] If the Current Hour is Equal to the DayMarket's Starting Hour
		if (chour == market->day_start_hour && cmin < market->day_start_min) {
			market->status = PRE_MARKET;
			return;
		}
	}

	/*
	 * DayMarket - AfhMarket
	 */
	if (chour >= market->day_start_hour) {
		// [1] If the Current Hour is equal to the Market's DayMarket Starting Hour AND the Current Minute is Greater or Equal to the PreMarket Starting Minute
		if ((chour == market->day_start_hour) && ((cmin >= market->day_start_min))) {
			market->status = PRE_MARKET;
			return;
		}

		// [2] If the Current Hour is between the Market's DayMarket Starting Hour AND the Market's AfhMarket Starting Hour
		//  + The Current minute is Greater or Equal to the PreMarket's Opening Minute 
		if (chour < market->afh_start_hour && cmin >= market->pre_start_min) {
			market->status = PRE_MARKET;
			return;
		}

		// [3] If the Current Hour is Equal to the DayMarket's Starting Hour
		if (chour == market->day_start_hour && cmin < market->day_start_min) {
			market->status = PRE_MARKET;
			return;
		}
	}

	/*
	 * DayMarket - AfhMarket
	 */
	// todo

	/*
	 * NoMarket - PreMarket
	 */
	market->status = NO_MARKET;
}

int market_update(void)
{
	for (int x = 0; x<sizeof(markets)/sizeof(struct market); x++)
		market_update_status(&markets[x]);
	return markets[0].status; // for a while yet still rely on global static market variable until removal
}

// this will be moved to LMDB with a (country-exchange-ticker):(last_update_eod_timestamp) key/pair
time_t ticker_last_EOD(char *path)
{
	struct stat sb;
	char buf[256];
	char *p;
	int fd, nbytes;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;

	fstat(fd, &sb);
	lseek(fd, -256, SEEK_END);
	nbytes      = read(fd, buf, 256);
	buf[nbytes] = 0;
	p           = buf+nbytes;
	while (*p != '-')
		p--;

	*(p+3) = 0;
	p     -= 7;
//	printf("p: %s utc: %lu\n", p, str2utc(p));
	return str2utc(p);
}

bool ticker_needs_update(struct stock *stock, time_t *start_timestamp, time_t *end_timestamp)
{
	struct stat    sb;
	struct market *market          = search_market(stock);
	time_t         market_prev_eod = market->prev_eod_timestamp;
	time_t         market_next_eod = market->next_eod_timestamp;
	time_t         epoch           = time(NULL);
	time_t         last_csv_eod_date; // the last date string in the CSV file (converted to a UTC time_t) (midnight UTC of the current day)
	char           path[256];
	int            nr_days;

	if (market->status == NO_MARKET)
		*end_timestamp  = market_next_eod;

	snprintf(path, sizeof(path)-1, "%s/%s.csv", STOCKDB_CSV_PATH , stock->sym);
	/*
	 * If the file does not exist then fetch several years worth of data and store it
	 * into a csv which will be updated automatically as time passes
	 */
	if (stat(path, &sb) == -1) {
		*start_timestamp = UNIX_TIMESTAMP_1990;
		*start_timestamp = UNIX_TIMESTAMP_2016;
		return true;
	}

	last_csv_eod_date = ticker_last_EOD(path);
	*start_timestamp  = last_csv_eod_date+(3600*24);

//	printf(BOLDWHITE "start timestamp: %lu" RESET "\n", *start_timestamp);
//	printf(BOLDWHITE "end   timestamp: %lu" RESET "\n", *end_timestamp);

	if (market->trading_period == MARKET_24_7) {
		if ((epoch-last_csv_eod_date)/3600/24 > 1) {
			--(*end_timestamp);
			return true;
		}
//		printf("no update: %d\n", (epoch-last_csv_eod_date)/3600, epoch, last_csv_eod_date );
		return false;
	}

	/* - For WEEKDAY markets:
	 * If the last date OHLC we have is from BEFORE the EOD of the current UTC calendar day
	 * and the current day is before the start of the NEXT trading day then we are in a holiday or weekend
 	 */
	nr_days = (epoch-last_csv_eod_date)/3600/24;
	if (nr_days >= 1) {
		if (nr_days == 1 && epoch < market_next_eod)
			return false;
		printf("update: %s %d epoch: %lu last_csv_eod: %lu\n", stock->sym, (epoch-last_csv_eod_date)/3600, epoch, last_csv_eod_date );
		return true;
	}
	return false;
}
