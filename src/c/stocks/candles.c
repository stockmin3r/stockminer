#include <conf.h>
#include <stocks/stocks.h>
#include <stocks/candletypes.h>

// many candles are not correctly processed, not sure where the problem lies
// - aside from the Engulfing pattern which i sometimes see matching on tradeview,
// but then i also see Engulfing (bearish or bullish) candles missing on tradeview
// that I see here, which makes all this very confusing. There are many online 
// resources on candles and it is difficult to say which is the best way to analyse them,
// i think that there may need to be a mechanism to report "fake" candles, which at the
// moment is probably most of them! First the source of so many fake candles needs to be determined
// most likely the error is in this very file. Note: everything is currently broken

/*
 * stock->candle_json    - CANDLE_STOCK_JSON - CRT Table All Candles for 1 Stock
 * candle_global_table   - GLOBAL (All Stocks) Candle Monster
 * candle_table          - Candle specific Table for ONE Candle
 * candle->bull_flags    - All Bullish Candles for 1 specific candle
 * candle->bear_flags    - All Bearish Candles for 1 specific candle
 * stock->bull_flags     - All Bullish Candles for one stock
 * stock->bear_flags     - All Bearish Candles for one stock
 * stock->mag->candles[] - Every Candle a stock has had
 * stock->candles[]      - Group Performance of All 100 Candles (Per Stock)
 */

