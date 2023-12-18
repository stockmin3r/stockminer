#include <stdinc.h>
#include <conf.h>

static char *server_cookie;
static int   nr_loops = 1;
static int   wait_time;
char        *webscripts;
int          webscripts_size;
int          max_scripts_size;

void init_webscript()
{
	struct dirmap  dirmap;
	struct stat    sb;
	char          *file, *filename;
	char           path[512];
	int64_t        filesize, pathlen;

	if (!fs_opendir("src/scripts/webscript", &dirmap))
		return;
	webscripts    = (char *)malloc(64 KB);
	max_scripts_size = 64 KB;
	strcpy(webscripts, "webscripts ");
	webscripts_size = 11;
	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (*filename == '.')
			continue;
		pathlen = snprintf(path, sizeof(path)-32, "src/scripts/webscript/%s", filename);
		if (path[pathlen-1] == '~')
			continue;
		if (stat(path, &sb) == -1)
			continue;
		file = fs_mallocfile_str(path, &filesize);
		if (!file)
			continue;
		if (webscripts_size + filesize + 4 > max_scripts_size) {
			max_scripts_size += MAX(64 KB, filesize+1);
			webscripts     = (char *)realloc(webscripts, max_scripts_size);
			if (!webscripts)
				exit(-1);
		}
		webscripts_size += snprintf(webscripts+webscripts_size, max_scripts_size-webscripts_size-32, "%s%s!", filename, file);
		free(file);
	}
	fs_closedir(&dirmap);
}

void run_script(char *job)
{
	char  path[256];
	char  cmd[4096];
	char *script, *p;

	snprintf(path, sizeof(path)-1, "scripts/webscript/%s", job);
	script = fs_mallocfile_str(path, NULL);
	if (!script)
		return;

	while (p=strchr(script, '\n')) {
		*p++ = 0;
		if (*script == '*') {
			script = p;
			continue;
		}
		if (!strncmp(script, "sleep ", 6))
			os_sleep(atoi(script+6));

		/*
		 * Format the script line command
		 */
		snprintf(cmd, sizeof(cmd)-1, "webscript %s", script);
		apc_send_command(cmd);
		script = p;
		os_usleep(500000);
	}
	free(script);
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

void apc_webscript(struct connection *connection, char **argv)
{
	struct session *session;
	char            cmd[8 KB];
	packet_size_t   packet_size;

	session = session_by_username(argv[0]);
	if (connection->session != session || !connection->authorized)
		return;

	packet_size = snprintf(cmd, sizeof(cmd)-1, "netsh %s", argv[1]);
	printf("cmd: %s\n", cmd);
	websockets_sendall(session, cmd, packet_size);
}

