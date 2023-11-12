#include <stdinc.h>
#include <conf.h>

void lpc_help    (struct connection *connection, char **argv);
void lpc_exit    (struct connection *connection, char **argv);
void lpc_users   (struct connection *connection, char **argv);
void lpc_options (struct connection *connection, char **argv);

static struct apc apc_commands[] = {
	{ APC, "sessions",    apc_sessions      }, // Show session list
	{ APC, "ohlc",        apc_ohlc          }, //
	{ APC, "update-eod",  apc_update_EOD    }, // Update all stock CSVs for EOD and rerun 1Day algorithms to produce binary *.mag files
	{ APC, "update-wsj",  apc_update_WSJ    }, //
	{ APC, "reload",      apc_reload_website}, // Instructs Server to reload src/website
	{ APC, "ebuild",      apc_ebuild_stocks }, // Instructs Server to rebuild earnings .e binary files from .earn txt files
	{ APC, "options",     lpc_options       }, // Terminal Procedure Call (alters the admin prompt for Stock Options specific commands)
	{ LPC, "help",        lpc_help          }, // Local Procedure Call: calls lpc_help()
	{ LPC, "users",       lpc_users         }, // List users from the database (Currently just a file)
	{ LPC, "exit",        lpc_exit          }  // exit();
};
static unsigned int NR_APC_COMMANDS = sizeof(apc_commands)/sizeof(struct apc);

static struct connection apc_connection;       // persistant client openssl connection to the server

/*
 * Admin Procedure Call (commands & results are now strings to keep it simple)
 *   ---packet format----
 * [ string cmdline ] x bytes (optional - not null terminated)
 */
void apc_send_result(struct connection *connection, char *result)
{
	openssl_write_sync(connection, result, strlen(result));
}

/*
 * Simple no argument APCs to send from main() via CLI
 */
void apc_connect_command(const char *command)
{
	if (!openssl_connect_sync(&apc_connection, LOCALHOST, ADMIN_PORT)) {
		printf(BOLDRED "[-] Can't connect to localhost:%d" RESET "\n", ADMIN_PORT);
		exit(-1);
	}
	openssl_write_sync(&apc_connection, (char *)command, strlen(command));
}

void apc_send_command(const char *command)
{
	openssl_write_sync(&apc_connection, (char *)command, strlen(command));
}

apc_t *search_apc(char *cmdline)
{
	apc_t *apc;
	int    len;

	for (int x=0; x<NR_APC_COMMANDS; x++) {
		apc = &apc_commands[x];
		len = strlen(apc->name);
		if (!strncmp(apc->name, cmdline, len))
			return (apc);
	}
	return (NULL);
}

/*
 * Client APC Handler 
 *   - called by term.c::terminal_loop()
 *   - Search for the APC struct that matches the command:
 *   - call local function handler for LPCs
 */
void apc_exec(char *command)
{
	apc_t *apc;
	char  *argv[MAX_APC_ARGV] = {0};
	char  *args;

	if (!(apc=search_apc(command))) {
		printf("\n[-] no such command\n");
		return;
	}

	/* Local Procedure Calls (LPC) */
	if (apc->type == LPC) {
		args = strchr(command, ' ');
		if (args)
			command = args + 1;
		cstring_split(command, argv, MAX_APC_ARGV, ' ');
		apc->handler(NULL, argv);
		return;
	}

	/* Auth Command is handled differently, see auth.c */
	if (!strcmp(apc->name, "auth")) {
		admin_client_auth(argv[0], argv[1]);
		return;
	}

	/* Remote APCs: openssl_write()'s command string */
	openssl_write_sync(&apc_connection, command, strlen(command));
}

/*
 * Client Terminal Loop
 */
void admin_client_loop(void)
{
	if (!openssl_connect_sync(&apc_connection, LOCALHOST, ADMIN_PORT)) {
		printf(BOLDRED "[-] Can't connect to localhost:%d" RESET "\n", ADMIN_PORT);
		exit(-1);
	}
	terminal_loop();
}

/* *******************
 *  APC SERVER LOOP  *
 ********************/
void *admin_server_loop(void *args)
{
	struct connection *connection = (struct connection *)args;
	packet_size_t      packet_size;
	char               packet[4096];
	char              *argv[MAX_APC_ARGV];
	apc_t             *apc;

	while (1) {
		memset(argv, 0, sizeof(argv));
		packet_size = openssl_read_sync2(connection, packet, sizeof(packet)-1);
		if (!packet_size || packet_size >= sizeof(packet)-1)
			goto out;

		if ((args=strchr(packet, ' ')))
			cstring_split(args+1, argv, MAX_APC_ARGV-1, ' ');

		/* Remote Procedure Calls (APC) */
		if (!(apc=search_apc(packet)))
			continue;
	
		if (apc->type == APC)
			apc->handler(connection, argv);
	}
out:
	return NULL;
}

void *netctl_thread(void *args)
{
	openssl_server_sync(admin_server_loop, LOCALHOST, ADMIN_PORT, 1);
	return NULL;
}

/*
 * Simple Local Procedure Calls
 */
void lpc_options(struct connection *connection, char **argv)
{
}

void lpc_users(struct connection *connection, char **argv)
{
	db_user_list();
}

void lpc_help(struct connection *connection, char **argv)
{
	printf(BOLDWHITE "alerts  [USERNAME]    - list alerts of a user" RESET "\n");
	printf(BOLDWHITE "watchdb [USERNAME]    - list watchlists of username" RESET "\n");
	printf(BOLDWHITE "watch   [Watchlist]   - list stocks from Watchlist " RESET "\n");
	printf(BOLDWHITE "ohlc    [TICKER]      - show OHLC 1m data for a stock\n");
	printf(BOLDWHITE "edays   [DAYS]        - show upcoming earnings #DAYS from today\n");
	printf(BOLDWHITE "sigs    [TICKER]      - show signals of stock\n");
	printf(BOLDWHITE "sessions              - list all current sessions" RESET "\n");
	printf(BOLDWHITE "users                 - list all USERS" RESET "\n");
	printf(BOLDWHITE "ebuild                - rebuild earnings\n");
	printf(BOLDWHITE "esync                 - fetch earnings from yahoo\n");
	printf(BOLDGREEN "reload                - reload HTML/JS/CSS/WASM" RESET "\n");
	printf(BOLDGREEN "cookie                - cookie ID of session to use for script execution" RESET "\n");
	printf(BOLDGREEN "exec                  - execute 1 line" RESET "\n");
	printf(BOLDGREEN "chain                 - chain job scripts" RESET "\n");
	printf(BOLDGREEN "loops                 - number of tests to do when using --chain" RESET "\n");
	printf(BOLDGREEN "wait                  - wait x amount of seconds before calling the undo script during a chain test" RESET "\n");
	printf(BOLDGREEN "update-wsj            - update all stocks' CSV's with EOD OHLC line" RESET "\n");
	printf(BOLDGREEN "update-eod            - reload stockdb and generate new signals.csv" RESET "\n");
	printf(BOLDGREEN "update-forks          - reload forks" RESET "\n");
	printf(BOLDGREEN "tradetime             - show tradetime score (PREMARKET DATA AVAILABILITY)" RESET "\n");
	printf(BOLDGREEN "threadstat            - thread workloads" RESET "\n");
	printf(BOLDGREEN "obj                   - object info" RESET "\n");
}

void lpc_exit(struct connection *connection, char **argv) {system("reset");exit(0);}