struct candle_type candle_types[] = {
    {	CDL_3LINESTRIKE,      CANDLE_BULL_BEAR, "3LS", 3, NULL },
    {   CDL_3BLACKCROWS,      CANDLE_BEAR,      "3BC", 3, NULL },
    {   CDL_TASUKIGAP,        CANDLE_BULL_BEAR, "TG",  2, NULL },
    {   CDL_INVERTEDHAMMER,   CANDLE_BULL,      "IH",  1, NULL },
    {   CDL_MATCHINGLOW,      CANDLE_BULL,      "ML",  2, NULL },
    {   CDL_BREAKAWAY,        CANDLE_BULL_BEAR, "BRK", 5, NULL },
    {   CDL_PIERCING,         CANDLE_BULL,      "PRC", 2, NULL },
    {   CDL_STICKSANDWICH,    CANDLE_BULL,      "ST",  3, NULL },
    {   CDL_THRUSTING,        CANDLE_BEAR,      "THR", 2, NULL },
    {   CDL_INNECK,           CANDLE_BEAR,      "INK", 2, NULL },
    {   CDL_3INSIDE,          CANDLE_BULL_BEAR, "3IN", 3, NULL },
    {   CDL_HOMINGPIGEON,     CANDLE_BULL,      "HP",  2, NULL },
    {   CDL_IDENTICAL3CROWS,  CANDLE_BEAR,      "I3C", 3, NULL },
    {   CDL_XSIDEGAP3METHODS, CANDLE_BULL_BEAR, "SG3", 3, NULL },
    {   CDL_TRISTAR,          CANDLE_BULL_BEAR, "TRI", 3, NULL },
    {   CDL_GAPSIDESIDEWHITE, CANDLE_BULL_BEAR, "GSW", 2, NULL },
    {   CDL_3WHITESOLDIERS,   CANDLE_BULL,      "3WS", 3, NULL },
    {   CDL_ONNECK,           CANDLE_BEAR,      "ON",  2, NULL },
    {   CDL_3OUTSIDE,         CANDLE_BULL_BEAR, "3O",  3, NULL },
    {   CDL_RICKSHAWMAN,      CANDLE_BULL,      "RSM", 1, NULL }, /* Indecition */
    {   CDL_SEPARATINGLINES,  CANDLE_BULL_BEAR, "SEP", 2, NULL },
    {   CDL_LONGLEGGEDDOJI,   CANDLE_BULL,      "LNG", 1, NULL }, /* Indecision */
    {   CDL_HARAMI,           CANDLE_BULL_BEAR, "HR",  2, NULL },
    {   CDL_LADDERBOTTOM,     CANDLE_BULL,      "LB",  5, NULL },
    {   CDL_CLOSINGMARUBOZU,  CANDLE_BULL_BEAR, "CM",  1, NULL },
    {   CDL_TAKURI,           CANDLE_BULL,      "TAK", 1, NULL },
    {   CDL_DOJISTAR,         CANDLE_BULL_BEAR, "DS",  1, NULL },
    {   CDL_HARAMICROSS,      CANDLE_BULL_BEAR, "HC",  2, NULL },
    {   CDL_ADVANCEBLOCK,     CANDLE_BEAR,      "ABS", 3, NULL },
    {   CDL_SHOOTINGSTAR,     CANDLE_BEAR,      "SS",  1, NULL },
    {   CDL_MARUBOZU,         CANDLE_BULL_BEAR, "MB",  1, marubozu_scan },
    {   CDL_UNIQUE3RIVER,     CANDLE_BULL,      "UR",  3, NULL },
    {   CDL_2CROWS,           CANDLE_BEAR,      "2C",  2, NULL },
    {   CDL_BELTHOLD,         CANDLE_BULL_BEAR, "BH",  1, NULL },
    {   CDL_HAMMER,           CANDLE_BULL,      "HMR", 1, NULL },
    {   CDL_HIGHWAVE,         CANDLE_BULL_BEAR, "HW",  1, NULL },
    {   CDL_SPINNINGTOP,      CANDLE_BULL_BEAR, "SPN", 1, NULL },
    {   CDL_UPSIDEGAP2CROWS,  CANDLE_BEAR,      "U2C", 2, NULL },
    {   CDL_GRAVESTONEDOJI,   CANDLE_BULL,      "GSD", 1, NULL },
    {   CDL_HIKKAKEMOD,       CANDLE_BULL_BEAR, "HKM", 4, NULL },
    {   CDL_HIKKAKE,          CANDLE_BULL_BEAR, "HKK", 3, NULL },
    {   CDL_ENGULFING,        CANDLE_BULL_BEAR, "BE",  2, NULL },
    {   CDL_HANGINGMAN,       CANDLE_BEAR,      "HM",  1, NULL },
    {   CDL_RISEFALL3METHODS, CANDLE_BULL_BEAR, "RF3", 5, NULL },
    {   CDL_KICKING,          CANDLE_BULL_BEAR, "KIK", 2, NULL },
    {   CDL_DRAGONFLYDOJI,    CANDLE_BULL,      "DFD", 1, NULL }, /* Indecision */
    {   CDL_CONCEALBABYSWALL, CANDLE_BULL,      "CBS", 4, NULL },
    {   CDL_3STARSINSOUTH,    CANDLE_BULL,      "3SS", 3, NULL },
    {   CDL_DOJI,             CANDLE_BULL,      "DOJ", 1, NULL }, /* Indecision */
    {   CDL_COUNTERATTACK,    CANDLE_BULL_BEAR, "CA",  2, NULL },
    {   CDL_LONGLINE,         CANDLE_BULL_BEAR, "LL",  1, NULL },
    {   CDL_STALLEDPATTERN,   CANDLE_BEAR,      "SP",  3, NULL },
    {   CDL_SHORTLINE,        CANDLE_BULL_BEAR, "SL",  1, NULL },
    {   CDL_KICKINGBYLENGTH,  CANDLE_BULL_BEAR, "KBL", 2, NULL },
    {   CDL_MORNINGSTAR,      CANDLE_BEAR,      "MS",  3, NULL },
    {   CDL_MORNINGDOJISTAR,  CANDLE_BULL,      "MDS", 3, NULL },
    {   CDL_EVENINGSTAR,      CANDLE_BEAR,      "ES",  3, NULL },
    {   CDL_EVENINGDOJISTAR,  CANDLE_BEAR,      "EDS", 3, NULL },
    {   CDL_ABANDONEDBABY,    CANDLE_BULL_BEAR, "AB",  3, NULL },
    {   CDL_DARKCLOUDCOVER,   CANDLE_BEAR,      "DCC", 2, NULL },
    {   CDL_MATHOLD,          CANDLE_BULL,      "MH",  5, NULL }
};

struct candle_type  *CTYPES = &candle_types[0];
struct candlefunc    CDL[55];
struct candlefunc2   CDL2[8];
struct candle        global_candles[NR_CANDLES];
char                *candle_global_table;
int                  candle_global_tablesize;
char                *candle_screener;
int                  candle_screener_size;
bool                 CANDLES_LOADED = false; // only load if build/libta_lib.so is present

int candle_ignore(int x)
{
	int z;

	for (z=0; z<NR_CANDLE_IGNORE; z++) {
		if (x == CDL_IGNORE[z])
			return 1;
	}
	return 0;
}

void hammer_scan(struct stock *stock, struct mag *mag, struct candle *candle, int start_entry, int nr_days, int *candles, int cdx)
{
	double xopen, xhigh, xlow, xclose, body;
	int x;

	for (x=0; x<nr_days; x++) {
		xopen  = mag->open[start_entry+x];
		xhigh  = mag->high[start_entry+x];
		xlow   = mag->low[start_entry+x];
		xclose = mag->close[start_entry+x];

		/* Get the Open -> Close spread */
		body   = fabs(((xopen/xclose)-1)*100.0);

		/* If Open is the same as Low then there is now bottom shaddow */
		if (xopen == xlow) {

		}
	}
}

