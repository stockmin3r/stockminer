#include <conf.h>
#include <extern.h>
#include <curl/curl.h>

const char *DB_PATH;
const char *DB_USERS_PATH;
const char *DB_REPO_PATH;
const char *DB_LOG_PATH;
const char *STOCKDB_PATH;
const char *STOCKS_DIR_PATH;
const char *STOCKDB_MAG2_PATH;
const char *STOCKDB_MAG3_PATH;
const char *STOCKDB_MAG4_PATH;
const char *STOCKDB_CSV_PATH;
const char *STOCKS_PATH;
const char *STOCKS_DAYS_PATH;
const char *STOCKS_WEEKS_PATH;
const char *GSPC_PATH;
const char *OPTIONS_PATH;

int             verbose;
int             market;
int             do_resume;
struct server   Server;

void init_main(struct server *server)
{
	init_paths();
	init_config(server);
	init_time(&Server);
	hydro_init();
	SSL_library_init();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	init_os(server);
	init_ip();
	init_webscript();
	init_db(server);
}

struct option main_options[] =
{
	{ "admin",           0, NULL, 'a' },
	{ "verbose",         0, NULL, 'v' },
	{ "update-fundb",    0, NULL, 'F' },
	{ "mag",             0, NULL, 'm' },
	{ "EOD",             1, NULL, '3' },
	{ "holiday",         1, NULL, 'H' },
	{ "wsjall",          1, NULL, 'w' },
	{ "debug",           1, NULL, 'D' },
	{ "daemon",          1, NULL, 'd' },
	{ "ebuild",          0, NULL, 'e' },
	{ "reload",          0, NULL, 'r' },
	{ "benchmark",       1, NULL, 'b' }
};

int main(int argc, char *argv[])
{
	long op;

	init_main(&Server);

	while ((op=getopt_long(argc, argv, "W:H:mw:D:b:a3evrd", &main_options[0], NULL)) != -1) {
		switch (op) {
			case 'a':
				admin_client_loop();
				exit(0);
			case 'b':
				task_benchmark(optarg);
				break;
			case 'd':
				Server.daemon = true;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'r':
				/* reload src/website && www/hydrogen.wasm */
				apc_connect_command("reload");
				exit(0);
			case 'm':
				build_mag(NULL, Server.XLS);
				exit(0);
			case '3':
				wsj_update_stock(optarg);
				exit(0);
			case 'F':
				update_fundb(argv[2], Server.XLS);
				exit(0);
			case 'H':
				holiday = atoi(optarg);
				break;
			case 'w':
				argv_wsj_allday(optarg);
				exit(0);
			case 'D':
				Server.DEBUG_STOCK = strdup(optarg);
				printf(BOLDYELLOW "DEBUG_STOCK=%s" RESET "\n", Server.DEBUG_STOCK);
				break;
			case 'e':
				// ebuild
				build_stock_earnings();
				exit(0);
			default:
				exit(0);
		}
	}

	init_modules(&Server);

	/* 
	 * Start the www server BEFORE stocks we attempt to load the tickers
	 * since there may be no tickers at all (first install) or there may
	 * not be enough (partial install). In this case the websocket will
	 * send back an RPC CHECKPOINT message notifying the browser/app
	 * that data is still being downloaded.
	 */
	thread_create(init_www, NULL);

	init_tasks();

	while (stockdata_checkpoint < SD_CHECKPOINT_COMPLETE)
		sleep(1);

	fs_log("checkpoint");
	module_hook(&Server, MODULE_MAIN_PRE_LOOP);
//	create_stock_threads(Server.XLS);
	module_hook(&Server, MODULE_MAIN_POST_LOOP);

	thread_create(json_thread,          NULL);
	thread_create(netctl_thread,        NULL);
	thread_create(db_thread,            NULL);

	set_current_minute();

	/*
 	 * On first or partial install, stocks.c::stocks_update_checkpoint() will fetch stocks until
	 * complete, meanwhile, depending on the value of stockdata_checkpoint, the www server will
	 * respond with different RPCs when the app/browser loads the websocket
	 */
//	stockdata_checkpoint = true;

	stock_loop(&Server);
}
