#ifndef __STOCKS_H
#define __STOCKS_H

#include <conf.h>
#include <apc.h>
#include <stocks/api.h>
#include <stocks/algo.h>
#include <stocks/ufo.h>
#include <stocks/options.h>
#include <stocks/candles.h>
#include <stocks/ranks.h>
#include <stocks/forks.h>
#include <stocks/backtest.h>

#define PRE_MARKET             1
#define DAY_MARKET             2
#define AFH_MARKET             3
#define NO_MARKET              4

#define MARKET_NASDAQ          0
#define MARKET_NYSE            1
#define MARKET_OTC             2
#define MARKET_ASE             3
#define MARKET_CRYPTO          4

#define HIGHCAPS_TABLE         0
#define LOWCAPS_TABLE          1

#define STOCK_TYPE_STOCK       0
#define STOCK_TYPE_FOREX       1
#define STOCK_TYPE_CRYPTO      2
#define STOCK_TYPE_INDEX       3
#define STOCK_TYPE_FUND        4

#define STOCK_SUBTYPE_LOWCAPS  1
#define STOCK_SUBTYPE_HIGHCAPS 2

#define XLS_DATA_SOURCE_WSJ    1
#define XLS_DATA_SOURCE_WWW    2
#define XLS_DATA_SOURCE_YAHOO  4

#define NR_FUNDS              12
#define NR_MENU_STOCKS         4
#define MAX_DATA_RETRIES       5

#define SD_CHECKPOINT_EMPTY    0
#define SD_CHECKPOINT_PARTIAL  1
#define SD_CHECKPOINT_COMPLETE 2

#define STOCKDATA_OFF           0
#define STOCKDATA_YAHOO         1
#define STOCKDATA_WSJ           2
#define STOCKDATA_CRYPTOCOMPARE 3

struct price;
struct ohlc;
struct tick;
struct board;
struct wsid;
struct workspace;
struct connection;

extern int     market;

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
	int         nr_trading_minutes;
	char       *market_eod_prev_str; // %Y%M%D
	char       *market_eod_next_str; // %Y%M%D
	char       *prev_eod;
	char       *next_eod;
	time_t      prev_eod_timestamp;  // the last calendar EOD timestamp for this market
	time_t      next_eod_timestamp;  // next calendar trading day
	int         status;              // NO_MARKET|PRE_MARKET|DAY_MARKET|AFH_MARKET
	bool        holiday;             // is the market open today?
};

struct WSJ {
	char           *CUR;
	char           *ALL;
	char           *CLOSE;
	char           *CLOSE2;
	uint64_t        CUR_SZ;
	uint64_t        ALL_SZ;
	uint64_t        CLOSE_SZ;
	uint64_t        CLOSE2_SZ;
};

struct YAHOO {
	char           *OPTIONS_URL;
	char           *FIVE_YEAR;
	char           *STOCK_EOD;
	char           *OHLC;
	char           *CLOSE;
	char           *WATCH;
	char           *HISTORY;
	char           *OPTIONS_DATE;
	char           *OPTIONS_1D;
	char           *OPTIONS_15M;
	char           *OPTIONS_1M;
	char           *OPTION_CHECK;
};

struct API {
	struct WSJ      WSJ;
	struct YAHOO    YAHOO;
};

struct tick {
	char            current_ohlc[256];
	char            current_mini[256];
	int             current_ohlc_size;
	int             current_mini_size;
};

struct price {
	char            *price_1m;
	char            *price_1d;
	char            *price_mini;
	int              price_1m_len;
	int              price_mini_len;
	int              price_1d_len;
	char            *price_1d_close;
	int              price_1d_close_len;
	int              nr_points_1d;
	int              stale;
};

struct thread {
	struct stock   **stocks;
	struct XLS      *XLS;
	int              nr_stocks;
	int              epoll_fd;
	int              stop;
	int              stocks_per_thread;
	int              workload;
	int              id;
};

struct ohlc {
	double           open;
	double           high;
	double           low;
	double           close;
	time_t           timestamp;
	uint64_t         volume;
};

struct board {
	struct stock **stocks;
	void         (*update)(struct board *board);
	void         (*add)(struct stock *stock, struct board *board);
	int          (*format)(char *fmtstr);
	char          *fmtstr;
	char          *table;
	char          *buf1;
	char          *buf2;
	int            tsize;
	int            buffer;
	int            json_size1;
	int            json_size2;
	int            nr_stocks;
	int            max_stocks;
	int            dirty;
	int            offset;
};