void marubozu_scan(struct stock *stock, struct mag *mag, struct candle *candle, int start_entry, int nr_days, int *candles, int cdx)
{
	double xopen, xhigh, xlow, xclose;
	int x;

	for (x=0; x<nr_days; x++) {
		xopen  = mag->open[start_entry+x];
		xhigh  = mag->high[start_entry+x];
		xlow   = mag->low[start_entry+x];
		xclose = mag->close[start_entry+x];

		/* Red Candle */
		if (xopen > xclose) {
			if (xopen != xhigh)
				continue;
			if (xclose != xlow)
				continue;
			candles[x] = -100; // bear marubozu
		/* Green Candle */
		} else if (xopen < xclose) {
			if (xclose != xhigh)
				continue;
			if (xopen != xlow)
				continue;
			candles[x] = 100; // bull marubozu
		}
	}
}


/* CMT - GLOBAL CANDLE MONSTER TABLE */
void build_candle_monster(struct XLS *XLS)
{
	struct stock  *stock, **STOCKS;
	struct candle *candle;
	char *jptr;
	char dbuf1[64];char dbuf2[64]; char dbuf3[64]; char dbuf4[64];
	int x, y, csize, nr_stocks, nr_bear_candles = 0, nr_bull_candles = 0, do_bull, do_bear;

	return;
	for (x=0; x<NR_CANDLES; x++) {
		if (candle_ignore(x))
			continue;
		candle = &global_candles[x];
		nr_bull_candles = 0;
		nr_bear_candles = 0;
		if (candle_types[x].type == CANDLE_BULL)
			do_bull = 1;
		else if (candle_types[x].type == CANDLE_BEAR)
			do_bear = 1;
		else if (candle_types[x].type == CANDLE_BULL_BEAR) {
			do_bull = 1;
			do_bear = 1;
		}

		nr_stocks = XLS->nr_stocks;
		STOCKS    = XLS->STOCKS_PTR;
		for (y=0; y<nr_stocks; y++) {
			stock = STOCKS[y];
			if (!stock || !stock->candles || !stock->candles[x])
				continue;
			if (do_bull) {
				candle->day1_bull  += stock->candles[x]->day1_bull;
				candle->day3_bull  += stock->candles[x]->day3_bull;
				candle->day7_bull  += stock->candles[x]->day7_bull;
				candle->day21_bull += stock->candles[x]->day21_bull;
				nr_bull_candles    += stock->candles[x]->nr_bull_candles;
			}
			if (do_bear) {
				candle->day1_bear  += stock->candles[x]->day1_bear;
				candle->day3_bear  += stock->candles[x]->day3_bear;
				candle->day7_bear  += stock->candles[x]->day7_bear;
				candle->day21_bear += stock->candles[x]->day21_bear;
				nr_bear_candles    += stock->candles[x]->nr_bear_candles;
			}
		}
		if (do_bear) {
			if (candle->day1_bear)
				candle->day1_bear  /= (double)nr_bear_candles;
			if (candle->day3_bear)
				candle->day3_bear  /= (double)nr_bear_candles;
			if (candle->day7_bear)
				candle->day7_bear  /= (double)nr_bear_candles;
			if (candle->day21_bear)
				candle->day21_bear /= (double)nr_bear_candles;
			candle->nr_bear_candles = nr_bear_candles;
			if (!strcmp(stock->sym, "AAPL") && x == CDL_ENGULFING)
				printf(BOLDRED "%-23s " RESET BOLDWHITE "candle day1: %.2f day3: %.2f day7: %.2f day21: %.2f nr_candles: %d" RESET "\n", CANDLE_NAME[x], candle->day1_bear, candle->day3_bear, candle->day7_bear, candle->day21_bear, nr_bear_candles);
		}
		if (do_bull) {
			if (candle->day1_bull)
				candle->day1_bull  /= (double)nr_bull_candles;
			if (candle->day3_bull)
				candle->day3_bull  /= (double)nr_bull_candles;
			if (candle->day7_bull)
				candle->day7_bull  /= (double)nr_bull_candles;
			if (candle->day21_bull)
				candle->day21_bull /= (double)nr_bull_candles;
			candle->nr_bull_candles = nr_bull_candles;
//			if (!strcmp(stock->sym, "AAPL") && x == CDL_ENGULFING)
//				printf(BOLDGREEN "%-23s " RESET BOLDWHITE "candle day1: %.2f day3: %.2f day7: %.2f day21: %.2f nr_candles: %d" RESET "\n", CANDLE_NAME[x], candle->day1_bull, candle->day3_bull, candle->day7_bull, candle->day21_bull, nr_bull_candles);
		}
	}

	/* GLOBAL CANDLE TABLE */
	jptr = candle_global_table + candle_global_tablesize;
	for (x=0; x<NR_CANDLES; x++) {
		candle = &global_candles[x];
		csize = sprintf(jptr, CANDLE_GENTRY, CANDLE_NAME[x], dt(candle->day1_bear, dbuf1), dt(candle->day3_bear, dbuf2), dt(candle->day7_bear, dbuf3), dt(candle->day21_bear, dbuf4));
		jptr += csize;
		candle_global_tablesize += csize;
	}
	strcpy(jptr, "</tbody></table>");
	candle_global_tablesize += 16;
}

