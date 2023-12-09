#include <conf.h>

#define MAX_CONF 512

static struct server *SERVER;

char *alloc_config_file()
{
	const char *path;

	if (fs_file_exists("etc/config.ini"))
		path = "etc/config.ini";
	else if (fs_file_exists("/usr/local/stockminer/etc/config.ini"))
		path = "/usr/local/stockminer/etc/config.ini";
	else if (fs_file_exists("config.ini"))
			path = "config.ini";
	else
		return NULL;
	return fs_mallocfile_str((char *)path, NULL);
}

bool config_enabled(const char *key)
{
	struct config *config = NULL;

	HASH_FIND_STR(SERVER->CONFIG_HASHTABLE, key, config);
	if (!config)
		return 0;
	if (config->val_uint64_t == 1)
		return true;
	return false;
}

struct config *config_get(const char *key, int var_type, char **out_cstr, uint64_t *out_uint64)
{
	struct config *config = NULL;

	if (!key)
		return NULL;

	HASH_FIND_STR(SERVER->CONFIG_HASHTABLE, key, config);
	if (!config)
		return NULL;

	switch (var_type) {
		case CONF_TYPE_INT:
			if (out_uint64)
				*out_uint64 = config->val_uint64_t;
			break;
		case CONF_TYPE_STR:
			if (out_cstr)
				*out_cstr = config->val_string_t;
			break;
	}
	return config;
}

void init_config(struct server *server)
{
	struct config *config_entry;
	char          *config_file_str = alloc_config_file();
	char          *config_argv[MAX_CONF];
	char          *key, *val;
	char          *domain;
	int            argc, x;
	int64_t        nr_lines;

	SERVER = server;
	if (!config_file_str) {
		printf(BOLDRED "[config] file missing in stockminer directory" RESET "\n");
		exit(-1);
	}

	nr_lines = cstring_line_count(config_file_str);
	if (!nr_lines)
		exit(-1);

	argc = cstring_split(config_file_str, config_argv, nr_lines, '\n');
	if (argc <= 0)
		return;

	for (x=0; x<nr_lines; x++) {
		config_entry = (struct config *)zmalloc(sizeof(struct config));
		key          = config_argv[x];
		if (*key == '#' || *key == '\n' || *key == '\r' || *key == '[')
			continue;
		val = strchr(key, '=');
		if 	(!val || (*(val+1)=='\n') || (*(val+1) == '\r'))
			continue;
		if (*val == '\0')
			goto out_error;
		*val++ = 0;
		config_entry->key = key;
		config_entry->val_string_t = strdup(val);
		config_entry->val_uint64_t = strtoull(val, NULL, 10);
		HASH_ADD_STR(SERVER->CONFIG_HASHTABLE, key, config_entry);
	}

	if (config_enabled("production"))
		server->production = true;

	if (config_enabled("async"))
		server->async = true;

	if (config_get("domain", CONF_TYPE_STR, &domain, NULL))
		strncpy(server->domain, domain, sizeof(server->domain)-1);

	char *stocks_1D, *stocks_1M, *crypto_1D, *crypto_1M, *stocks_WS, *crypto_WS;

	config_get("stocks-1D", CONF_TYPE_STR, &stocks_1D, NULL); // Historical 1D OHLCv (HTTP GET)
	config_get("crypto-1D", CONF_TYPE_STR, &crypto_1D, NULL); // Historical 1D OHLCv (HTTP GET)
	config_get("stocks-1M", CONF_TYPE_STR, &stocks_1M, NULL); // Daily 1M OHLCv      (HTTP GET)
	config_get("crypto-1M", CONF_TYPE_STR, &crypto_1M, NULL); // Daily 1M OHLCv      (HTTP GET)
	config_get("crypto-WS", CONF_TYPE_STR, &crypto_WS, NULL); // Websocket crypto data: eg cryptocompare,kucoin(TODO)

	if (!strcmp(stocks_1D, "yahoo"))
		server->stocks_1D = STOCKDATA_YAHOO;
	else
		server->stocks_1D = STOCKDATA_OFF;

	if (!strcmp(crypto_1D, "yahoo"))
		server->crypto_1D = STOCKDATA_YAHOO;
	else
		server->crypto_1D = STOCKDATA_OFF;

	/* Polling (GET requests per ticker) every (x) second(s)
	 */
	if (!strcmp(stocks_1M, "WSJ"))
		server->stocks_1M = STOCKDATA_WSJ;
	else
		server->stocks_1M = STOCKDATA_OFF;

	/*
	 * Crypto (use WSJ only to get last 24 hours of OHLCv data up until this second
	 * since WSJ only has GET request polling which is costly
	 * After that use cryptocompare and other datasources for live price streaming
	 * since they have websockets with live updates for hundreds of coinpairs on only 1 connection
 	 */

	// 1M OHLCv ticks - since the last EOD - previous midnight (UTC)
	if (!strcmp(crypto_1M, "WSJ"))
		server->crypto_1M = STOCKDATA_WSJ;
	else
		server->crypto_1M = STOCKDATA_OFF;

	// websocket stream data
	if (!strcmp(crypto_WS, "cryptocompare"))
		server->crypto_WS = STOCKDATA_CRYPTOCOMPARE;

	config_get("http_port",  CONF_TYPE_INT, NULL, (uint64_t *)&server->http_port);
	config_get("https_port", CONF_TYPE_INT, NULL, (uint64_t *)&server->https_port);
	config_get("daemon",     CONF_TYPE_INT, NULL, (uint64_t *)&server->daemon);

	return;
out_error:
	printf(BOLDRED "config param: (%s:%s) failure", key, val);
	exit(-1);
}
