#include <conf.h>
#include <extern.h>

struct module **module_list;
int             nr_modules;

// module::filename.c::hookname
/**************************
 *                        *
 * QuadVerse Module Hooks *
 *                        *
 *************************/
// quadverse::session.c::init
__MODULE_HOOK quadverse_session_init(struct session *session)
{
	printf(BOLDCYAN "workspace init hook" RESET "\n");
	session_load_quadverse(session);
	session_load_qcache(session);
	session_load_profile(session);
	session_load_css(session);
}

// quadverse::session.c::alloc
__MODULE_HOOK quadverse_session_alloc(struct session *session)
{
	session_load_quadverse(session);
}

// quadverse::main.c::post_loop
__MODULE_HOOK quadverse_main_post_loop(struct server *server)
{
	load_quadverse_pages();
}

/**********************
 *                    *
 * Stock Module Hooks *
 *                    *
 *********************/

// stocks::session.c::init
__MODULE_HOOK stocks_session_init(struct session *session)
{
	session_load_watchlists(session);
	session_load_presets(session);
	session_load_watchtable_presets(session);
	session->ufo_tables = UFO_LOWCAPS;
}

// stocks::session.c::alloc
__MODULE_HOOK stocks_session_alloc(struct session *session)
{
	struct watchlist *watchlist;
	struct wtab      *wtab;

	session->ufo_tables = UFO_LOWCAPS;

	/* morphtab watchtable */
	watchlist = (struct watchlist *)zmalloc(sizeof(*watchlist));
	memset(watchlist, 0, sizeof(*watchlist));
	strcpy(watchlist->name,      "morphtab");
	strcpy(watchlist->basetable, "morphtab");
	watchlist->origin      = NULL;
	session->watchlists[0] = watchlist;
	watchlist->config      = IS_MORPHTAB;
	session->morphtab      = watchlist;
	session->nr_watchlists = 1;

	/* morphtab New^Preset */
	wtab = (struct wtab *)zmalloc(sizeof(*wtab));
	wtab->colmap[0]  = 900; wtab->colmap[1]  = 901;
	wtab->colmap[2]  = 903; wtab->colmap[3]  = 904;
	wtab->nr_columns = 4;
	wtab->table_type = WATCHTABLE_ROOT;
	session->nr_watchtable_presets = 1;
	session->watchtable_presets[0] = wtab;
	strcpy(wtab->name, "New^Preset");
	session->morphtab->wtab = wtab;
}

__MODULE_HOOK stocks_main_post_loop  (struct server  *server)
{
	verify_stocks(server->XLS);
	init_anyday(server->XLS);
	init_BIX(server->XLS);
	init_monster(server->XLS, 1); // will be called via a 2nd module layer when monster.c,forks.c,etc all move to a separate subdir of src/stocks/
	init_ufo(server->XLS);
	build_candle_screener(server->XLS);
//	init_options();
//	init_backtest();
}


/***************************
 * QuadVerse __MODULE_INIT
 **************************/
__MODULE_INIT init_quadverse_module(void)
{
	struct module *module       = (struct module *)zmalloc(sizeof(*module));
	module->name                = "quadverse";
	module->enabled             = true;
	module->main_post_loop_hook = quadverse_main_post_loop;
	module->session_init_hook   = quadverse_session_init;
	module->session_alloc_hook  = quadverse_session_alloc;
	register_module(module);
}

/***************************
 * Stocks __MODULE_INIT
 **************************/
__MODULE_INIT init_stocks_module(struct server *server)
{
	struct module *module       = (struct module *)zmalloc(sizeof(*module));
	module->name                = "stocks";
	module->enabled             = true;
	module->session_init_hook   = stocks_session_init;
	module->session_alloc_hook  = stocks_session_alloc;
	module->main_post_loop_hook = stocks_main_post_loop;
	server->XLS = CURRENT_XLS   = load_stocks(); // load data/stocks/STOCKS.TXT into server->XLS
	server->XLS->config         = server;
	init_wtable();
	init_watchtable();
	init_candles(server); // load build/libta.so, if present
	init_ranks(server->XLS);
	market = get_time();
	if (server->XLS)
		register_module(module);
	/*
	 * Checkpoint for installation/updating:
	 *   - if user is installing for the first time then they will have no stock data
	 *   - the data must be fetched first (from a configed provider (default: yahoo)
	 *   - the web server will send back an "RPC checkpoint" message until the data
	 *     has downloaded. When every x amount of tickers (eg: 50) are fetched,
	 *     the webserver will (re)create the stock threads and send all sessions
	 *     an RPC message to either reload the website or re-issue the "boot" RPC
	 *     (in which case the website wouldn't need to be reloaded)
	 */
	thread_create(stocks_update_checkpoint, (void *)server->XLS);
}


void module_hook(void *args, int hook)
{
	if (unlikely(!module_list))
		return;

	for (int x=0; x<nr_modules; x++) {
		struct module *module = module_list[x];
		if (!module || !module->enabled)
			continue;

		switch (hook) {
			/*
			* Session Hooks
			*/
			case MODULE_SESSION_INIT:
				if (module->session_init_hook)
					module->session_init_hook((struct session *)args);
				break;
			case MODULE_SESSION_ALLOC:
				if (module->session_alloc_hook)
					module->session_alloc_hook((struct session *)args);
				break;
			/*
			* Main loop pre/post hooks
			*/
			case MODULE_MAIN_PRE_LOOP:
				if (module->main_pre_loop_hook)
					module->main_pre_loop_hook((struct server *)args);
				break;
			case MODULE_MAIN_POST_LOOP:
				if (module->main_post_loop_hook)
					module->main_post_loop_hook((struct server *)args);
				break;

		}
	}
}

void init_modules(struct server *server)
{
	char *modules[MAX_MODULES];
	struct config *config;

	if (!(config=config_get("modules", CONF_TYPE_STR, NULL, NULL)))
		return;

	nr_modules  = cstring_split(config->val_string_t, modules, MAX_MODULES, ',');
	module_list = (struct module **)zmalloc(sizeof(struct module *) * MAX_MODULES);
	for (int x  = 0; x<nr_modules; x++) {
		if (!strcmp(modules[x], "stocks")) {
			init_stocks_module(server);
			printf(BOLDGREEN "+Loaded stocks module"    RESET "\n");
		} else if (!strcmp(modules[x], "quadverse")) {
			init_quadverse_module();
			printf(BOLDGREEN "+Loaded quadverse module" RESET "\n");
		}
	}
}


void module_enable(char *module_name)
{


}

void module_disable(char *module_name)
{

}

void register_module(struct module *module)
{
	for (int x=0; x<MAX_MODULES; x++) {
		if (module_list[x] && !strcmp(module_list[x]->name, module->name))
			return;
		if (!module_list[x]) {
			printf("registering module: %s idx: %d\n", module->name, x);
			module_list[x] = module;
			return;
		}
	}
}
