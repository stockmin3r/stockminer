#ifndef __WATCHLIST_H
#define __WATCHLIST_H

#include <stdinc.h>
#include <list.h>

#define WATCHLIST_NAME_MAX     63
#define BASETABLE_NAME_MAX     15
#define MAX_EXEC_LEN           71
#define MAX_WATCHLISTS         64
#define MAX_STOCKS             96
#define MAX_CONDITIONS        128
#define MAX_COLUMNS            32
#define MAX_WTAB_SIZE        2048
#define MAX_WTAB_NAME          47
#define MAX_WTAB_COLUMNS       64
#define MAX_WTAB_PRESETS       16
#define MAX_BULK_ADD          256
#define MAX_TABLE_CSS_STYLES   32
#define MAX_CSS_SIZE         8192
#define IS_PUBLIC               1
#define IS_MORPHTAB             2
#define WATCHTABLE_ROOT         1
#define ADD_STOCK               0
#define REMOVE_STOCK            1

/* Condition */
#define CONDITION_PRICE     0
#define CONDITION_VOLUME    1
#define CONDITION_REACHED   2

/* ConditionArgType */
#define CONDARG_PT          0
#define CONDARG_VOL_TARGET  1
#define CONDARG_VOL_AVG     2
#define CONDARG_VOL_RVOL    3

#define PRICE_GREATER_THAN  1
#define PRICE_LESSER_THAN   2
#define VOLUME_GREATER_THAN 3
#define VOLUME_LESSER_THAN  4

#define TABLE_TYPE_WATCHLIST  '0'
#define TABLE_TYPE_WATCHTABLE '1'

struct watchstock {
	struct stock     *stock;
	char              ticker[8];
};

struct watchcond {
	struct stock     *stock;
	char              ticker[8];
	char              exec[MAX_EXEC_LEN+1];
	double            price;
	uint64_t          volume;
	uint64_t          id;
	unsigned short    condition;
	unsigned short    condclass;
	unsigned short    cmd;
	unsigned short    exec_len;
	unsigned char     periods[6];
};

struct watchlist {
	char              name[WATCHLIST_NAME_MAX+1];
	char              basetable[BASETABLE_NAME_MAX+1];
	struct watchstock stocks[MAX_STOCKS];
	struct watchcond  conditions[MAX_CONDITIONS];
	struct list_head  publist;
	struct wtab      *wtab;
	struct watchlist *origin;
	struct session   *owner;
	uint64_t          watchlist_id;
	uint64_t          config;
	unsigned char     nr_conditions;
	unsigned char     nr_stocks;
};

//{watchlist:"Highcaps",basetable:"morphtab",stocks:["AAXN","TSLA","ENPH"],conditions:[{ticker:"AAXN",exec:"^^^^",price:22.23,volume:0,id:"34234232",condition:0,condclass:0,cmd:1,periods:4}]}
/*
struct watchtable {
	char              dict[MAX_WTAB_SIZE];
	char              name[64];
	struct watchlist *watchlist;
	unsigned short    colmap[MAX_WTAB_COLUMNS];
	unsigned int      table_type;
	unsigned short    col_type;
	unsigned char     nr_columns;
	unsigned char     id;
};*/

/* Price */
#define COL_SYMBOL      900
#define COL_PRICE       901
#define COL_RANK        902
#define COL_DELTA       903
#define COL_VOLUME      904
#define COL_OPEN        905
#define COL_HIGH        906
#define COL_LOW         907
#define COL_PC          908 /* Prior Close */
#define COL_OP          909 /* Percent up since Open (live) */
#define COL_HP          910
#define COL_LP          911
#define COL_PK          912
#define COL_PKP         913
#define COL_VS          914
#define COL_DATE        915
#define COL_PT          916

/* Trend */
#define COL_NDUP     400
#define COL_NDDW     401
#define COL_NWUP     402
#define COL_NWDW     403
#define COL_PNDUP    404
#define COL_PNDDW    405
#define COL_BIX      406
#define COL_5AD21Q1  418
#define COL_5AD21Q2  419
#define COL_5AD20Q1  420
#define COL_5AD20Q2  421
#define COL_5AD19Q1  422
#define COL_5AD19Q2  423
#define COL_10AD21Q1 424
#define COL_10AD21Q2 425
#define COL_10AD20Q1 426
#define COL_10AD20Q2 427
#define COL_10AD19Q1 428
#define COL_10AD19Q2 429
#define COL_5AD21    430
#define COL_5AD20    431
#define COL_5AD19    432
#define COL_10AD21   433
#define COL_10AD20   434
#define COL_10AD19   435