struct earn {
	char           date[12];
	double         pc[30];
};

struct earnings {
	struct earn    earn[10];
	int            nr_earnings;
	char           next_earnings[32];
	char           prev_earnings[32];
	int            earning_days;
	time_t         next_ts;
};

struct fund_str {
	char    market_cap[12];
	char    enterprise_value[12];
	char    trailing_pe[12];
	char    forward_pe[12];
	char    peg_ratio[12];
	char    price_sales[12];
	char    PBR[12];
	char    enterprise_revenue[12];
	char    enterprise_EBITDA[12];
	/* Share Statistics */
	char    avg_volume_3mo[12];
	char    avg_volume_10d[12];
	char    shares_outstanding[12];
	char    float_value[12];
	char    insiders[12];
	char    institutions[12];
	char    shares_short[12];
	char    short_ratio[12];
	char    short_pc_float[12];
	char    short_pc_outstanding[12];
	char    shares_short_prior[12];

	/* Dividends */
	char    forward_annual_div_rate[12];
	char    forward_annual_div_yield[12];
	char    trailing_annual_div_rate[12];
	char    trailing_annual_div_yield[12];
	char    div_yield_5yr_avg[12];
	char    payout_ratio[12];
	char    div_date[12];
	char    xdiv_date[12];
	char    last_split_factor[12];
	char    last_split_date[12];

	/* Financial Highlights */
	char    profit_margin[12];
	char    operating_margin[12];
	char    return_on_assets[12];
	char    return_on_equity[12];
	char    revenue[12];
	char    revenue_per_share[12];
	char    quarterly_revenue_growth[12];
	char    gross_profit[12];              /* Gross Profit */
	char    EBITDA[12];                    /* EBITDA */
	char    net_income_avi[12];            /* Net Income avi to Common */
	char    diluted_eps[12];
	char    quarterly_earnings_growth[12];
	char    total_cash[12];
	char    total_cash_per_share[12];
	char    total_debt[12];
	char    total_debt_equity[12];
	char    current_ratio[12];
	char    book_value_per_share[12];
	char    operating_cash_flow[12];
	char    levered_free_cash_flow[12];
};

struct fund_int {
	double   market_cap;
	double   enterprise_value;
	double   trailing_pe;
	double   forward_pe;
	double   peg_ratio;
	double   price_sales;
	double   PBR;
	double   enterprise_revenue;
	double   enterprise_EBITDA;
	double   avg_volume_3mo;
	double   avg_volume_10d;
	double   shares_outstanding;
	double   float_value;
	double   insiders;
	double   institutions;
	double   shares_short;
	double   short_ratio;
	double   short_pc_float;
	double   short_pc_outstanding;
	double   shares_short_prior;
	double   forward_annual_div_rate;
	double   forward_annual_div_yiel;
	double   trailing_annual_div_rate;
	double   trailing_annual_div_yield;
	double   div_yield_5yr_avg;
	double   payout_ratio;
	uint64_t div_date;           // 26
	uint64_t xdiv_date;          // 27
	uint64_t last_split_factor;  // 28
	uint64_t last_split_date;    // 29
	double   profit_margin;
	double   operating_margin;
	double   return_on_assets;
	double   return_on_equity;
	double   revenue;
	double   revenue_per_share;
	double   quarterly_revenue_growth;
	double   gross_profit;
	double   EBITDA;
	double   net_income_avi;
	double   diluted_eps;
	double   quarterly_earnings_growth;
	double   total_cash;
	double   total_cash_per_share;
	double   total_debt;
	double   total_debt_equity;
	double   current_ratio;
	double   book_value_per_share;
	double   operating_cash_flow;
	double   levered_free_cash_flow;
	time_t   timestamp;
	char     reserved[128];
};

