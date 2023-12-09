#include <conf.h>
#include <extern.h>

#define CCCAGG_CRYPTO_IDX    1  // cryptocurrency symbol eg BTC
#define CCCAGG_CURRENCY_IDX  2  // vs currency eg USD
#define CCCAGG_TIMESTAMP_IDX 5  // last update timestamp
#define CCCAGG_PRICE_IDX     4  // current price
#define CCCAGG_VOLUME_IDX   13  // volume index

/*
 * {"action":"SubAdd","subs":["5~CCCAGG~BTC~USD"]} (msgtype: 5 - CryptoCompare's Aggregate Index)
 *
 */
 

void update_crypto_data(char *buf)
{
	struct stock *stock;
	struct ohlc  *ohlc;
	char         *argv[64];
	char          ticker[32];
	time_t        last_update_timestamp;
	int           argc;

	argc = cstring_split(buf, argv, 63, '~');
	if (argc <= 12)
		return;
	snprintf(ticker, sizeof(ticker)-1, "%s-%s", argv[CCCAGG_CRYPTO_IDX], argv[CCCAGG_CURRENCY_IDX]);
	printf("price; %s\n", argv[CCCAGG_PRICE_IDX]);
	stock = search_stocks(ticker);
	if (!stock) {
		printf(BOLDRED "ticker error: %s" RESET "\n", ticker);
		return;
	}
	// December 8 2023, timestamp should always be higher than this date
	last_update_timestamp = strtoull(argv[CCCAGG_TIMESTAMP_IDX], NULL, 10);
	if (last_update_timestamp < 1702038335)
		return;

	stock->current_price  = strtod  (argv[CCCAGG_PRICE_IDX], NULL);
	stock->current_volume = strtoull(argv[CCCAGG_VOLUME_IDX], NULL, 10);
	printf("price: %.3f vol: %lu curtimestamp: %lu laststamp: %lu\n", stock->current_price, stock->current_volume, stock->current_timestamp, last_update_timestamp);

	if (!stock->current_timestamp)
		stock->current_timestamp = last_update_timestamp;

	if (stock->current_timestamp < last_update_timestamp) {
		// a new tick has passed
		ohlc = &stock->ohlc[stock->nr_ohlc++];
		stock->current_timestamp = last_update_timestamp;
		stock->current_open      = stock->current_price;
		printf(BOLDRED "ohlc: %p\n", ohlc);
	} else {
		ohlc = &stock->ohlc[stock->nr_ohlc==0?0:(stock->nr_ohlc-1)];
		printf(BOLDGREEN "ohlc: %p\n", ohlc);
	}

	if (stock->current_price > stock->current_high)
		stock->current_high = ohlc->high  = stock->current_price;
	else if (stock->current_price < stock->current_low)
		stock->current_low  = ohlc->low   = stock->current_price;

	stock->current_close    = ohlc->close = stock->current_price;
	ohlc->volume            = stock->current_volume;

	if (stock->current_price < stock->prior_close)
		stock->pr_percent = -((stock->prior_close-stock->current_price)/stock->prior_close)*100;
	else
		stock->pr_percent = ((stock->current_price-stock->prior_close)/stock->prior_close)*100;
}

void cryptocompare_handler(struct connection *connection)
{
	struct XLS   *XLS;
	char          sslbuf[16 KB];
	char          wsbuf[32 KB];
	char          cpair[16];
	char          subactions[32 KB];
	char         *ticker;
	struct frame  frames[6];
	int           nr_frames, nbytes = 0, subaction_size = 0, ticker_size = 0;
	packet_size_t packet_size = 0;

	char *str = "{\"action\":\"SubAdd\",\"subs\":[\"5~CCCAGG~%s\"]}";

	strcpy(subactions, "{\"action\":\"SubAdd\",\"subs\":[");
	subaction_size = 27;

	XLS = CURRENT_XLS;
	for (int x = 0; x<XLS->nr_crypto; x++) {
		struct stock *stock = XLS->CRYPTO[x];

		ticker = stock->sym;
		if (strlen(ticker) > 15)
			continue;
	
		while (*ticker != '\0') {
			if (*ticker == '-') {
				cpair[ticker_size++] = '~';
				ticker++;
				continue;
			}
			cpair[ticker_size++] = *ticker++;
		}
		ticker = cpair;
		subaction_size += snprintf(subactions+subaction_size, 24, "\"5~CCCAGG~%s\",", cpair);
		if (subaction_size + 64 >= sizeof(subactions))
			break;
		ticker_size = 0;
	}
	subactions[subaction_size-1] = ']';
	subactions[subaction_size++] = '}';
	printf(BOLDCYAN "subactions: %s" RESET "\n", subactions);

	// Get the STREAMERWELCOME message
	openssl_read_sync2(connection, sslbuf, sizeof(sslbuf));

	websocket_send(connection, subactions, subaction_size);
	while ((nbytes=openssl_read_sync2(connection, sslbuf+packet_size, sizeof(sslbuf)-packet_size-64))) {
		packet_size += nbytes;
		nr_frames = websocket_recv(sslbuf, nbytes, &frames[0], wsbuf, 0);
		printf("nr_frames: %d packet: %s\n", nr_frames, wsbuf);
		update_crypto_data(wsbuf);
	}
}

void *cryptocompare_thread(void *args)
{
	websocket_connect_sync("streamer.cryptocompare.com", Server.CC_ADDR, "/v2?format=streamer", cryptocompare_handler);
	printf(BOLDGREEN "cryptocompare connected" RESET "\n");	
}
