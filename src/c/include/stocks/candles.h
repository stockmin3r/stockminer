#ifndef __CANDLES_H
#define __CANDLES_H

#include <stocks/stocks.h>

#define NR_CANDLES       61
#define CANDLE_BULL      100
#define CANDLE_BEAR     -100
#define CANDLE_BULL_BEAR 88
#define CBULL 1
#define CBEAR 0

struct candle {
	double         day1_bull;
	double         day3_bull;
	double         day7_bull;
	double         day21_bull;
	double         day1_bear;
	double         day3_bear;
	double         day7_bear;
	double         day21_bear;
	double         day1_bull_trend;
	double         day3_bull_trend;
	double         day7_bull_trend;
	double         day21_bull_trend;
	double         day1_bear_trend;
	double         day3_bear_trend;
	double         day7_bear_trend;
	double         day21_bear_trend;
	char          *bull_flags;
	char          *bear_flags;
	int            bull_flags_len;
	int            bear_flags_len;
	short          type;
	short          nr_bull_candles;
	short          nr_bear_candles;
};

struct CANDLE {
	char               date[16];
	char               D1[12];
	char               D2[12];
	char               D3[12];
	char               D4[12];
	char               D5[12];
	char               D6[12];
	char               D7[12];
	char               D8[12];
	char               D9[12];
	char               D10[12];
	char               D11[12];
	char               D12[12];
	char               D13[12];
	char               D14[12];
	char               D15[12];
	char               D21[12];
	double             day1;
	double             day2;
	double             day3;
	double             day4;
	double             day5;
	double             day6;
	double             day7;
	double             day8;
	double             day9;
	double             day10;
	double             day11;
	double             day12;
	double             day13;
	double             day14;
	double             day15;
	double             day21;
	char              *name;
	int                type;
	int                ctype;
	int                entry;
};

struct candle_type {
	int            candle;
	int            type;
	char          *name;
	int            nr_candlesticks;
	void          (*scan)(struct stock *stock, struct mag *mag, struct candle *candle, int start_entry, int nr_days, int *candles, int cdx);
};

extern struct candle_type *CTYPES;

/* GLOBAL CANDLE TABLE */
#define CANDLE_GTABLE "<table id=CDL class=XTAB>" \
                      "<thead>"                   \
                      "<tr>"                      \
                        "<th>Candle</th>"         \
                        "<th></th>"               \
                        "<th>1d</th>"             \
                        "<th>3d</th>"             \
                        "<th>7d</th>"             \
                        "<th>21d</th>"            \
                      "</tr>"                     \
                     "</thead>"                   \
                     "<tbody>"                    \

#define CANDLE_GENTRY "<tr>" \
            "<td>%s</td>"    \
            "<td></td>"      \
            "<td %s</td>"    \
            "<td %s</td>"    \
            "<td %s</td>"    \
            "<td %s</td>"    \
            "</td>"

#define CANDLE_STOCK_JSON "{\"C\":\"%s\",\"D\":\"%s\",\"1d\":%.2f,\"2d\":%.2f,\"3d\":%.2f,\"4d\":%.2f,\"5d\":%.2f,\"6d\":%.2f,\"7d\":%.2f,\"8d\":%.2f,\"9d\":\"%.2f\",\"10d\":%.2f,\"11d\":%.2f,\"12d\":%.2f,\"13d\":%.2f,\"14d\":%.2f,\"15d\":%.2f,\"T\":\"%d\"},"

/* candle-> FLAG_SERIES contains BOTH BULL & BEAR Candle Flags */
#define FLAG_SERIES	"{\"type\":\"flags\"," \
            "\"id\":\"CSP\","              \
            "\"onSeries\":\"%s-price\","   \
            "\"onKey\":\"high\","          \
            "\"data\":["

/* Candle->bull_flags/bear_flags specific candle series */
#define BULL_FLAGS	"{\"type\":\"flags\"," \
            "\"id\":\"CSP-BULL\","         \
            "\"onSeries\":\"%s-price\","   \
            "\"color\":\"green\","         \
            "\"onKey\":\"high\","          \
            "\"shape\":\"squarepin\","     \
            "\"data\":["

#define BEAR_FLAGS	"{\"type\":\"flags\"," \
            "\"id\":\"CSP-BEAR\","         \
            "\"onSeries\":\"%s-price\","   \
            "\"color\":\"red\","           \
            "\"onKey\":\"high\","          \
            "\"data\":["

#define BEAR_CANDLE	"{\"x\":\"%lu\",\"color\":\"red\",\"title\":\"%s\"},"
#define BULL_CANDLE	"{\"x\":\"%lu\",\"color\":\"green\",\"title\":\"%s\"},"
#define FLAG_CANDLE	"{\"x\":\"%lu\",\"title\":\"%s\"},"

