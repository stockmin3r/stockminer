#include <conf.h>
#include <extern.h>

struct market {
	int         country_id;
	int         instrument;
	int         exchange;
	int         pre_start_hour;
	int         pre_start_min;
	int         day_start_hour;
	int         day_start_min;
	int         afh_start_hour;
	int         afh_start_min;
	int         afh_end_hour;
	int         afh_end_min;
	int         trading_period;
	int         nr_trading_hours;
	time_t      market_eod_prev_ts;  // the last calendar EOD timestamp for this market: eg 2023-11-30 14:30 UTC
	time_t      market_eod_next_ts;  // + 24 hours from the previous EOD (market_eod_timestamp) calendar trading day 
	char       *market_eod_prev_str; // %Y%M%D
	char       *market_eod_next_str; // %Y%M%D
	int         status;              // NO_MARKET|PRE_MARKET|DAY_MARKET|AFH_MARKET
};

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
	{ US, STOCK_TYPE_STOCK,  MARKET_NASDAQ,  9,  0, 14, 30, 21, 0, 1, 0, MARKET_WEEKDAYS, 5, 7  },
	{ US, STOCK_TYPE_STOCK,  MARKET_NYSE,   11, 30, 14, 30, 21, 0, 1, 0, MARKET_WEEKDAYS, 3, 7  },
	{ WW, STOCK_TYPE_CRYPTO, MARKET_CRYPTO,  0,  0,  0,  0,  0, 0, 0, 0, MARKET_24_7,    24, 24 }
};

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

struct market *search_market(struct stock *stock)
{
	for (int x=0; x<sizeof(markets)/sizeof(struct market); x++) {
		struct market *market = &markets[x];
		if (market->country_id == stock->country_id)
			return (market);
	}
	return (NULL);
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
	printf("ticker last EOD: %s\n", p);
	return str2utc(p) + 24*3600;
}

bool ticker_needs_update(struct stock *stock, time_t *update_timestamp, int *nr_trading_hours)
{
	struct stat    sb;
	struct tm      current_utc_tm;
	struct tm      last_update_tm;
	time_t         epoch;
	struct market *market = search_market(stock);
	time_t         last_update_timestamp;
	char           path[256];

	market = search_market(stock);
	*nr_trading_hours = market->nr_trading_hours;

	snprintf(path, sizeof(path)-1, "%s/%s.csv", STOCKDB_CSV_PATH , stock->sym);
	printf("path: %s\n", path);

	// fetch data for ticker if it doesn't exist
	if (stat(path, &sb) == -1) {
		*update_timestamp = 0;
		return true;
	}

	last_update_timestamp = sb.st_mtime;
	gmtime_r(&last_update_timestamp, &last_update_tm);

	// get the current utc tm - Current UTC time in broken down time format
	time(&epoch);
	gmtime_r(&epoch, &current_utc_tm);
	printf("file_modified_time: %lu curday: %d lastday: %d\n", last_update_timestamp, current_utc_tm.tm_mday, last_update_tm.tm_mday);

	if (current_utc_tm.tm_year > last_update_tm.tm_year || current_utc_tm.tm_mon > last_update_tm.tm_mon) {
		*update_timestamp = ticker_last_EOD(path);
		printf("month stale: %lu QDATE[0]: %lu\n", *update_timestamp, QDATESTAMP[0]);
		return true;
	}

	// update the ticker if it has been more than one day (unless it's sunday & market is weekdays)
	if ((current_utc_tm.tm_mday - last_update_tm.tm_mday) > 1) {
		*update_timestamp = ticker_last_EOD(path);
		printf("day stale: %lu QDATE[0]: %lu\n", *update_timestamp, QDATESTAMP[0]);
		return true;
	}

	if (market->trading_period != MARKET_24_7) {
		if (current_utc_tm.tm_wday == 0 || current_utc_tm.tm_wday == 6) {
			printf("skipping weekend\n");
			return false;}
	}

	printf("last_update_tm day: %d utc_tm_day: %d\n", last_update_tm.tm_mday, current_utc_tm.tm_mday);
	if (current_utc_tm.tm_hour >= market->afh_start_hour) {
		printf("curren_utc hour: %d EOD hour: %d\n", current_utc_tm.tm_hour, market->afh_start_hour);
		*update_timestamp = ticker_last_EOD(path);
		return true;
	}
	return false;
}
