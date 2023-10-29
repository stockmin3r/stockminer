#ifndef __OPTIONS_H
#define __OPTIONS_H

#include <conf.h>

#define RANK_OPEN_INTEREST    1
#define RANK_LAST_TRADED      2
#define RANK_LAST_TRADED_LOW  3
#define RANK_LAST_TRADED_HIGH 4

#define OPCHAIN_LIVE          1
#define OPCHAIN_OVERWRITE     2
#define OPCHAIN_UPDATE        3
#define OPCHAIN_CREATE        4

#define OPTION_VOLUME         2
#define OPTION_OHLC           3
#define YAHOO_CONTRACT        1
#define OPSHOW_CORRUPT        1

#define OPTION_1D             1
#define OPTION_1M             2
#define OPTION_LIVE           4
#define OPTION_PRINT          8

#define OPTION_EXPIRED       -1
#define OPTION_CALL          'C'
#define OPTION_PUT           'P'

#define MAX_OPCHAINS        256
#define MAX_CONTRACTS       256

#define YEAR_2021_TIMESTAMP 1609473600UL

struct server;
struct XLS;

extern int nr_lowcap_options;
extern int nr_highcap_options;

extern struct opstock *options_trade_board[1024];
extern struct opstock *options_oi_calls[35];
extern struct opstock *options_vol_calls[35];
extern struct opstock *options_oi_puts[35];
extern struct opstock *options_vol_puts[35];
extern struct opstock *options_trade_board[1024];
extern int nr_options_oi_calls;
extern int nr_options_vol_calls;
extern int nr_options_oi_puts;
extern int nr_options_vol_puts;
extern int nr_lastTraded;
extern int nr_lowInterest;
extern int nr_global_calls;
extern int nr_global_puts;

extern char option_oi_json[8192];
extern char option_oi_json2[8192];
extern char option_vol_json[8192];
extern char option_vol_json2[8192];
extern int option_oi_json_len;
extern int option_oi_json_len2;
extern int option_vol_json_len;
extern int option_vol_json_len2;
extern int option_oi_buffer;
extern int option_vol_buffer;

extern struct oprank **RANKED_OPCHAINS;
extern struct oprank **RANKED_CONTRACTS;
extern struct opchain **ranked_opchains;
extern struct opstock **ranked_contracts;
extern int nr_ranked_opchains;
extern int nr_opchain_stocks;
extern int nr_ranked_contracts;
extern int nr_contract_stocks;

struct opmath {
	double    NetDebit;
	double    PctNetDebit;
	double    ReturnCalled;
	double    APRCalled;
	double    ReturnUnchanged;
	double    APRUnchanged;
	double    PctOTM;
};

struct update {
	struct opchain *opchain;
	struct Option  *option;
	char           *ticker;
	char           *page;
	char           *expiry_path;
	time_t         *saved_expiry;
	int             nr_saved_expiry;
	int             expfd;
	time_t          expiry;
	int             cmd;
	/* get_contract_ohlc() */
	double          open[256];
	double          high[256];
	double          low[256];
	double          close[256];
	time_t          timestamps[256];
	int             volume[256];
	int             nmap[256];
	int             overwrite;
	int             nr_points;
	time_t          timefrom;
	time_t          timeto;
};

/* In-Memory representation of Option Chain */
struct opchain {
	struct Option  **call_options;
	struct Option  **put_options;
	struct opstock **call_opstocks;
	struct opstock **put_opstocks;
	struct opmath   *opmath;
	struct opstock  *ranked_calls[8];
	struct opstock  *ranked_puts[8];
	time_t           expiry;
	char            *ticker;
	char           **cnames;
	char           **pnames;
	char           **csv_calls;
	char           **csv_puts;
	int             *csv_calls_len;
	int             *csv_puts_len;
	int             *csv_call_points;
	int             *csv_put_points;
	double           csv_total_points;  /* percentage average of total OHLC points per opchain */
	int              openInterest_avg;
	int              volume_avg;
	unsigned short   nr_calls;
	unsigned short   nr_puts;
	char             expiry_date[12];
	time_t           last_update;
	unsigned short   nr_days;
	unsigned short   nr_expiry;
	unsigned short   nr_failed;
	unsigned char    expired;
	unsigned char    corrupt;
	unsigned char    nr_ranked_calls;
	unsigned char    nr_ranked_puts;
};

struct opstock {
	struct Option  *option;
	struct stock   *stock;
	struct opmath  *opmath;
	/* load_options()    */
	char           *name;
	char           *csv;
	int             csv_len;
	int             csv_nr_points;
	/* search_contract() */
	struct opchain *opchain;
	int             expiry;
	int             nr_op;
	int             optype;
	/* opchain_thread()  */
	int             nr_failed;
	/* update_contract_live() */
	struct ohlc    *OHLC;
	int             nr_ohlc;
};

struct Option2 {
	char          contract[24];
	double        strike;
	double        lastPrice;
	double        change;
	double        percentChange;
	int           volume;
	int           openInterest;
	double        bid;
	double        ask;
	time_t        expiry;
	time_t        lastTrade;
	double        impliedVolatility;
	time_t        timestamp;
}__attribute__((packed));

struct Option {
	char          contract[24];
	double        strike;
	double        lastPrice;
	double        change;
	double        percentChange;
	int           volume;
	int           openInterest;
	double        bid;
	double        ask;
	time_t        expiry;
	time_t        lastTrade;
	double        impliedVolatility;
	time_t        timestamp;
	double        delta;
	double        theta;
	double        rho;
	double        vega;
	double        gamma;
}__attribute__((packed));

