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

/* 
   - 1M Daily OHLC: 1 Minute has passed so add a new OHLC tick to the price->price_1m (and price->price_1d) 
   - adding the 1m ticks to price->price_1d buffer was done for "optimization" purposes to avoid extra copying
   - however it made the code more complex and may be split up
*/
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
		price->price_1d_len += tick->current_ohlc_size+1;
	}

	/* Line Chart Update */
	tick->current_mini_size = sprintf(tick->current_mini, "[%lu,%.2f]", new_tick, new_close);
	tmp                 = stock->current_tick;
	stock->current_tick = tick;
	stock->sparetick    = tmp;
}

/*
 * Polling-based OHLCv update: fetching data via HTTP GET
 */
void update_current_price(struct stock *stock)
{
	int *data_source;

	switch (stock->type) {
		case STOCK_TYPE_INDEX:
		case STOCK_TYPE_FUND:
		case STOCK_TYPE_STOCK:
			data_source = &Server.stocks_1M;
			break;
		case STOCK_TYPE_CRYPTO:
			data_source = &Server.crypto_1M;
			break;
		default:
			return;
	}

	switch (*data_source) {
		case STOCKDATA_WSJ:
			printf("updating current price for: %s\n", stock->sym);
			WSJ_update_current_price(stock);
			break;
		case STOCKDATA_OFF:
			break;
		
	}
}
