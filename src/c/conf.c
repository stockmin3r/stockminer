#include <conf.h>

#define MAX_CONF 512

static struct server *SERVER;

char *alloc_config_file()
{
	struct stat sb;
	const char *path;

	if (stat("etc/config.ini", &sb) != -1)
		path = "etc/config.ini";
	else if (stat("config.ini", &sb) != -1)
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

	config_get("http_port",  CONF_TYPE_INT, NULL, (uint64_t *)&server->http_port);
	config_get("https_port", CONF_TYPE_INT, NULL, (uint64_t *)&server->https_port);
	config_get("daemon",     CONF_TYPE_INT, NULL, (uint64_t *)&server->daemon);

	return;
out_error:
	printf(BOLDRED "config param: (%s:%s) failure", key, val);
	exit(-1);
}