struct optick {
	struct stock     *stock;
	char              contract[32];
};

struct opcond {
	struct stock     *stock;
	char              ticker[8];
	char              exec[MAX_EXEC_LEN+1];
	double            price;
	time_t            volume;
	uint64_t          id;
	unsigned short    condition;
	unsigned short    condclass;
	unsigned short    cmd;
	unsigned short    exec_len;
	unsigned char     periods[6];
};

struct opwatch {
	char              name[WATCHLIST_NAME_MAX+1];
	char              basetable[WATCHLIST_NAME_MAX+1];
	struct optick     stocks[MAX_STOCKS];
	struct opcond     conditions[128];
	struct list_head  publist;
	struct wtab      *wtab;
	struct watchlist *origin;
	unsigned char     wtab_id;
	unsigned char     nr_conditions;
	unsigned char     nr_stocks;
	unsigned char     config;
};

struct opconf {
	unsigned short    nr_calls;
	unsigned short    nr_puts;
} __attribute__((packed));

struct oprank {
	struct opchain **opchains;
	struct opstock **contracts;
	int              nr_opchains;
	int              nr_contracts;
};

/* option.c */
void            option_submit_thread (int sockfd);
void            init_options         (struct server *server);
struct opstock *search_contract      (struct opchain *opchain, char *contract);

/* opdb.h */
struct opchain *search_opchain(struct opchain *opchain, char *expiry);
void   opchains_fix        (char *ticker);
int    opchain_exists      (char *ticker, char *expiry);
int    extract_options     (struct update *update);
void   option_load_csv     (char *ticker, char *contract, struct opchain *opchain, int opindex, char optype);
void   load_all_options    (void);
int    load_opchains       (struct stock *stock);
void   load_option_stocks  (void);
void   show_corrupt        (char   *ticker);
void   opchains_fixoi      (char   *ticker, char   *expiry_date, struct server *server);
time_t get_last_update     (char   *ticker, char   *expiry_date);
void   update_1d           (char   *ticker, char   *contract);
void   add_broken_opchain  (char   *ticker, time_t  timestamp);
int    load_expiry         (char   *ticker, time_t *expiry,    int    *nr_expiry);
int    load_expiry_file    (char   *ticker, int    *expfd,     time_t *saved_expiry);
int    expiry_present      (time_t *expiry, int     nr_expiry, time_t timestamp);

/* opcsv.h */
int csv_load               (char *csvbuf, struct ohlc *OHLC);
int csv_load_all           (char *csvbuf, struct ohlc **ohlc);
int csv_exists             (char *ticker, char *contract);
int opstock_csv_load       (struct opstock *opstock, char *path);

/* opnet.h */
int  update_opchain        (struct update  *update);
void update_contract_1d    (struct opstock *opstock);
int  get_contract_ohlc     (char *ticker, char *contract, struct update *update);
void get_opchain_history   (char *offset, char *contract);
void stock_update_options  (char *ticker, char *exp_year, int update);
void stock_update          (char *ticker, char *exp_year, struct server *server);
int  get_option_history    (char *ticker, char *contract, int overwrite, int opt);
int  fetch_options         (char *ticker, time_t date, char *page);
int  options_get_expiry    (char *page,   time_t *expired, int *nr_expired);
void refetch_opchain       (char *ticker, time_t timestamp);
void update_options        (char *offset, int stop, int update);
void update_lowcap_options (char *offset, int stop, int update);
int  get_nr_options        (char *page,   int *nr_call_options, int *nr_put_options, char **puts);

/* opshow.h */
void opchain_query   (char *ticker, char *expyear, int cmd);
void show_options    (char *ticker, char *expiry_year);
void stock_query     (char *ticker, int cmd);
void show_days       (char *ticker, char *exp_year);
void show_expiry     (char *ticker, int cmd);
void backup_stocks   (char *ticker);
void backup_restore  (char *ticker, char *expiry_year, char *backup_day);
int  opchain_list    (char *ticker, char **opchains);
void show_csv_points (char *ticker, char *contract);
void show_stats      (char *ticker, char *exp_year);
void show_missing    (char *ticker);
void show_chainstamp (char *ticker, struct server *server);
void show_contract_1d(char *contract, int range);
void option_query    (char *contract);
void verify_expiry   (char *contract);
void db_query        (int cmd);

/* oprank */
int  RankOption            (char *ticker, int rank_type, void *rank, struct Option **option);
void RankLastTradedLow     (char *ticker);
void RankLastTradedHigh    (char *ticker);
void RankOpenInterest      (char *ticker);
void sort_options_vol_calls(struct opstock *opstock);
void sort_options_oi_calls (struct opstock *opstock);
void sort_options_vol_puts (struct opstock *opstock);
void sort_options_oi_puts  (struct opstock *opstock);
void sort_lastTraded       (struct opstock *opstock, int rank);
void oprank_add_opchain    (struct opchain *opchain);
void oprank_add_contract   (struct opstock *opstock);

/* opcboe.c */
void op_query              (char *ticker);
void op_query_ticker       (char *ticker);
void op_query_opchain      (char *ticker, char *opchain);

/* opdb.c && options.c */
char *json_option_oi_calls_leaders (int *json_len);
char *json_option_vol_calls_leaders(int *json_len);
void  update_json_options_oi_calls ();
void  update_json_options_vol_calls();
char *json_option_oi_puts_leaders  (int *json_len);
char *json_option_vol_puts_leaders (int *json_len);
void  update_json_options_oi_puts  ();
void  update_json_options_vol_puts ();


#endif
