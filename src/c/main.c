#include <conf.h>
#include <extern.h>
#include <curl/curl.h>

int             verbose;
int             market;
int             do_resume;
struct server   Server;

void init_main(struct server *server)
{
	init_config(server);
	hydro_init();
	SSL_library_init();
//	SSL_load_error_strings();
//	ERR_load_crypto_strings();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	init_os(server);
	init_time(server);
	init_ip();
	init_webscript();
	init_db(server);
	init_modules(server);
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
	{ "ebuild",          0, NULL, 'e' },
	{ "reload",          0, NULL, 'r' }
};

void stockminer_main(struct server *server)
{
	memcpy(&Server, server, sizeof(*server));

	init_main(server);

	module_hook(&Server, MODULE_MAIN_PRE_LOOP);
	create_stock_threads(server->XLS);
	module_hook(&Server, MODULE_MAIN_POST_LOOP);

	stock_loop(server);
}

int main(int argc, char *argv[])
{
	long op;

	init_main(&Server);

	while ((op=getopt_long(argc, argv, "W:H:m:w:D:a3evr", &main_options[0], NULL)) != -1) {
		switch (op) {
			case 'a':
				admin_client_loop();
				exit(0);
			case 'v':
				verbose = 1;
				break;
			case 'r':
				/* reload src/website && www/hydrogen.wasm */
				apc_send_command("reload");
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

	module_hook(&Server, MODULE_MAIN_PRE_LOOP);
	create_stock_threads(Server.XLS);
	module_hook(&Server, MODULE_MAIN_POST_LOOP);

	thread_create(init_www,      NULL);
	thread_create(json_thread,   NULL);
	thread_create(netctl_thread, NULL);
	thread_create(db_thread,     NULL);
//	thread_create(db_submit_tasks, NULL);

	set_current_minute();
	printf(BOLDGREEN "SERVER INITIALIZED" RESET "\n");
	stock_loop(&Server);
}