struct stock {
	char            *sym;
	struct price    *price;
	struct mag      *mag;
	struct mag2     *mag2;
	struct mag3     *mag3;
	struct mag4     *mag4;
	struct XLS      *XLS;
	struct market   *market;
	struct sig     **signals;
	struct API       API;
	int              type;
	int              subtype;
	int              country_id;
	double           sig_avgdays;
	uint64_t         indicators;
	unsigned short   nr_signals;
	unsigned short   max_signals;
	unsigned short   nr_bullish_indicators;
	unsigned short   nr_bearish_indicators;
	unsigned short   nr_mag2_entries;
	unsigned short   nr_mag3_entries;
	double           peak_1year;
	double           peak_1year_pc;
	char            *name;
	char            *country;
	char            *exchange_str;
	char            *sector;
	char            *industry;
	char             options_url[256];
	struct tick      tickbuf;
	struct tick      tickbuf2;
	struct tick     *current_tick;
	struct tick     *sparetick;
	char            *scatter_table;
	char            *ctable;
	int              scatter_size;
	int              ctable_size;
	/* Candles */
	int              candle_json_size;
	int              candle_json_max;
	char            *candle_json;
	char            *bull_flags;
	char            *bear_flags;
	int              bull_flags_len;
	int              bear_flags_len;
	struct candle  **candles;
	struct CANDLE   *bull_candles[5];
	struct CANDLE   *bear_candles[5];
	struct CANDLE   *last_candle;
	/* Live Prices */
	struct ohlc     *ohlc;
	unsigned short   nr_updates;
	unsigned short   nr_ohlc;
	time_t           current_timestamp;
	double           current_open;
	double           current_high;
	double           current_low;
	double           current_close;
	/* Options */
	struct opchain  *options;
	char            *option_price_1d;
	int              option_price_1d_len;
	int              nr_call_options;
	int              nr_put_options;
	int              options_url_size;
	/* ESP */
	int              nr_qslide;
	int              a1esp;
	int              a4esp;
	/* Trend */
	int              nr_days_up;
	int              nr_days_down;
	int              nr_weeks_up;
	int              nr_weeks_down;
	/* UFOS */
	double           price_15m;
	double           price_5m;
	double           price_1min;
	double           volume_15m;
	double           volume_5m;
	double           volume_1m;
	uint64_t         vol_15m;
	uint64_t         vol_5m;
	uint64_t         vol_1m;
	double           green_days;
	double           pc_days_up;
	double           pc_days_down;
	double           day1;
	double           day3;
	double           day5;
	double           day8;
	double           day13;
	double           day21;
	double           day42;
	double           day63;
	struct earnings  earnings;
	struct earnings *EARNINGS;
	/* Ranks */
	unsigned short   ranks_2019[12];
	unsigned short   ranks_2020[12];
	unsigned short   ranks_2021[12];
	unsigned short   ranks_2022[12];
	unsigned short   ranks_2023[12];
	unsigned short   ranks_2024[12];
	char             rankstr[32];
	uint16_t         rank;
	uint16_t         prev_rank;
	uint8_t          sector_index;
	uint8_t          dead;
	uint16_t         nr_ranks;
	uint16_t         exchange;
	uint16_t         id;
	uint16_t         thread_id;
	uint16_t         airfork_stock_class;
	uint16_t         nr_data_retries;
	/* Price */
	double           prior_close;
	double           price_open;
	double           intraday_low;
	double           intraday_high;
	double           open_pc;
	double           high_pc;
	double           low_pc;
	double           pr_percent;
	double           current_price;
	uint64_t         current_volume;
	uint64_t         day_volume;
	uint64_t         premarket_volume;
	time_t           volume_timestamp;
	time_t           volume_last_save;
	unsigned short   nr_fundamentals;
	unsigned short   update;
	struct fund_int *fund_int;
	struct fund_str  fund_str;
};

struct thash {
	struct stock   *stock;
	char            ticker[8];
	UT_hash_handle  hh;
};

struct XLS {
	struct thash   *hashtable;
	struct thread  *stock_threads;
	struct stock   *STOCKS_ARRAY;
	struct stock  **STOCKS_PTR;
	char          **STOCKS_STR;
	struct stock  **LOWCAPS;
	struct stock  **HIGHCAPS;
	struct stock  **FOREX;
	struct stock  **CRYPTO;
	struct stock   *GSPC;
	struct stock  **ranked_stocks;
	struct board  **boards;
	struct server  *config;
	char           *tickers;
	void           *MONSTER;
	struct forkmem *forks;
	double          sigrank_avgdays;
	int             nr_stocks;
	int             nr_crypto;
	int             nr_forex;
	int             nr_stock_threads;
	int             nr_highcaps;
	int             nr_lowcaps;
	int             nr_forks;
	int             nr_ranked_stocks;
	int             max_ranks;
	unsigned char   data_source;
	unsigned char   network_protocol;
};

struct tradetime {
	char time_4AM;
	char time_5AM;
	char time_6AM;
	char time_7AM;
	char time_8AM;
	char time_9AM;
};

