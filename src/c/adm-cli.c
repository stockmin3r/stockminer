#include <stdinc.h>
#include <conf.h>

struct apc apc_commands[] = {
	{ APC_SESSION_LIST,   "sessions",    APC_SEND_STR},
	{ APC_WATCHLIST,      "watchlist",   APC_SEND_STR},
	{ APC_OHLC,           "ohlc",        APC_SEND_STR},
	{ APC_UPDATE_EOD,     "update-eod",  APC_SEND_VOID},
	{ APC_UPDATE_WSJ,     "update-wsj",  APC_SEND_VOID},
	{ APC_RELOAD_WEBSITE, "reload",      APC_SEND_VOID},
	{ APC_OPTIONS,        "options",     APC_MODE_OPTIONS},
	{ LPC_HELP,           "help",        LPC_HELP},
	{ LPC_USERS,          "users",       LPC_USERS},
	{ LPC_EXIT,           "exit",        LPC_EXIT}
};
int NR_APC_COMMANDS  = sizeof(apc_commands)/sizeof(struct apc);

struct history {
	char       cmd[256];
	int        id;
};

static struct connection  apc_connection;
static struct ssl_server *APC_SERVER;
static int                TERMINAL_MODE;
static int                nr_history = 0;
static struct history    *history_table;
static char              *server_cookie;
static int                nr_loops = 1;
static int                wait_time;

static void               lpc_help(void);
static void               lpc_users(void);

/*
 * Admin Procedure Call
 * [1] Send APC (no args)
 *   ---packet format----
 * [ apc_t   ] 2 bytes
 */
void apc_send(apc_t apc)
{
	packet_size_t packet_size  = sizeof(apc_t);
	char          packet[packet_size];

	*(apc_t *)packet = apc;
	openssl_write_sync(&apc_connection, packet, packet_size);
}

/*
 * Admin Procedure Call
 * [1] Send Integer
 *   ---packet format----
 * [ apc_t         ] 2 bytes (APC_ARG_INT | APC_CONSTANT)
 * [ integer arg   ] 4 bytes
 */
void apc_send_int(apc_t apc, int apc_integer_arg)
{
	packet_size_t packet_size  = sizeof(apc_t) + sizeof(int);
	char          packet[packet_size];

	apc                            = APC_SET_ARG_INT(apc);
	*(apc_t *)(packet)             = apc;
	*(int   *)(packet+sizeof(apc)) = apc_integer_arg;
	openssl_write_sync(&apc_connection, packet, packet_size);
}

/*
 * Admin Procedure Call
 * [1] Send String
 *   ---packet format----
 * [ apc_t         ] 2 bytes (APC_ARG_STR | APC_CONSTANT)
 * [ packet_size_t ] 8 bytes size of string
 * [ string arg    ] x bytes (not null terminated)
 */
void apc_send_str(apc_t apc, char *apc_string_arg)
{
	packet_size_t string_size  = strlen(apc_string_arg);
	packet_size_t packet_size  = sizeof(apc_t) + sizeof(packet_size_t) + string_size;
	char          packet[packet_size];

	apc                                    = APC_SET_ARG_STR(apc);
	*(apc_t         *)(packet)             = apc;
	*(packet_size_t *)(packet+sizeof(apc)) = string_size;
	memcpy(&packet[sizeof(apc)+sizeof(packet_size_t)], apc_string_arg, string_size);
	openssl_write_sync(&apc_connection, packet, packet_size);
}

apc_result_t apc_wait(apc_t apc)
{
	apc_result_t result;

	apc_send(apc);
//	apc_get_int(&result);
	return (result);
}

/*
 * Simple no argument APCs to send from main() via CLI
 */
void apc_send_command(apc_t apc)
{
	if (!openssl_connect_sync(&apc_connection, LOCALHOST, NETCTL_PORT)) {
		printf(BOLDRED "[-] Can't connect to localhost:%d" RESET "\n", NETCTL_PORT);
		exit(-1);
	}
	apc_send(apc);
}