void rpc_stockpage_candle(struct rpc *rpc)
{
	char           *packet      = rpc->packet;
	char           *ticker      = rpc->argv[1];
	char           *div         = rpc->argv[2];
	int             candle_type = atoi(rpc->argv[3]);
	struct stock   *stock       = search_stocks(ticker);
	struct mag     *mag;
	struct CANDLE **CANDLES, *C;
	struct candle  *candle;
	int             packet_len, x;
	double          avg = 0.0;

	if (!stock || !(mag=stock->mag))
		return;
	if (candle_type == CBULL) {
		if (!stock->bull_candles[0])
			return;
		CANDLES = stock->bull_candles;
	} else if (candle_type == CBEAR) {
		if (!stock->bear_candles[0])
			return;
		CANDLES = stock->bear_candles;
	}
	C      = CANDLES[0];
	candle = stock->candles[C->ctype];
	if (candle->type == CANDLE_BULL)
		avg = candle->day7_bull;
	else if (candle->type == CANDLE_BEAR)
		avg = candle->day7_bear;
	/* svgClassName div date recentCandlesTable */
	packet_len = sprintf(packet, "candy %s %s%d %d %s %s %.2f [", ticker, candle_types[C->ctype].name, candle_type, candle_type, div+1, stock->mag->date[C->entry], avg);

	for (x=0; x<5; x++) {
		if (!(C=CANDLES[x]))
			break;
		packet_len += sprintf(packet+packet_len, "{\"800\":\"%s\",\"915\":\"%s\",\"853\":\"%s\",\"855\":\"%s\",\"860\":\"%s\",\"865\":\"%s\",\"871\":\"%s\"},",
							  CANDLE_NAME[C->ctype], C->date, C->D3, C->D5, C->D10, C->D15, C->D21);
	}
	packet[packet_len-1] = ']';
	websocket_send(rpc->connection, packet, packet_len);
}

void build_candle_screener(struct XLS *XLS)
{
	struct stock  *stock, **stocks;
	struct mag    *mag;
	struct CANDLE *C;
	int            x, nr_stocks;

	candle_screener = (char *)malloc(512 KB);
	strcpy(candle_screener, "csr [");
	candle_screener_size = 5;

	stocks    = XLS->STOCKS_PTR;
	nr_stocks = XLS->nr_stocks;
	for (x=0; x<nr_stocks; x++) {
		stock = stocks[x];
		mag   = stock->mag;
		if (stock->rank == -1 || stock->rank > 200 || !mag || !mag->nr_candles || !(C=stock->last_candle))
			continue;

		candle_screener_size += sprintf(candle_screener+candle_screener_size,
								"{\"900\":\"%s\",\"800\":\"%s\",\"915\":\"%s\",\"852\":\"%s\",\"853\":\"%s\",\"854\":\"%s\",\"855\":\"%s\",\"899\":\"%s\"},",
								stock->sym, CANDLE_NAME[C->ctype], C->date, C->D2, C->D3, C->D4, C->D5, C->type==CANDLE_BULL?"Bull":"Bear");
	}
	candle_screener[candle_screener_size-1] = ']';
}

static __inline__ void candle_mini_table(struct stock *stock)
{
	struct mag *mag = stock->mag;
	int x, nr_bulls = 0, nr_bears = 0, nr_candles = mag->nr_candles;

//	for (x=0; x<nr_days; x++) {
	for (x=0; x<nr_candles; x++) {
//		struct CANDLE *C = &mag->candles[nr_days-x-1];
		struct CANDLE *C = &mag->candles[nr_candles-x-1];
		if (!C->name)
			continue;
		switch (C->type) {
			case CANDLE_BULL:
				if (nr_bulls < 5)
					stock->bull_candles[nr_bulls++] = C;
				break;
			case CANDLE_BEAR:
				if (nr_bears < 5)
					stock->bear_candles[nr_bears++] = C;
				break;
		}
		if (!stock->last_candle)
			stock->last_candle = C;
		if (nr_bulls == 5 && nr_bears == 5)
			break;
	}
}

