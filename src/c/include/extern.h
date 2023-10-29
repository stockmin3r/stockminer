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
extern int                NR_USERS;

extern char *test_scripts;
extern int   test_scripts_len;

extern int   current_week;
extern int  *month_days;

extern int  NR_RANKS;

#endif