struct apc *search_apc(char *cmdline)
{
	struct apc *apc;
	int         len;

	for (int x=0; x<NR_APC_COMMANDS; x++) {
		apc = &apc_commands[x];
		len = strlen(apc->name);
		if (!strncmp(apc->name, cmdline, len))
			return (apc);
	}
	return (NULL);
}

static void
apc_exec(char *cmdline)
{
	struct apc *apc;
	char       *args;

	if (*cmdline == '\0' || *cmdline == '\n')
		return;

	if (!(apc=search_apc(cmdline))) {
		printf("\n[-] no such command\n");
		return;
	}

	switch (apc->argtype) {
		case APC_SEND_STR:
			args = strchr(cmdline, ' ');
			if (!args)
				break;
			apc_send_str(apc->apc, args+1);
			break;
		case APC_SEND_INT:
			args = strchr(cmdline, ' ');
			if (!args)
				break;
			apc_send_int(apc->apc, atoi(args+1));
			break;
		case APC_SEND_VOID:
			apc_send(apc->apc);
			break;
		case LPC_HELP:
			lpc_help();
			break;
		case LPC_USERS:
			lpc_users();
			break;
		case LPC_EXIT:
			exit(0);
	}
}

static void
cmd_history_add(char *cmd)
{
	strncpy(history_table[nr_history].cmd, cmd, 255);
	history_table[nr_history].id = nr_history;
	nr_history++;
}

static struct history *
cmd_history_last_cmd(void)
{
	return &history_table[nr_history-1];
}

static void
cmd_history_print(void)
{
	for (int x=0; x<nr_history; x++)
		printf("%s\n", history_table[x].cmd);
}

void debug_stock(char *ticker, int thread)
{
	char msg[256];
	char *cmdstr = msg;
	if (thread != -1) {
		snprintf(msg, sizeof(msg)-1, "%s:%d", ticker, thread);
		cmdstr = msg;
	} else
		cmdstr = ticker;
	apc_send_str(APC_DEBUG_STOCK, cmdstr);
}

void cmd_ebuild(char *ticker)
{
	if (*ticker != '-')
		apc_send_str(APC_EBUILD_STOCK, ticker);
	else
		apc_wait(APC_EBUILD_STOCKS);
}

static void lpc_users(void)
{
	struct filemap  filemap;
	struct user    *user;
	char            buf[256];
	char            daystr[256];
	char           *umap;
	time_t          timenow = time(NULL);
	int             nr_users, days, x;

	umap     = MAP_FILE_RO("db/users.db", &filemap);
	nr_users = (filemap.filesize/sizeof(struct user));
	user     = (struct user *)umap;
	for (x=0; x<nr_users; x++) {
		days = (timenow-user->join_date)/86400;
		snprintf(daystr, sizeof(daystr)-1, BOLDGREEN "(%d days)" RESET, days);
		printf(BOLDBLUE "%-24s [uid: %d] %s logged_in: %d", user->uname, user->uid, daystr, user->logged_in);
		user++;
	}
	UNMAP_FILE(umap, &filemap);
}

int get_uid(char *username)
{
	struct user *user;
	struct filemap filemap;
	char *umap;
	int nr_users, x, id = -1;

	umap     = MAP_FILE_RO("db/users.db", &filemap);
	nr_users = (filemap.filesize/sizeof(struct user));
	user     = (struct user *)umap;
	for (x=0; x<nr_users; x++) {
		if (!strcmp(username, user->uname)) {
			id = x;
			break;
		}
		user++;
	}
	UNMAP_FILE(umap, &filemap);
	return (id);
}

