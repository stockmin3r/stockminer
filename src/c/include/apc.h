#ifndef __APC_H
#define __APC_H

#define APC_ARG_INT 1
#define APC_ARG_STR 2
#define APC_ARG_OBJ 4

#define APC_MASK 0x3FFF
#define APC_SET_ARG_OBJ(apc) (apc|(1<<15))
#define APC_SET_ARG_STR(apc) (apc|(1<<14))
#define APC_SET_ARG_INT(apc) (apc|(1<<13))
#define APC(apc) (apc & APC_MASK)
#define APC_GET_ARG(apc) (apc >> 13)

#define MAX_APC_STRLEN 8192

/*
 * Admin Procedure Calls (APCs)
 */
struct apc {
        apc_t   apc;
        char   *name;
        int     argtype;
};


enum {
        APC_SEND_STR,
        APC_SEND_INT,
        APC_SEND_VOID,
        LPC_HELP,
	LPC_USERS,
        LPC_EXIT
};

enum {
        APC_MODE_DEFAULT,
        APC_MODE_OPTIONS,
        APC_MODE_SCRIPTING,
        APC_MODE_WWW
};

enum {
	APC_UPDATE_EOD,
	APC_UPDATE_WSJ,
	APC_UPDATE_EARNINGS,
	APC_UPDATE_FORKS,
	APC_PRINT_STOCKS,
	APC_PRINT_STOCK,
	APC_MAG2,
	APC_SESSION_LIST,
	APC_USER_ADD,
	APC_RESTART,
	APC_OHLC,
	APC_OHLC_DATA,
	APC_EARNINGS,
	APC_SIGNALS,
	APC_LIVE,
	APC_ESP,
	APC_LIST_WATCHLISTS,
	APC_WATCHLIST,
	APC_ALERTS,
	APC_OPTIONS,
	APC_CRYPTO,
	APC_RELOAD_WEBSITE = 21,
	APC_DEBUG_STOCK,
	APC_EBUILD_STOCKS,
	APC_TRACK,
	APC_CANDLE,
	APC_GETPID,
	APC_NETSH,
	APC_EBUILD_STOCK,
	APC_JSON_CLEAR,
	APC_THREADSTAT,
	APC_WWW,
	APC_OBJ_LIST
};

#endif