static __inline__ void stock_candle_series(struct stock *stock, struct mag *mag, struct CANDLE *C, int *bull_flags_len, int *bear_flags_len)
{
	int bullsize = *bull_flags_len, bearsize = *bear_flags_len;

	if (C->type == CANDLE_BULL) {
		bullsize += sprintf(stock->bull_flags+bullsize, FLAG_CANDLE, str2unix(mag->date[C->entry+MIN(mag->nr_entries-C->entry-1,2)])*1000, C->name);
		*bull_flags_len = bullsize;
	} else if (C->type == CANDLE_BEAR) {
		bearsize += sprintf(stock->bear_flags+bearsize, FLAG_CANDLE, str2unix(mag->date[C->entry+MIN(mag->nr_entries-C->entry-1,2)])*1000, C->name);
		*bear_flags_len = bearsize;
	}
}
/* STOCK CANDLES TERMINATION */
static __inline__ void stock_candles_end(struct stock *stock, int bull_flags_len, int bear_flags_len)
{
	if (bull_flags_len) {
		*(stock->bull_flags+bull_flags_len-1) = ']';
		*(stock->bull_flags+bull_flags_len+0) = '}';
		*(stock->bull_flags+bull_flags_len+1) = 0;
		stock->bull_flags_len = (bull_flags_len + 1);
	}
	if (bear_flags_len) {
		*(stock->bear_flags+bear_flags_len-1) = ']';
		*(stock->bear_flags+bear_flags_len+0) = '}';
		*(stock->bear_flags+bear_flags_len+1) = 0;
		stock->bear_flags_len = (bear_flags_len + 1);
	}
}

/*
 *
 * PROCESS ONE CANDLE PER STOCK
 *
 */