void cmd_watchdb(char *username)
{
	struct watchlist  *watchlist;
	struct watchstock *watchstock;
	struct filemap     filemap;
	struct user       *user;
	char              *umap, *stock;
	char               path[256];
	int nr_watchlists, nr_users, count = 0, x, y, id = -1;

	id = get_uid(username);
	if (id >= 0) {
		snprintf(path, sizeof(path)-1, "db/uid/%d.watch", id);
		umap = MAP_FILE_RO(path, &filemap);
		if (!umap) {
			printf(BOLDRED "user: %s has no watchlists" RESET "\n", username);
			return;
		}
	} else {
		snprintf(path, sizeof(path)-1, "db/uid/free/%s.watch", username);
		umap = MAP_FILE_RO(path, &filemap);
		if (!umap) {
			printf(BOLDRED "user: %s not found" RESET "\n", username);
			return;
		}
	}
	watchlist = (struct watchlist *)umap;
	nr_watchlists = (filemap.filesize/sizeof(struct watchlist));
	for (x=0; x<nr_watchlists; x++) {
		printf("WatchList Name:   %s %s\n", watchlist->name, watchlist->config==IS_PUBLIC?"(PUB)":"(PRIV)");
		printf("BaseTable:        %s\n", watchlist->basetable);
		printf("Number of Stocks: %d\n", watchlist->nr_stocks);
		printf("--------------------\n");
		for (y=0; y<watchlist->nr_stocks; y++) {
			watchstock = &watchlist->stocks[y];
			if (count++ > 6) {
				printf("\n");
				count = 0;
			}
			printf(BOLDWHITE "%-8s " RESET, watchstock->ticker);
		}
		printf(BOLDGREEN "\n--------------------------------------" RESET "\n");
		watchlist++;
	}
	UNMAP_FILE(path, &filemap);
}

void cmd_watchlist(char *watchlist)
{
	int sockfd, id, cmd, len;

	if (watchlist) {
		cmd = APC_WATCHLIST;
		len = strlen(watchlist);
	} else {
		cmd = APC_LIST_WATCHLISTS;
	}
	sockfd = net_tcp_connect("127.0.0.1", NETCTL_PORT);
	write(sockfd, (void *)&cmd, 4);
	if (watchlist) {
		write(sockfd, (void *)&len, 4);
		write(sockfd, watchlist, len);
	} else {
		len = 0;
		write(sockfd, (void *)&len, 4);
	}
}

void netshell_cmd(char *str)
{
	char cmd[4096];

	snprintf(cmd, sizeof(cmd)-1, "%.8s %s", server_cookie, str);
	apc_send_str(APC_NETSH, cmd);
}

void run_script(char *job)
{
	char *p;
	char path[256];

	snprintf(path, sizeof(path)-1, "scripts/jobs/%s", job);
	job = fs_mallocfile_str(path, NULL);
	if (!job)
		return;
	while (p=strchr(job, '\n')) {
		*p++ = 0;
		if (*job == '*') {
			job = p;
			continue;
		}
		if (!strncmp(job, "sleep ", 6))
			os_sleep(atoi(job+6));
		netshell_cmd(job);
		job = p;
		os_usleep(500000);
	}
	free(job);
}

void cmd_netshell_chain(char *scripts)
{
	char *argv[32];
	char undo[512];
	char *p;
	int argc, x, y;

	argc = cstring_split(scripts, argv, 31, ',');
	if (argc <= 0)
		return;
	argc++;
	for (x=0; x<nr_loops; x++) {
		for (y=0; y<argc; y++)
			run_script(argv[y]);
		if (wait_time)
			os_sleep(wait_time);
		for (y=0; y<argc; y++) {
			snprintf(undo, sizeof(undo)-1, "%s.undo", argv[y]);
			run_script(undo);
		}
	}
}

void cmd_exec(char *cmd)
{
	netshell_cmd(cmd);
}

void cmd_tradetime(char *ticker)
{
	struct filemap    filemap;
	struct tradetime *tradetime;
	char              path[256];

	snprintf(path, sizeof(path)-1, "data/stocks/stockdb/%s.tradetime", ticker);
	tradetime = (struct tradetime *)MAP_FILE_RO(path, &filemap);
	if (!tradetime) {
		printf("no tradetime data for: %s\n", ticker);
		exit(-1);
	}
	printf("4am: %d\n", tradetime->time_4AM);
	printf("5am: %d\n", tradetime->time_5AM);
	printf("6am: %d\n", tradetime->time_6AM);
	printf("7am: %d\n", tradetime->time_7AM);
	printf("8am: %d\n", tradetime->time_8AM);
	printf("9am: %d\n", tradetime->time_9AM);
}

