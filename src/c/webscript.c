#include <stdinc.h>
#include <conf.h>

static char *server_cookie;
static char *test_scripts;
static int   test_scripts_len;
static int   max_scripts_len;
static int   nr_loops = 1;
static int   wait_time;

void init_webscript()
{
	struct dirmap  dirmap;
	struct stat    sb;
	char          *file, *filename;
	char           path[512];
	int64_t        filesize, pathlen;

	if (!fs_opendir("scripts/jobs", &dirmap))
		return;
	test_scripts    = (char *)malloc(64 KB);
	max_scripts_len = 64 KB;
	strcpy(test_scripts, "qsh ");
	test_scripts_len = 4;
	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (*filename == '.')
			continue;
		pathlen = snprintf(path, sizeof(path)-32, "scripts/jobs/%s", filename);
		if (path[pathlen-1] == '~')
			continue;
		if (stat(path, &sb) == -1)
			continue;
		file = fs_mallocfile_str(path, &filesize);
		if (!file)
			continue;
		if (test_scripts_len + filesize + 4 > max_scripts_len) {
			max_scripts_len += MAX(64 KB, filesize+1);
			test_scripts     = (char *)realloc(test_scripts, max_scripts_len);
			if (!test_scripts) {
				printf(BOLDRED "[-] no memory to allocate test scripts!" RESET "\n");
				exit(-1);
			}
		}
		test_scripts_len += snprintf(test_scripts+test_scripts_len, max_scripts_len-test_scripts_len-32, "%s%s!", filename, file);
		free(file);
	}
	fs_closedir(&dirmap);
}

void netshell_cmd(char *str)
{
	char cmd[4096];

	snprintf(cmd, sizeof(cmd)-1, "%.8s %s", server_cookie, str);
//	apc_send_str(APC_NETSH, cmd);
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
/*
apc_result_t *apc_netsh(int sockfd)
{
	struct session *session;
	char buf[8 KB];
	char cmd[9 KB];
	size_t packet_len = 0;
	int nbytes;

	recv(sockfd, (void *)&packet_len, 4, 0);
	if (packet_len > 256)
		return;
	nbytes = recv(sockfd, buf, sizeof(buf)-1, 0);
	buf[nbytes] = 0;
	if (nbytes <= 0)
		return;
	session = session_by_cookie(buf);
	if (!session)
		return;
	nbytes = snprintf(cmd, sizeof(cmd)-1, "netsh %s", buf+9);
	websockets_sendall(session, cmd, nbytes);
}
*/