/* stocks.c */
__MODULE_INIT    init_stocks_module       (struct server  *server);
__MODULE_HOOK    stocks_session_init      (struct session *session);
__MODULE_HOOK    stocks_main_pre_loop     (struct server  *server);
__MODULE_HOOK    stocks_main_post_loop    (struct server  *server);
struct stock     *search_stocks           (char *name);
struct stock     *search_stocks_XLS       (struct XLS *XLS, char *ticker);
struct XLS       *load_stocks             (void);
void              apc_update_EOD          (struct connection *connection, char **argv);
void              stock_loop              (struct server *config);
void              verify_stocks           (struct XLS *XLS);
void              cmd_delisted            (void);
void              load_fundamentals       (struct stock *stock);
void             *stock_thread            (void *args);
int               stocks_get_indexes      (char *ibuf);
void              http_stock_api          (struct connection *connection, char *ticker, char *api, char **opt);
int               is_index                (char *ticker);
int               is_fund                 (char *ticker);
void              set_index               (struct stock *stock);
unsigned short    stock_id                (char *ticker);
char             *json_board              (struct board *board, int *json_len);
void              generate_monster_db     (struct XLS *XLS);
void              update_quicklook        (struct XLS *XLS);
void              update_peakwatch        (struct XLS *XLS);
void              init_monster            (struct XLS *XLS, int do_forks);
void              store_monster_db        (struct monster *monster);
void              http_send_monster       (struct connection *connection);
void             *stocks_update_checkpoint(void *args);

/* market.c  */
bool              ticker_needs_update     (struct stock *stock, time_t *start_timestamp, time_t *end_timestamp);
int               market_update           (void);
int               market_get_nr_minutes   (struct stock *stock);
int               market_get_nr_hours     (struct stock *stock);
void              market_set_EOD          (time_t current_utc_time, int country_id, char *prev_day, char *curr_day, char *next_day);
time_t            market_prev_eod_unix    (struct stock *stock);
char             *market_prev_eod_str     (struct stock *stock);
struct market    *search_market           (struct stock *stock);

/* price.c */
void              init_ip                 (void);
void              update_current_price    (struct stock *stock);
void              addPoint                (struct stock *stock, time_t new_tick, double new_open, double new_high, double new_low, double new_close, uint64_t new_volume);

/* wsj.c */
void              WSJ_update_current_price(struct stock *stock);
void              apc_update_WSJ          (struct connection *connection, char **argv);
void              wsj_update_stock        (char *ticker);
void              wsj_get_EOD             (struct stock *stock, char *page);
int               wsj_query               (struct stock *stock, char *url, int url_size, char *page, void (*query)(struct stock *stock, char *page));
int               WSJ_update_allday_price (struct stock *stock);
double            price_by_date           (struct stock *stock, char *date);
void              argv_wsj_allday         (char *ticker);
void              load_WSJ                (struct XLS *XLS);

/* volume.c */
char             *volume                  (uint64_t vol, char *buf);

/* ufo.c */
void ufo_scan             (struct XLS *XLS);
void init_ufo             (struct XLS *XLS);
void sort_double_gainers  (struct board *board);
int  insert_double_gainer (struct stock *stock, struct board *board);
void add_delta_gainer     (struct stock *stock, struct board *board);
void add_delta_loser      (struct stock *stock, struct board *board);
void add_double_loser     (struct stock *stock, struct board *board);
void add_double_gainer    (struct stock *stock, struct board *board);
int  stock_present        (struct stock **bstocks, struct stock *stock, int max);
int  highcap_tables       (struct session *session, char *packet, struct workspace *workspace);
int  lowcap_tables        (struct session *session, char *packet, struct workspace *workspace);
int  peakwatch_tables     (struct session *session, char *packet, struct workspace *workspace);
int  ufo_tables           (struct session *session, char *packet, struct workspace *workspace);
int  ufo_load_charts      (struct session *session, char *packet, struct workspace *workspace, struct wsid *wsid, int interval, int table);
void ufo_reload_charts    (struct session *session, char *packet, char *id, int interval, int table);
int  ufo_search_table     (char *table, char *packet);
int  ufo_table_json       (char *table, char *packet);

/* apc_stocks.c */
void          apc_ebuild_stocks (struct connection *connection, char **argv);
void          apc_yahoo_fetch   (struct connection *connection, char **argv);
void          apc_ohlc          (struct connection *connection, char **argv);
void          apc_stock_debug   (struct connection *connection, char **argv);
void          apc_live          (struct connection *connection, char **argv);
void          apc_signals       (struct connection *connection, char **argv);
void          apc_print_stock   (struct connection *connection, char **argv);
void          apc_candle        (struct connection *connection, char **argv);