/* Indicators */
#define COL_MACD       700
#define COL_RSI        701
#define COL_AROON      702
#define COL_STOCHASTIC 703
#define COL_BB         704
#define COL_CCI        705
#define COL_KCH        706

/* Technical */
enum {
	COL_MAVG200 = 100,
	COL_MAVG100,
	COL_MAVG50,
	COL_MAVG20,
	COL_VWAP,
	COL_VOL5,
	COL_VOL10,
	COL_VOL21,
	COL_VOL3M,
	COL_VOL_ADI,
	COL_VOL_OBV,
	COL_VOL_CMF,
	COL_VOL_FI,
	COL_VOL_MFI,
	COL_VOL_EM,
	COL_VOL_VPT,
	COL_VOL_NVI,
	COL_VOL_VWAP,
	COL_VOL_ATR,
	COL_VOL_BBM,
	COL_VOL_BBH,
	COL_VOL_BBL,
	COL_VOL_BBW,
	COL_VOL_BBP,
	COL_VOL_BBHI,
	COL_VOL_BBLI,
	COL_VOL_KCC,
	COL_VOL_KCL,
	COL_VOL_KCH,
	COL_VOL_KCW,
	COL_VOL_KCP,
	COL_VOL_KCHI,
	COL_VOL_KCLI,
	COL_VOL_DCL,
	COL_VOL_DCH,
	COL_VOL_DCM,
	COL_VOL_DCW,
	COL_VOL_DCP,
	COL_VOL_UI,
	COL_TR_MACD,
	COL_TR_MACD_SIG,
	COL_TR_MACD_DIFF,
	COL_TR_SMA_FAST,
	COL_TR_SMA_SLOW,
	COL_TR_EMA_SLOW,
	COL_TR_ADX,
	COL_TR_ADX_POS,
	COL_TR_ADX_NEG,
	COL_TR_VTX_POS,
	COL_TR_VTX_NEG,
	COL_TR_VTX_DIFF,
	COL_TR_TRIX,
	COL_TR_MASS_IDX,
	COL_TR_CCI,
	COL_TR_DPO,
	COL_TR_KST,
	COL_TR_KST_SIG,
	COL_TR_KST_DIFF,
	COL_TR_ICH_CONV,
	COL_TR_ICH_BASE,
	COL_TR_ICH_A,
	COL_TR_ICH_B,
	COL_TR_VICH_A,
	COL_TR_VICH_B,
	COL_TR_AROON_UP,
	COL_TR_AROON_DOWN,
	COL_TR_AROON_IND,
	COL_TR_PSAR_UP,
	COL_TR_PSAR_DOWN,
	COL_TR_PSAR_UP_IND,
	COL_TR_PSAR_DOWN_IND,
	COL_TR_STC,
	COL_TR_MOM_RSI,
	COL_TR_MOM_STOCH_RSI,
	COL_TR_MOM_STOCH_RSI_K,
	COL_TR_MOM_STOCH_RSI_D,
	COL_TR_MOM_TSI,
	COL_TR_MOM_UO,
	COL_TR_MOM_STOCH_K,
	COL_TR_MOM_STOCH_D,
	COL_TR_MOM_WR,
	COL_TR_MOM_AO,
	COL_TR_MOM_KAMA,
	COL_TR_MOM_ROC,
	COL_TR_MOM_PPO,
	COL_TR_MOM_PPO_SIG,
	COL_TR_MOM_PPO_HIST,
	COL_TR_MOM_OTHERS_DR,
	COL_TR_MOM_OTHERS_DLR,
	COL_TR_MOM_OTHERS_CR,
	COL_EMA10,
	COL_EMA20,
	COL_EMA50,
	COL_EMA100,
	COL_EMA200,
	COL_MFI2
};

