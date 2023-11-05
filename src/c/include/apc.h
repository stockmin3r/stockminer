#ifndef __APC_H
#define __APC_H

#define MAX_APC_COMMANDS 1024
#define MAX_APC_ARGV       64

struct connection;

typedef struct apc  apc_t;        /* APC Command */
typedef void      (*apc_handler)(struct connection *connection, char **argv);

/*
 * Admin Procedure Calls (APCs)
 */
struct apc {
		int         type;        /* APC || LPC */
        char       *name;        /* name of APC command to type in the terminal shell */
		apc_handler handler;     /* apc function handler (used by both server and client) */
};

enum {
	APC, /* Admin Procedure Call (Remote)         */
	LPC  /* Local Procedure Call (local function) */
};

enum {
	APC_MODE_DEFAULT,
	APC_MODE_OPTIONS,
	APC_MODE_WEBSCRIPT,
	APC_MODE_WWW
};

enum {
	LPC_HELP,
	LPC_USERS,
	LPC_EXIT,
};

#endif