void candle_process(struct stock *stock, struct candle *candle, int mag_entry, int nr_days, int *candles, int candle_id)
{
	struct mag *mag = stock->mag;
	int x, y, candle_type, offset, nr_candles = 0, nbytes = 0, bull_flags_len = 0, bear_flags_len = 0, nr_bulls=0, nr_bears=0, bulls = 0, bears = 0;
	char *candle_name       = candle_types[candle_id].name;
	int nr_candlesticks     = candle_types[candle_id].nr_candlesticks;
	int ctype               = candle_types[candle_id].type;
	double total;
	char cbuf[512];

	// struct candle *candle == &stock->candles[candle_id]

	bull_flags_len = alloc_bull_flags(candle, stock->sym, ctype);
	bear_flags_len = alloc_bear_flags(candle, stock->sym, ctype);
	candle->type   = ctype;

	nr_candles = mag->nr_candles;
	for (x=0; x<nr_days; x++) {
		if (!candles[x]) {
			mag_entry++;
			continue;
		}
		candle_type = candles[x];
		offset      = (mag_entry + 2);
		total       = 0;
		if (*mag->candles[nr_candles].date != '\0')
			fs_newfile("clog.txt", cbuf, sprintf(cbuf, BOLDRED "candle overwrite: [%s] %s %s overwritten by %s %d" RESET "\n", stock->sym, mag->candles[nr_candles].date, mag->candles[nr_candles].name,candle_types[candle_id].name, mag_entry));

		mag->candles[nr_candles].name  = candle_types[candle_id].name;
		mag->candles[nr_candles].type  = candle_type;
		mag->candles[nr_candles].ctype = candle_id;
		mag->candles[nr_candles].entry = mag_entry;
		strcpy(mag->candles[nr_candles].date, mag->date[mag_entry]);
		nr_candles += 1;

		if (nr_candles >= mag->max_candles) {
			mag->candles = (struct CANDLE *)realloc(mag->candles, (nr_candles+32)*sizeof(struct CANDLE));
			mag->max_candles += 32;
		}
		if (verbose && !strcmp(stock->sym, "AAPL") && (candle_id == CDL_ENGULFING || candle_id == CDL_INVERTEDHAMMER))
			printf(BOLDWHITE "%p name: %s date: %s entry: %d x: %d xoff: %d" RESET "\n", &mag->candles[nr_candles], candle_types[candle_id].name, mag->candles[nr_candles].date, mag_entry, x, mag_entry + MIN(2, nr_candlesticks));
		if (verbose && !strcmp(stock->sym, "AAPL") && !strcmp(mag->candles[nr_candles].date, "2022-08-30"))
			printf(BOLDMAGENTA "%s %p name: %s date: %s entry: %d x: %d xoff: %d" RESET "\n", stock->sym, &mag->candles[nr_candles], candle_types[candle_id].name, mag->candles[nr_candles].date, mag_entry, x, mag_entry + MIN(2, nr_candlesticks));

		if (candle_type == CANDLE_BULL)
			bulls++;
		else if (candle_type == CANDLE_BEAR)
			bears++;

		/* Add to this stock's Candle[candle_id] Flag Series */
		candle_flag_series(candle, str2unix(mag->date[mag_entry])*1000, candle_type, ctype, candle_name, &bull_flags_len, &bear_flags_len);		
		for (y=0; y<21; y++) {
			if (offset+y >= mag->nr_entries)
				break;
			total += delta(mag->close[offset+y], mag->close[offset+y-1]);
/*			if (!strcmp(stock->sym, "AAPL") && candle_id == CDL_ENGULFING)
				printf(BOLDYELLOW "%s [%d]  [%.2f vs %.2f] d: %.2f t: %.2f" RESET "\n"
				BOLDCYAN  "(1d:  [%.2f vs %.2f] chgpc: %.2f) " RESET
				BOLDGREEN "(3d:  [%.2f vs %.2f] chgpc: %.2f) " RESET
				BOLDBLUE  "(21d: [%.2f vs %.2f] chgpc: %.2f) " RESET "\n",mag->date[offset+y], y,
				mag->close[offset+y],  mag->close[offset+y-1],  delta(mag->close[offset+y],  mag->close[offset+y-1]), total,
				mag->close[offset+1],  mag->close[offset+1-1],  delta(mag->close[offset+1],  mag->close[offset+1-1]),
				mag->close[offset+3],  mag->close[offset+3-1],  delta(mag->close[offset+3],  mag->close[offset+3-1]),
				mag->close[offset+21], mag->close[offset+21-1], delta(mag->close[offset+20], mag->close[offset+20-1]));			*/
			if (candle_type == CANDLE_BULL) {
				switch (y) {
					case 1:  candle->day1_bull  += total; break;
					case 3:  candle->day3_bull  += total; break;
					case 7:  candle->day7_bull  += total; break;
					case 20: candle->day21_bull += total; break;
				}
			} else if (candle_type == CANDLE_BEAR) {
				switch (y) {
					case 1:  candle->day1_bear  += total; break;
					case 3:  candle->day3_bear  += total; break;
					case 7:  candle->day7_bear  += total; break;
					case 20: candle->day21_bear += total; break;
				}
			}
		}
/*		if (!strcmp(stock->sym, "AAPL") && candle_id == CDL_ENGULFING && type==CANDLE_BEAR)
			printf(BOLDRED "[%s] day1: %.2f day3: %.2f day7: %.2f day21: %.2f" RESET "\n", mag->date[offset-1], candle->day1_bear, candle->day3_bear, candle->day7_bear, candle->day21_bear);
		else if (!strcmp(stock->sym, "AAPL") && candle_id == CDL_ENGULFING && type==CANDLE_BULL)
			printf(BOLDGREEN "[%s] day1: %.2f day3: %.2f day7: %.2f day21: %.2f" RESET "\n", mag->date[offset-1], candle->day1_bull, candle->day3_bull, candle->day7_bull, candle->day21_bull);*/
		mag_entry += 1;
	}
	mag->nr_candles = nr_candles;

	if (candle->day1_bull)
		candle->day1_bull  /= bulls;
	if (candle->day1_bear)
		candle->day1_bear  /= bears;
	if (candle->day3_bull)
		candle->day3_bull  /= bulls;
	if (candle->day3_bear)
		candle->day3_bear  /= bears;
	if (candle->day7_bull)
		candle->day7_bull  /= bulls;
	if (candle->day7_bear)
		candle->day7_bear  /= bears;
	if (candle->day21_bull)
		candle->day21_bull /= bulls;
	if (candle->day21_bear)
		candle->day21_bear /= bears;
	candle_flags_end(candle, bull_flags_len, bear_flags_len);
}

/*
 * Scan all candles for one stock
 */
