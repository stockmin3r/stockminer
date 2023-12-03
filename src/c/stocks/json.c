#include <conf.h>
#include <extern.h>
#include <stocks/stocks.h>

/* ***********
 *  LOWCAPS
 */
void update_lowcaps(struct board *board)
{
	struct stock *stock;
	char         *json, *jptr;
	char          vbuf[32];
	int           x, buffer, nbytes = 0, nr_stocks;
	uint64_t      vol;

	switch (board->buffer) {
		case 1:json = board->buf2;buffer = 2;break;
		case 2:json = board->buf1;buffer = 1;break;
		default:return;
	}
	jptr      = json; *jptr  = '['; nbytes = 1;
	nr_stocks = board->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = board->stocks[x];
		if (market == PRE_MARKET)
 			vol = stock->premarket_volume*stock->current_price;
		else
			vol = stock->day_volume*stock->current_price;
		nbytes += snprintf(jptr+nbytes, 128, "{\"T\":\"%s\",\"P\":\"%.2f\",\"D\":\"%.2f\",\"V\":\"$%s\"},",stock->sym, stock->current_price, stock->pr_percent, volume(vol, vbuf));
	}
	if (nbytes == 1)
		return;
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;
	switch (buffer) {
		case 1:board->json_size1 = nbytes; board->buffer = 1; break;
		case 2:board->json_size2 = nbytes; board->buffer = 2; break;
	}
	board->dirty = 0;
}

/* ***********
 *  HIGHCAPS
 */