/* ESP Algorithm */
enum {
	COL_DAYS_5PC = 200,
	COL_DAYS_10PC,
	COL_DAYS_15PC,
	COL_DAYS_20PC,
	COL_RET_5PC,
	COL_RET_10PC,
	COL_RET_15PC,
	COL_RET_20PC,
	COL_MAX_5PC,
	COL_MAX_10PC,
	COL_MAX_15PC,
	COL_MAX_20PC,
	COL_A1_ESP,
	COL_A1,
	COL_A1SAME,
	COL_A1RESULT,
	COL_A4ESP,
	COL_A4,
	COL_1D,
	COL_3D,
	COL_5D,
	COL_8D,
	COL_13D,
	COL_21D,
	COL_42D,
	COL_63D,
	COL_RTD,
	COL_STREAK,
	COL_DIR,
	COL_BUY,
	COL_BUY_DELTA,
	COL_SELL,
	COL_SELL_DELTA,
	COL_FIB,
	COL_FIB_DIR,
	COL_BUY_FIB,
	COL_DELTA_FIB,
	COL_SELL_FIB,
	COL_SDT_FIB,   // SELL_DELTA_FIB
	COL_MEAN,
	COL_STD,
	COL_VAR90,
	COL_VAR95,
	COL_VAR99,
	COL_VAR99P,
	COL_1_YEAR_AGO,
	COL_1_YEAR_PKPC,
	COL_YTD,
	COL_SIG_21_A1,
	COL_SIG_42_A1,
	COL_SIG_63_A1,
	COL_MAXDAYS_5PC_21_A1,
	COL_MAXDAYS_5PC_42_A1,
	COL_MAXDAYS_5PC_63_A1,
	COL_SUCCESS_5PC_21_A1,
	COL_SUCCESS_5PC_42_A1,
	COL_SUCCESS_5PC_63_A1,
	COL_MAXDAYS_10PC_21_A1,
	COL_MAXDAYS_10PC_42_A1,
	COL_MAXDAYS_10PC_63_A1,
	COL_SUCCESS_10PC_21_A1,
	COL_SUCCESS_10PC_42_A1,
	COL_SUCCESS_10PC_63_A1,
	COL_SIG_21_A4,
	COL_SIG_42_A4,
	COL_SIG_63_A4,
	COL_MAXDAYS_5PC_21_A4,
	COL_MAXDAYS_5PC_42_A4,
	COL_MAXDAYS_5PC_63_A4,
	COL_SUCCESS_5PC_21_A4,
	COL_SUCCESS_5PC_42_A4,
	COL_SUCCESS_5PC_63_A4,
	COL_MAXDAYS_10PC_21_A4,
	COL_MAXDAYS_10PC_42_A4,
	COL_MAXDAYS_10PC_63_A4,
	COL_SUCCESS_10PC_21_A4,
	COL_SUCCESS_10PC_42_A4,
	COL_SUCCESS_10PC_63_A4,
	COL_1_YEAR_PKPR,
	COL_PLIMIT,
	COL_PDELTA,
	COL_ONE_YR_P10,
	COL_ONE_YR_P5,
	COL_LAST_PEAK,
	COL_P10ORP5,
	COL_PEAK2,
	COL_SIG
};


/* Fundamental */
#define COL_ED      300
#define COL_SEC     301
#define COL_DY      302
#define COL_DIV     303
#define COL_XD      304
#define COL_AR      305
#define COL_EPS     306
#define COL_VAR     307
#define COL_MCP     308
#define COL_PEG     309
#define COL_PBR     310
#define COL_PM      311
#define COL_ROA     312
#define COL_EDOFF   313

/* Options */
#define COL_CNAME   500
#define COL_OPRICE  501
#define COL_OBID    502
#define COL_OASK    503
#define COL_OI      504
#define COL_IV      505
#define COL_OVOL    506
#define COL_STRIKE  507
#define COL_HOLD    508
#define COL_ORC     509
#define COL_ORU     510
#define COL_APR     511
#define COL_OTM     512
#define COL_OND     513
#define COL_NDP     514
#define COL_OD      515

extern int               nr_global_watchlists;
extern struct watchlist *Watchlists[256];
extern mutex_t           watchlist_lock;

#endif