void candle_scan(struct stock *stock, struct mag *mag)
{
//	int year_index = (mag->year_2020 == -1) ? (mag->year_2021) : (mag->year_2020);
	int year_index = mag->year_2021;
	int nr_days    = mag->nr_entries-year_index-1;
	int candles[nr_days+1];
	int beginIndex, endIndex, x, y = 0, candle_day, days_left, bull_flags_len, bear_flags_len, nr_candles, nr_candlesticks;
	double prior_close, delta, next_close;

	if (!CANDLES_LOADED)
		return;

	stock->bull_flags   = (char *)malloc(32 KB);
	stock->bear_flags   = (char *)malloc(32 KB);
	bull_flags_len      = sprintf(stock->bull_flags, BULL_FLAGS, stock->sym);
	bear_flags_len      = sprintf(stock->bear_flags, BEAR_FLAGS, stock->sym);
	stock->mag->candles = (struct CANDLE *)zmalloc((nr_days/2)*sizeof(struct CANDLE));
	strcpy(stock->candle_json, "candle [");
	stock->candle_json_size = 8;
	stock->mag->max_candles = (nr_days/2);

	for (x=0; x<NR_CANDLES; x++) {
		if (candle_ignore(x))
			continue;

		memset(candles, 0, sizeof(candles));
		if (x >= 54)
			CDL2[y++].scan(0, nr_days,  &mag->open[year_index], &mag->high[year_index], &mag->low[year_index], &mag->close[year_index], 0.0, &beginIndex, &endIndex, candles);
		else
			CDL[x].scan   (0, nr_days,  &mag->open[year_index], &mag->high[year_index], &mag->low[year_index], &mag->close[year_index], &beginIndex, &endIndex, candles); 
		if (!endIndex)
			continue;
		stock->candles[x] = (struct candle *)zmalloc(sizeof(struct candle));
		candle_process(stock, stock->candles[x], year_index+beginIndex, nr_days-beginIndex-1, candles, x);
		if (verbose && !strcmp(stock->sym, "AAPL")) {
			printf("%s BEGIN: %d END: %d\n", candle_types[x].name, beginIndex, endIndex);
//			printf(BOLDCYAN   "%s" RESET "\n", stock->candles[x]->bull_flags);
//			printf(BOLDYELLOW "%s" RESET "\n", stock->candles[x]->bear_flags);
		}
	}
	candle_mini_table(stock);

	/* ********************
	 *    Candle Radar
	**********************/
	nr_candles = mag->nr_candles;
	for (x=0; x<nr_candles; x++) {
		struct CANDLE *C = &mag->candles[x];
		if (!C->entry)
			continue;
		nr_candlesticks = candle_types[C->ctype].nr_candlesticks;
		days_left       = MIN(21, (mag->nr_entries - C->entry - 1 - nr_candlesticks)); // days left before today
		candle_day      = (C->entry + nr_candlesticks);
		prior_close     = mag->close[candle_day];
        for (y=1; y<days_left; y++) {
            next_close = mag->close[candle_day+y];
            delta      = ((next_close/prior_close)-1)*100;
            switch (y) {
                case 1:C->day1   = delta;sprintf(C->D1,  "%.2f", delta);break;case 2:C->day2   = delta;sprintf(C->D2,  "%.2f", delta);break;
				case 3:C->day3   = delta;sprintf(C->D3,  "%.2f", delta);break;case 4:C->day4   = delta;sprintf(C->D4,  "%.2f", delta);break;
                case 5:C->day5   = delta;sprintf(C->D5,  "%.2f", delta);break;case 6:C->day6   = delta;sprintf(C->D6,  "%.2f", delta);break;
                case 7:C->day7   = delta;sprintf(C->D7,  "%.2f", delta);break;case 8:C->day8   = delta;sprintf(C->D8,  "%.2f", delta);break;
				case 9:C->day9   = delta;sprintf(C->D9,  "%.2f", delta);break;case 10:C->day10 = delta;sprintf(C->D10, "%.2f", delta);break;
                case 11:C->day11 = delta;sprintf(C->D11, "%.2f", delta);break;case 12:C->day12 = delta;sprintf(C->D12, "%.2f", delta);break;
                case 13:C->day13 = delta;sprintf(C->D13, "%.2f", delta);break;case 14:C->day14 = delta;sprintf(C->D14, "%.2f", delta);break;
				case 15:C->day15 = delta;sprintf(C->D15, "%.2f", delta);break;case 21:C->day21 = delta;sprintf(C->D21, "%.2f", delta);break;
			}
		}
		stock->candle_json_size += sprintf(stock->candle_json+stock->candle_json_size,CANDLE_STOCK_JSON,CANDLE_NAME[C->ctype],C->date,C->day1,C->day2,C->day3,C->day4,C->day5,C->day6,C->day7,C->day8,C->day9,C->day10,C->day11,C->day12,C->day13,C->day14,C->day15, C->type==CANDLE_BULL?1:0);
		/* All Candles a stock has had (bearish & bullish) - for the FLAG SERIES of ALL CANDLES */
		stock_candle_series(stock, mag, C, &bull_flags_len, &bear_flags_len);
		if (stock->candle_json_size + 512 > stock->candle_json_max) {
			stock->candle_json     = (char *)realloc(stock->candle_json, stock->candle_json_size+4096);
			stock->candle_json_max = stock->candle_json_size+4096;
		}
	}
	stock_candles_end(stock, bull_flags_len, bear_flags_len);
	*(stock->candle_json+stock->candle_json_size-1) = ']';
}

void rpc_candle_stock(struct rpc *rpc)
{
	char         *ticker = rpc->argv[1];
	struct stock *stock  = search_stocks(ticker);

	if (!stock)
		return;

	websocket_send(rpc->connection, stock->candle_json, stock->candle_json_size);
}