void update_highcaps(struct board *board)
{
	struct stock *stock;
	char         *json, *jptr;
	char          vbuf[32];
	int           x, buffer, nbytes = 0, nr_stocks;
	uint64_t      vol;

	switch (board->buffer) {
		case 1:json = board->buf2;buffer = 2;break;
		case 2:json = board->buf1;buffer = 1;break;
		default:return;
	}
	jptr      = json; *jptr  = '['; nbytes = 1;
	nr_stocks = board->nr_stocks;
	if (unlikely(nr_stocks < 0))
		return;
	for (x=0; x<nr_stocks; x++) {
		stock = board->stocks[x];
		if (stock->subtype != STOCK_SUBTYPE_HIGHCAPS)
			continue;
		if (market == PRE_MARKET)
 			vol = stock->premarket_volume*stock->current_price;
		else
			vol = stock->day_volume*stock->current_price;
		if (stock->current_price == 0.0)
			continue;
		nbytes += snprintf(jptr+nbytes, 8192, "{\"T\":\"%s\",\"R\":\"%d\",\"P\":\"%.2f\",\"D\":\"%.2f\",\"V\":\"$%s\"},", stock->sym, stock->rank, stock->current_price, stock->pr_percent, volume(vol, vbuf));
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;
	switch (buffer) {
		case 1:board->json_size1 = nbytes; board->buffer = 1; break;
		case 2:board->json_size2 = nbytes; board->buffer = 2; break;
	}
	board->dirty = 0;
}

/* ********************
 *  LOWCAPS UFO DOUBLE
 */
void update_lowcaps_double(struct board *board)
{
	struct stock *stock;
	char         *json, *jptr;
	char          vbuf[32];
	int           x, buffer, nbytes = 0, nr_stocks, offset;
	double        value;
	uint64_t      vol;

	switch (board->buffer) {
		case 1:json = board->buf2;buffer = 2;break;
		case 2:json = board->buf1;buffer = 1;break;
		default:return;
	}
	jptr      = json; *jptr  = '['; nbytes = 1;
	nr_stocks = board->nr_stocks;
	offset    = board->offset;
	if (unlikely(nr_stocks < 0))
		return;
	for (x=0; x<nr_stocks; x++) {
		stock = board->stocks[x];
		if (market == PRE_MARKET)
 			vol = stock->premarket_volume*stock->current_price;
		else
			vol = stock->day_volume*stock->current_price;
		value = *(double *)((char *)stock+offset);
		if (Server.DEBUG_STOCK && !strcmp(stock->sym, Server.DEBUG_STOCK))
			printf(BOLDBLUE "update_lowcaps_double(): [%s] update_lowcaps_double: value: %.2f offset: %d" RESET "\n", stock->sym, value, offset);
		nbytes += sprintf(jptr+nbytes, "{\"900\":\"%s\",\"901\":\"%.2f\",\"903\":\"%.2f\",\"904\":\"$%s\"},",stock->sym, stock->current_price, value, volume(vol, vbuf));
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;
	switch (buffer) {
		case 1:board->json_size1 = nbytes; board->buffer = 1; break;
		case 2:board->json_size2 = nbytes; board->buffer = 2; break;
	}
	board->dirty = 0;
}

/* *********************
 *  HIGHCAPS UFO DOUBLE
 */
void update_highcaps_double(struct board *board)
{
	struct stock *stock;
	char         *json, *jptr;
	char          vbuf[32];
	int           x, buffer, nbytes = 0, nr_stocks, offset;
	double        value;
	uint64_t      vol;

	switch (board->buffer) {
		case 1:json = board->buf2;buffer = 2;break;
		case 2:json = board->buf1;buffer = 1;break;
		default:return;
	}
	jptr      = json; *jptr  = '['; nbytes = 1;
	nr_stocks = board->nr_stocks;
	offset    = board->offset;
	if (unlikely(!nr_stocks || offset >= (sizeof(struct stock))-8))
		return;
	for (x=0; x<nr_stocks; x++) {
		stock = board->stocks[x];
		if (market == PRE_MARKET)
 			vol = stock->premarket_volume*stock->current_price;
		else
			vol = stock->day_volume*stock->current_price;
		value   = *(double *)((char *)stock+offset);
		if (Server.DEBUG_STOCK && !strcmp(stock->sym, Server.DEBUG_STOCK))
			printf(BOLDBLUE "update_highcaps_double(): [%s] value: %.2f offset: %d volume: %llu volstr: %s" RESET "\n", stock->sym, (double)value, (int)offset, vol, volume(vol, vbuf));
		nbytes += snprintf(jptr+nbytes, 128, "{\"900\":\"%s\",\"901\":\"%.2f\",\"903\":\"%.2f\",\"904\":\"$%s\"},", stock->sym, stock->current_price, value, volume(vol, vbuf));
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;
	switch (buffer) {
		case 1:board->json_size1 = nbytes; board->buffer = 1; break;
		case 2:board->json_size2 = nbytes; board->buffer = 2; break;
	}
	board->dirty = 0;
}


/* ****************
 *  LOWCAPS VOLUME
 */
void update_lowcaps_volume(struct board *board)
{
	struct stock *stock;
	char         *json, *jptr;
	char          vbuf[32];
	int           x, buffer, nbytes = 0, nstocks, offset;
	double        vspike;
	uint64_t      vol;

	switch (board->buffer) {
		case 1:json = board->buf2;buffer = 2;break;
		case 2:json = board->buf1;buffer = 1;break;
		default:return;
	}
	jptr    = json; *jptr  = '['; nbytes = 1;
	nstocks = board->nr_stocks;
	offset  = board->offset;
	for (x=0; x<nstocks; x++) {
		stock = board->stocks[x];
		if (market == PRE_MARKET)
 			vol = stock->premarket_volume*stock->current_price;
		else
			vol = stock->day_volume*stock->current_price;
		vspike  = *(double *)((char *)stock+offset);
		nbytes += snprintf(jptr+nbytes, 128, "{\"900\":\"%s\",\"901\":\"%.2f\",\"903\":\"%.2f\",\"905\":\"%.2f\",\"904\":\"$%s\"},", stock->sym, stock->current_price, stock->pr_percent, vspike, volume(vol, vbuf));
	}
	if (nbytes == 1)
		return;
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;
	switch (buffer) {
		case 1:board->json_size1 = nbytes; board->buffer = 1; break;
		case 2:board->json_size2 = nbytes; board->buffer = 2; break;
	}
	board->dirty = 0;
}

/* *****************
 *  HIGHCAPS VOLUME
 */
void update_highcaps_volume(struct board *board)
{
	struct stock *stock;
	char         *json, *jptr;
	char          vbuf[32];
	int           x, buffer, nbytes = 0, nstocks, offset;
	double        vspike;
	uint64_t      vol;

	switch (board->buffer) {
		case 1:json = board->buf2;buffer = 2;break;
		case 2:json = board->buf1;buffer = 1;break;
		default:return;
	}
	jptr    = json; *jptr  = '['; nbytes = 1;
	nstocks = board->nr_stocks;
	offset  = board->offset;
	for (x=0; x<nstocks; x++) {
		stock = board->stocks[x];
		if (market == PRE_MARKET)
 			vol = stock->premarket_volume*stock->current_price;
		else
			vol = stock->day_volume*stock->current_price;
		vspike  = *(double *)((char *)stock+offset);
		nbytes += snprintf(jptr+nbytes, 8192, "{\"900\":\"%s\",\"902\":\"%s\",\"901\":\"%.2f\",\"903\":\"%.2f\",\"905\":\"%.2f\",\"904\":\"$%s\"},", stock->sym, stock->rankstr, stock->current_price, stock->pr_percent, vspike, volume(vol, vbuf));
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;
	switch (buffer) {
		case 1:board->json_size1 = nbytes; board->buffer = 1; break;
		case 2:board->json_size2 = nbytes; board->buffer = 2; break;
	}
	board->dirty = 0;
}

/* ********************
 *      INDICATORS
 */
void update_indicators(struct board *board)
{
	struct stock *stock;
	char         *json, *jptr;
	char          vbuf[32];
	char         *macd, *rsi, *aroon, *stoch, *bb, *cci, *kch;
	int           x, buffer, nbytes = 0, nstocks, offset;
	double        value;
	uint64_t      vol;

	switch (board->buffer) {
		case 1:json = board->buf2;buffer = 2;break;
		case 2:json = board->buf1;buffer = 1;break;
		default:return;
	}
	jptr    = json; *jptr  = '['; nbytes = 1;
	nstocks = board->nr_stocks;
	offset  = board->offset;
	for (x=0; x<nstocks; x++) {
		stock = board->stocks[x];
		if (market == PRE_MARKET)
 			vol = stock->premarket_volume*stock->current_price;
		else
			vol = stock->day_volume*stock->current_price;

		if (stock->indicators      & BULLISH_MACD)
			macd = "🐃";
		else if (stock->indicators & BEARISH_MACD)
			macd = "🐨";
		else
			macd = "";

		if (stock->indicators      & BULLISH_RSI)
			rsi = "🐃";
		else if (stock->indicators & BEARISH_RSI)
			rsi = "🐨";
		else
			rsi = "";

		if (stock->indicators      & BULLISH_AROON)
			aroon = "🐃";
		else if (stock->indicators & BEARISH_AROON)
			aroon = "🐨";
		else
			aroon = "";

		if (stock->indicators      & BULLISH_STOCH)
			stoch = "🐃";
		else if (stock->indicators & BEARISH_STOCH)
			stoch = "🐨";
		else
			stoch = "";

		if (stock->indicators      & BULLISH_BB)
			bb = "🐃";
		else if (stock->indicators & BEARISH_BB)
			bb = "🐨";
		else
			bb = "";

		if (stock->indicators      & BULLISH_CCI)
			cci = "🐃";
		else if (stock->indicators & BEARISH_CCI)
			cci = "🐨";
		else
			cci = "";

		if (stock->indicators      & BULLISH_KCH)
			kch = "🐃";
		else if (stock->indicators & BEARISH_KCH)
			kch = "🐨";
		else
			kch = "";

		value = *(double *)((char *)stock+offset);
		nbytes += sprintf(jptr+nbytes, "{\"900\":\"%s\",\"901\":\"%.2f\",\"903\":\"%.2f\",\"904\":\"$%s\",\"700\":\"%s\",\"701\":\"%s\",\"702\":\"%s\",\"703\":\"%s\",\"704\":\"%s\",\"705\":\"%s\",\"706\":\"%s\"},",
						  stock->sym, stock->current_price, value, volume(vol, vbuf),
						  macd, rsi, aroon, stoch, bb, cci, kch);
	}
	jptr[nbytes-1] = ']'; jptr[nbytes] = 0;
	switch (buffer) {
		case 1:board->json_size1 = nbytes; board->buffer = 1; break;
		case 2:board->json_size2 = nbytes; board->buffer = 2; break;
	}
	board->dirty = 0;
}

void *json_thread(void *args)
{
	struct board *board;
	int x;

	while (1) {
		struct XLS *XLS = CURRENT_XLS;
		if (!XLS || !XLS->boards) {
			os_sleep(100000);
			continue;
		}
		for (x=0; x<NR_BOARDS; x++) {
			board = XLS->boards[x];
			if (board->dirty)
				board->update(board);
		}
		os_usleep(100000);
	}
}
