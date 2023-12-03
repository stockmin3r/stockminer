#ifndef __EXTERN_H
#define __EXTERN_H

#include <conf.h>

extern struct ssl_server *WWW_SERVER;
extern struct server      Server;
extern struct object     *Objects;
extern struct XLS        *CURRENT_XLS;      // this should be called MARKET or MARKETS since more world markets will be added
extern struct ihash      *UID_HASHTABLE;
extern struct chash      *COOKIE_HASHTABLE;
extern struct www        *WWW_HASHTABLE;
extern struct connection  apc_connection;
extern double             stockdata_completion;
extern int                stockdata_checkpoint;
extern int                NR_USERS;
extern int                stockdata_pending;
extern bool               STOCKDATA_PENDING;

extern const char *DB_PATH;
extern const char *DB_USERS_PATH;
extern const char *DB_REPO_PATH;
extern const char *STOCKDB_PATH;
extern const char *STOCKDB_MAG2_PATH;
extern const char *STOCKDB_MAG3_PATH;
extern const char *STOCKDB_MAG4_PATH;
extern const char *STOCKDB_CSV_PATH;
extern const char *STOCKS_PATH;
extern const char *STOCKS_DAYS_PATH;
extern const char *STOCKS_WEEKS_PATH;
extern const char *GSPC_PATH;
extern const char *OPTIONS_PATH;

extern char *test_scripts;
extern int   test_scripts_len;

extern int   current_week;
extern int  *month_days;

extern int  NR_RANKS;

#endif