void rpc_candle_zoom(struct rpc *rpc)
{
	char          *packet = rpc->packet;
	char          *ticker = rpc->argv[1];
	char          *date   = rpc->argv[2];
	char          *QGID   = rpc->argv[3];
	struct stock  *stock  = search_stocks(ticker);
	struct mag    *mag;
	struct CANDLE *C;
	uint64_t       d1;
	unsigned short d2;
	int            packet_len, candle_point, start_entry, x;
	time_t         timestamp;

	if (!stock || strlen(date) != 10)
		return;
	timestamp = str2unix(date)*1000;

	d1  = *(uint64_t *)date;
	d2  = *(unsigned short *)(date+8);
	mag = stock->mag;
	if (!mag || mag->nr_candles <= 0)
		return;

	packet_len = snprintf(packet, 64, "czoom %s %s %s ", ticker, date, QGID);
	for (x=0; x<mag->nr_candles; x++) {
		struct CANDLE *candle = &mag->candles[x];
		if (!candle)
			continue;
		if (*(uint64_t *)candle->date == d1 && *(unsigned short *)(candle->date+8) == d2) {
			if (mag->year_2020 != -1)
				start_entry = mag->year_2020;
			else if (mag->year_2021 == -1)
				start_entry = mag->year_2021;
			else if (mag->year_2022 == -1)
				start_entry = mag->year_2022;

			candle_point = (candle->entry-start_entry);
			packet_len  += sprintf(packet+packet_len, "%d ", candle_point);
			printf("candle entry: %d 2021: %d candle_point: %d\n", candle->entry, mag->year_2021, candle_point);
			if (candle->type == CANDLE_BULL) {
				packet_len += snprintf(packet+packet_len, 256, BULL_FLAGS, ticker);
				packet_len += snprintf(packet+packet_len, 256, BULL_CANDLE, timestamp, candle->name);
			} else if (candle->type == CANDLE_BEAR) {
				packet_len += snprintf(packet+packet_len, 256, BEAR_FLAGS, ticker);
				packet_len += snprintf(packet+packet_len, 256, BEAR_CANDLE, timestamp, candle->name);
			}
			break;
		}
	}
	if (packet_len <= 64)
		return;
	packet[packet_len-1] = ']';
	packet[packet_len++] = '}';
	packet[packet_len]   = 0;
	printf("packet: %s\n", packet);
	websocket_send(rpc->connection, packet, packet_len);
}

/* CandleStick Patterns */
void rpc_csp(struct rpc *rpc)
{
	struct session *session = rpc->session;
	char           *ticker  = rpc->argv[1];
	char           *div     = rpc->argv[2];
	struct stock   *stock   = search_stocks(ticker);
	char            flagbuf[256 KB];
	int             flag_size;

	if (!stock)
		return;

	flag_size  = snprintf(flagbuf, 24, "csp %s ", div);
	memcpy(flagbuf+flag_size, stock->bull_flags, stock->bull_flags_len);
	flag_size += stock->bull_flags_len;
	flagbuf[flag_size++] = '!';
	memcpy(flagbuf+flag_size, stock->bear_flags, stock->bear_flags_len);
	flag_size += stock->bear_flags_len;
	websocket_send(rpc->connection, flagbuf, flag_size);
}

/* Candlestick Screener Table */
void rpc_csr(struct rpc *rpc)
{
	websocket_send(rpc->connection, candle_screener,candle_screener_size);
}

typedef void (*init_func)(void);

bool init_talib()
{
	void      *dso;
	init_func  init;
	int      (*lookback)();
	char       lookback_func[256];
	int        x, y = 0;

	dso  = (void *)   os_dlopen("build/libta_lib.so");
	if (!dso)
		return false;
	init = (init_func)os_dlsym(dso, "TA_Initialize");
	init();

	for (x=0; x<NR_CANDLES; x++) {
		if (x >= 54)
			CDL2[y++].scan = (CANDLE_FUNC2)os_dlsym(dso, CANDLES[x]);
		else
			CDL[x].scan    = (CANDLE_FUNC)os_dlsym(dso, CANDLES[x]);
/*		sprintf(lookback_func, "%s_Lookback", CANDLES[x]);
		lookback = os_dlsym(dso, lookback_func);
		printf("%s [%d]\n", CANDLES[x], lookback());*/
	}
	CANDLES_LOADED = true;
	return true;
}

void init_candles(struct server *server)
{
	if (CANDLES_LOADED)
		return;

	if (!init_talib()) {
		server->candles_enabled = 0;
		return;
	}
	candle_global_table     = (char *)malloc(256 KB);
	strcpy(candle_global_table, CANDLE_GTABLE);
	candle_global_tablesize = sizeof(CANDLE_GTABLE)-1;
}