/* CANDLE SPECIFIC SERIES */
static __inline__ int alloc_bull_flags(struct candle *candle, char *ticker, int ctype)
{
	if (ctype == CANDLE_BULL || ctype == CANDLE_BULL_BEAR) {
		candle->bull_flags = (char *)malloc(12 KB);
		return sprintf(candle->bull_flags, FLAG_SERIES, ticker);
	}
	return 0;
}
/* CANDLE SPECIFIC SERIES */
static __inline__ int alloc_bear_flags(struct candle *candle, char *ticker, int ctype)
{
	if (ctype == CANDLE_BEAR || ctype == CANDLE_BULL_BEAR) {
		candle->bear_flags = (char *)malloc(12 KB);
		return sprintf(candle->bear_flags, FLAG_SERIES, ticker);
	}
	return 0;
}
/* ADD TO CANDLE FLAG SERIES: There are two separate series, bull & bear */
static __inline__ void candle_flag_series(struct candle *candle, time_t timestamp, int type, int ctype, char *candle_name, int *bull_flags_len, int *bear_flags_len)
{
	int bullsize = *bull_flags_len, bearsize = *bear_flags_len;
	if (type == CANDLE_BULL && (ctype == CANDLE_BULL || ctype == CANDLE_BULL_BEAR)) {
		candle->nr_bull_candles++;
		bullsize += sprintf(candle->bull_flags+bullsize, BULL_CANDLE, timestamp, candle_name);
	}
	if (type == CANDLE_BEAR && (ctype == CANDLE_BEAR || ctype == CANDLE_BULL_BEAR)) {
		candle->nr_bear_candles++;
		bearsize += sprintf(candle->bear_flags+bearsize, BEAR_CANDLE, timestamp, candle_name);
	}
	*bull_flags_len = bullsize;
	*bear_flags_len = bearsize;
}
/* CANDLE SPECIFIC TERMINATION */
static __inline__ void candle_flags_end(struct candle *candle, int bull_flags_len, int bear_flags_len)
{
	if (bull_flags_len) {
		*(candle->bull_flags+bull_flags_len-1) = ']';
		*(candle->bull_flags+bull_flags_len+0) = '}';
		*(candle->bull_flags+bull_flags_len+1) = 0;
		candle->bull_flags_len = (bull_flags_len + 1);
	}
	if (bear_flags_len) {
		*(candle->bear_flags+bear_flags_len-1) = ']';
		*(candle->bear_flags+bear_flags_len+0) = '}';
		*(candle->bear_flags+bear_flags_len+1) = 0;
		candle->bear_flags_len = (bear_flags_len + 1);
	}
}

static __inline__ const char *dt(double value, char *buf)
{
        if (value == 0.0)
                return "";
        if (value > 0)
                snprintf(buf, 50, "class=%s>+%.2f%%", "g", value);
        else
                snprintf(buf, 50, "class=%s>-%.2f%%", "r", value);
        return (buf);
}

extern char *candle_global_table;
extern int candle_global_tablesize;

typedef int (*CANDLE_FUNC) (int     startIdx,
							int     endIdx,
							double  inOpen[],
							double  inHigh[],
							double  inLow[],
							double  inClose[],
							int    *outBegIdx,
							int    *outNBElement,
							int     outInteger[]);

typedef int (*CANDLE_FUNC2)(int     startIdx,
							int     endIdx,
							double  inOpen[],
							double  inHigh[],
							double  inLow[],
							double  inClose[],
							double  optInPenetration,
							int    *outBegIdx,
							int    *outNBElement,
							int     outInteger[]);

struct candlefunc {
	CANDLE_FUNC  scan;
};

struct candlefunc2 {
	CANDLE_FUNC2 scan;
};

enum {
	CDL_3LINESTRIKE,
	CDL_3BLACKCROWS,
	CDL_TASUKIGAP,
	CDL_INVERTEDHAMMER,
	CDL_MATCHINGLOW,
	CDL_BREAKAWAY,
	CDL_PIERCING,
	CDL_STICKSANDWICH,
	CDL_THRUSTING,
	CDL_INNECK,
	CDL_3INSIDE,
	CDL_HOMINGPIGEON,
	CDL_IDENTICAL3CROWS,
	CDL_XSIDEGAP3METHODS,
	CDL_TRISTAR,
	CDL_GAPSIDESIDEWHITE,
	CDL_3WHITESOLDIERS,
	CDL_ONNECK,
	CDL_3OUTSIDE,
	CDL_RICKSHAWMAN,
	CDL_SEPARATINGLINES,
	CDL_LONGLEGGEDDOJI,
	CDL_HARAMI,
	CDL_LADDERBOTTOM,
	CDL_CLOSINGMARUBOZU,
	CDL_TAKURI,
	CDL_DOJISTAR,
	CDL_HARAMICROSS,
	CDL_ADVANCEBLOCK,
	CDL_SHOOTINGSTAR,
	CDL_MARUBOZU,
	CDL_UNIQUE3RIVER,
	CDL_2CROWS,
	CDL_BELTHOLD,
	CDL_HAMMER,
	CDL_HIGHWAVE,
	CDL_SPINNINGTOP,
	CDL_UPSIDEGAP2CROWS,
	CDL_GRAVESTONEDOJI,
	CDL_HIKKAKEMOD,
	CDL_HIKKAKE,
	CDL_ENGULFING,
	CDL_HANGINGMAN,
	CDL_RISEFALL3METHODS,
	CDL_KICKING,
	CDL_DRAGONFLYDOJI,
	CDL_CONCEALBABYSWALL,
	CDL_3STARSINSOUTH,
	CDL_DOJI,
	CDL_COUNTERATTACK,
	CDL_LONGLINE,
	CDL_STALLEDPATTERN,
	CDL_SHORTLINE,
	CDL_KICKINGBYLENGTH,
	CDL_MORNINGSTAR,
	CDL_MORNINGDOJISTAR,
	CDL_EVENINGSTAR,
	CDL_EVENINGDOJISTAR,
	CDL_ABANDONEDBABY,
	CDL_DARKCLOUDCOVER,
	CDL_MATHOLD
};

#endif