/* fund.c */
void update_fundb           (char *ticker, struct XLS *XLS);

/* candles.c */
void init_candles           (struct server *server);
void build_candle_screener  (struct XLS *XLS);
void build_candle_monster   (struct XLS *XLS);
void candle_scan            (struct stock *stock, struct mag *mag);
void marubozu_scan          (struct stock *stock, struct mag *mag, struct candle *candle, int start_entry, int nr_days, int *candles, int cdx);

/* forks.c */
void          apc_update_FORKS     (struct connection *connection, char **argv);
void          store_forkdb         (char *forkpath, struct forkmem *FORKMEM, int NR_FORKS);
int           load_forkdb          (struct XLS *XLS);
void          load_forks           (struct XLS *XLS);
void          init_forks           (struct XLS *XLS);
void          init_signals         (struct XLS *XLS);
void          build_flight_info    (struct monster *monster);

/* algo.c */
void   algorithm_mag1(struct stock *stock, struct mag *mag);
void   init_algo     (struct XLS *XLS, struct stock *stock);
void   init_ctable   (struct XLS *XLS);
void   init_anyday   (struct XLS *XLS);
void   init_BIX      (struct XLS *XLS);

/* mag.c */
void   load_mag2     (struct stock *stock);
void   load_mag3     (struct stock *stock);
void   load_mag4     (struct stock *stock);
void   build_mag     (char *ticker, struct XLS *XLS);
void   process_mag3  (struct XLS *XLS);

/* crypto.c */
void         *cryptocompare_thread (void *args);


static __inline__ double get_stock_price(struct stock *stock, int entry, int nr_entries)
{
	if (entry == 0 && stock->market->status != NO_MARKET)
		return stock->current_price;
	return stock->mag->close[nr_entries-entry];
}

static __inline__ double get_stock_delta(struct stock *stock, int entry, int nr_entries)
{
	double entry_close, prior_close;

	if (entry == 0 && stock->market->status != NO_MARKET)
		return stock->pr_percent;

	entry_close = stock->mag->close[nr_entries-entry];
	prior_close = stock->mag->close[nr_entries-entry-1];
	return delta(entry_close, prior_close);
}

static __inline__ double get_stock_open(struct stock *stock, int entry, int nr_entries)
{
	if (entry == 0 && stock->market->status != NO_MARKET)
		return stock->price_open;
	return stock->mag->open[nr_entries-entry];
}

static __inline__ double get_stock_high(struct stock *stock, int entry, int nr_entries)
{
	if (entry == 0 && stock->market->status != NO_MARKET)
		return stock->intraday_high;
	return stock->mag->high[nr_entries-entry];
}

static __inline__ double get_stock_low(struct stock *stock, int entry, int nr_entries)
{
	if (entry == 0 && stock->market->status != NO_MARKET)
		return stock->intraday_low;
	return stock->mag->low[nr_entries-entry];
}

static __inline__ double get_stock_prior_close(struct stock *stock, int entry, int nr_entries)
{
	return stock->mag->close[nr_entries-entry-1];
}

static __inline__ double get_stock_openpc(struct stock *stock, int entry, int nr_entries)
{
	double prior_close;

	if (entry == 0 && stock->market->status != NO_MARKET)
		return stock->open_pc;
	prior_close = stock->mag->close[nr_entries-entry-1];
	return (((stock->mag->open[nr_entries-entry]/prior_close)-1)*100.0);
}

static __inline__ double get_stock_highpc(struct stock *stock, int entry, int nr_entries)
{
	double prior_close;

	if (entry == 0 && market != NO_MARKET)
		return stock->high_pc;

	prior_close = stock->mag->close[nr_entries-entry-1];
	return (((stock->mag->high[nr_entries-entry]/prior_close)-1)*100.0);
}

static __inline__ double get_stock_lowpc(struct stock *stock, int entry, int nr_entries)
{
	double prior_close;

	if (entry == 0 && market != NO_MARKET)
		return stock->low_pc;

	prior_close = stock->mag->close[nr_entries-entry-1];
	return (((stock->mag->low[nr_entries-entry]/prior_close)-1)*100.0);
}

static __inline__ char *get_stock_volume(struct stock *stock, int entry, int nr_entries, char *vstr)
{
	if (entry == 0 && market != NO_MARKET)
		return volume(stock->day_volume, vstr);
	return volume(stock->mag->volume[nr_entries-entry], vstr);
}

#endif