static void
lpc_help(void)
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

void admin_client_loop(void)
{
	FILE           *fp;
	struct termios  term;
	struct history *hist      = NULL;
	char            buf[256]  = {0};
	int             n = 0, key, pos = 0, noprompt = 0, cmd_mode = 0;

	history_table = zmalloc(sizeof(struct history)*4096);
	if (!openssl_connect_sync(&apc_connection, LOCALHOST, NETCTL_PORT)) {
		printf(BOLDRED "[-] Can't connect to localhost:%d" RESET "\n", NETCTL_PORT);
		exit(-1);
	}

	tcgetattr(0, &term);
	memcpy(&sterm, &term, sizeof(term));
	term.c_lflag |= IGNPAR;
	term.c_lflag &= ~(ISTRIP | INLCR  | IGNCR | ICRNL | IXON  | IXANY | IXOFF);
	term.c_lflag &=	~(ISIG   | ICANON | ECHO  | ECHOE | ECHOK | ECHONL);
	term.c_lflag &= ~OPOST;
	term.c_cc[VMIN]  = 1;
	term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &term);

	for (;;) {
		term_noecho();
		switch (TERMINAL_MODE) {
			/**********************************
			 * QSHELL scripting command prompt
			 */
			case APC_MODE_SCRIPTING:
				printf(BLUE  "<QSH> " RESET);
				fflush(stdout);
				break;
			/*******************************************************
			 * For commands dealing with remotely fetching objects
			 * such as (HTML Tables, PDFs) and processing them
			 * there will be a "set" command for defining variables
			 */
			case APC_MODE_WWW:
				printf(GREEN "<WWW> " RESET);
				fflush(stdout);
				break;
			default:
				printf(BOLDRED "<#> " RESET);
				fflush(stdout);
		}
		noprompt = 0;
		key      = fs_readline_tty(buf+pos, &n);
		if (key == -1) {
			noprompt = 1;
			continue;
		}
		pos = n;
		switch (key) {
			case KEY_NONE: {
				noprompt = 1;
				continue;
			};
			case UP_KEY: {
				if (!nr_history) {
					noprompt = 1;
					continue;
				}
				if (hist) {
					if (!hist->id) {
						noprompt = 1;
						continue;
					}
					term_erase(strlen(hist->cmd));
					hist = &history_table[hist->id-1];
				}
				else
					hist = cmd_history_last_cmd();
				write(1, hist->cmd, strlen(hist->cmd));
				noprompt = 1;
				continue;
			};
			case DOWN_KEY: {
				if (!nr_history || !hist || (hist->id == nr_history)) {
					noprompt = 1;
					continue;
				}
				term_erase(strlen(hist->cmd));
				hist = &history_table[hist->id+1];
				write(1, hist->cmd, strlen(hist->cmd));
				noprompt = 1;
				if (!hist->id)
					hist = NULL;
				continue;
			};
			case BACKSPACE_KEY: {
				buf[pos] = 0;
				noprompt = 1;
				continue;
			};
			case KEY_ENTER: {
				if (!hist)
					break;
				cmd_history_add(hist->cmd);
				buf[n+pos] =0;
				break;
			}
			default:
				cmd_history_add(buf);
				break;
		}
//		tcsetattr(0,TCSAFLUSH,&sterm);
//		tcsetattr(0, TCSADRAIN, &sterm);
		if (hist) {
			apc_exec(hist->cmd);
			hist = NULL;
			continue;
		}
		cmd_history_add(buf);
		apc_exec(buf);
		pos = 0;
		n   = 0;
		memset(buf, 0, sizeof(buf));
	}
}