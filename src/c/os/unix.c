#include <stdinc.h>
#include <conf.h>
#include <extern.h>

struct termios sterm;

#ifndef __WINDOWS__

char *MAP_FILEMAP_RW(struct filemap *filemap)
{
	char *map = (char *)mmap(NULL, filemap->filesize, PROT_READ|PROT_WRITE, MAP_SHARED, filemap->fd, 0);
	if (map == (void *)-1)
		return NULL;
	return (map);
}

char *MAP_FILE_RO(char *path, struct filemap *filemap)
{
	struct stat sb;
	int fd;

	fd = open(path, O_RDONLY|O_CLOEXEC);
	if (fd < 0)
		return NULL;
	fstat(fd, &sb);
	if (!sb.st_size) {
		close(fd);
		return NULL;
	}
	filemap->map = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (filemap->map == (void *)-1) {
		close(fd);
		return NULL;
	}
	filemap->fd = fd;
	filemap->filesize = sb.st_size;
	return (filemap->map);
}

char *MAP_FILE_RW(char *path, struct filemap *filemap)
{
	struct stat sb;
	char *map;
	int fd;

	fd = open(path, O_RDWR|O_CLOEXEC);
	if (fd < 0) {
		filemap->filesize = 0;
		filemap->fd = -1;
		return NULL;
	}
	fstat(fd, &sb);
	if (!sb.st_size) {
		close(fd);
		return NULL;
	}
	map = (char *)mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == (void *)-1) {
		close(fd);
		return NULL;
	}
	filemap->fd = fd;
	filemap->filesize = sb.st_size;
	return (map);
}

char *MAP_FILE_CRW(char *path, struct filemap *filemap)
{
	struct stat sb;
	char *map;
	int fd;

	fd = open(path, O_RDWR|O_CREAT|O_CLOEXEC, 0644);
	if (fd < 0)
		return NULL;
	fstat(fd, &sb);
	if (!sb.st_size) {
		close(fd);
		return NULL;
	}
	map = (char *)mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == (void *)-1) {
		close(fd);
		return NULL;
	}
	filemap->fd = fd;
	filemap->filesize = sb.st_size;
	return (map);
}

void UNMAP_FILE(char *map, struct filemap *filemap)
{
	munmap(map, filemap->filesize);
	close(filemap->fd);
}

void net_socket_block(socket_t sockfd)
{
	int val = fcntl(sockfd, F_GETFL);
	fcntl(sockfd, F_SETFL, val&~O_NONBLOCK);
}
void net_socket_nonblock(socket_t sockfd)
{
	int val = fcntl(sockfd, F_GETFL);
	fcntl(sockfd, F_SETFL, val|O_NONBLOCK);
}

void event_mod(struct connection *connection)
{
	struct epoll_event event;
	event.events   = connection->events;
	event.data.ptr = (void *)connection;
	epoll_ctl(connection->event.event_fd, EPOLL_CTL_MOD, connection->fd, &event);
}

void event_del(struct connection *connection)
{
	struct epoll_event event;
	event.events   = 0;
	event.data.ptr = NULL;
	epoll_ctl(connection->event.event_fd, EPOLL_CTL_DEL, connection->fd, &event);
}


void thread_create(void *(*thread)(void *), void *args)
{
	pthread_t tid;
	pthread_create(&tid, NULL, thread, args);
	pthread_detach(tid);
}

static void server_restart(int sig)
{
	char *exec[2] = { "./stockminer", NULL };
	execve(exec[0], exec, NULL);
}

static void child_handler(int signum)
{
    switch(signum) {
	    case SIGALRM: exit(-1); break;
	    case SIGUSR1: exit(0);  break;
	    case SIGCHLD: exit(-1); break;
    }
}

void term_noecho()
{
	struct termios term;

	tcgetattr(0, &term);
	term.c_lflag |= IGNPAR;
	term.c_lflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXANY | IXOFF);
	term.c_lflag &=	~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL);
	term.c_lflag &= ~OPOST;
	term.c_cc[VMIN]  = 1;
	term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &term);
}

void term_reset()
{
	tcsetattr(0, TCSANOW, &sterm);
}

void term_erase(int n)
{
	for (int x=0; x<n; x++)
		write(1, "\x08\x1b\x5b\x4b", 4);
}

int fs_readline_tty(char *s, int *len)
{
	int           cmd_esc = 0, count = 0, n;
	char          key[2] = {0, 0};
	unsigned char c = 0;

	while (1) {
		n = read(0, &c, 1);
		if (n <= 0)
			return -1;
		switch (c) {
			case '\n':
				*s = '\0';
				write(2, "\n", 1);
				return (KEY_ENTER);
			case '\033':
				n = read(0, key, 2);
				if (((key[0] == 0x4f) || (key[0] == 0x5b)) && key[1] == 0x41)
					return UP_KEY;
				if (((key[0] == 0x4f) || (key[0] == 0x5b)) && key[1] == 0x42)
					return DOWN_KEY;
				return 0;
			case '\x7f':
				if (*len == 0)
					return KEY_NONE;
				write(1, "\x08\x1b\x5b\x4b", 4);
				(*len)--;
				return BACKSPACE_KEY;
		}
		*s++ = c;
		(*len)++;
		write(1, &c, 1);
		if (*len >= 255)
			return KEY_ENTER;
	}
}

pipe_t fs_open_pipe(const char *path)
{
	pipe_t pipe_fd;

	if (mkfifo(path, 0600) == -1) {
		perror("mkfifo");
		return -1;
	}

	pipe_fd = open(path, O_RDWR | O_NONBLOCK, 0);
	if (pipe_fd == -1)
		perror("open");
	return (pipe_fd);
}


void disk_encryption_cryptsetup()
{
	system("cryptsetup -y luksFormat -c aes -s 512 -h sha256 /dev/loop000");
	system("cryptsetup luksOpen /dev/loop000 db/crypto.fs");
	system("mkfs.ext2 /dev/mapper/cryptofs");
	system("e2fsck -f /dev/mapper/cryptofs");
	system("mkdir db/cryptofs");
	system("mount /dev/mapper/cryptofs db/cryptofs");
	system("mkdir db/cryptofs");
	system("mount /dev/mapper/cryptofs db/cryptofs");
}

void cryptofs_mount()
{
	system("losetup /dev/loop000 db/crypto.fs");
	system("cryptsetup luksOpen /dev/loop6 cryptofs");
	system("mount /dev/mapper/cryptofs db/cryptofs");
}

static void init_db_crypto(void)
{
	struct config *config;    // disk_encryption arguments
	char cmd[512];
	char cryptofs_path[280];
	char *stockminer_path;

	if (!(config=config_get("disk_encryption", 0, NULL, NULL)))
		return;

	if (!(config_get("stockminer_path", CONF_TYPE_STR, &stockminer_path, NULL)))
		exit(-1);

	snprintf(cryptofs_path, sizeof(cryptofs_path)-1, "%s/db/crypto.fs", stockminer_path);

	if (fs_file_exists(cryptofs_path)) {
		cryptofs_mount();
		return;
	}

	snprintf(cmd, sizeof(cmd)-1, "losetup /dev/loop6 %s", cryptofs_path);
	system(cmd);

 	system("losetup /dev/loop6 db/crypto.fs");
	system("cryptsetup luksOpen /dev/loop6 cryptofs");
	system("mount /dev/mapper/cryptofs build/cryptofs");
}

void init_paths()
{
	struct utsname u;
	char *home = getenv("HOME");
	char homepath[256];
	char procpath[256];
	char exepath[256];
	bool relative = false;

	if (!home)
		exit(-1);

	snprintf(procpath, sizeof(procpath)-1, "/proc/%d/exe", getpid());
	readlink(procpath, exepath, sizeof(exepath)-1);
	if (strstr(procpath, "/usr/local/stockminer/bin")) {
		relative = false;
	} else if (fs_file_exists("src/c")) {
		relative = true;
	} else {
		snprintf(homepath, sizeof(homepath)-1, "%s/.config/stockminer", home);
		char buf[512];
		snprintf(buf, sizeof(buf)-1, "chdir: %s\n", homepath);
		chdir(homepath);
		relative = true;
	}

	if (relative) {
		DB_PATH           = RELATIVE_DB_PATH;
		DB_REPO_PATH      = RELATIVE_DB_REPO_PATH;
		DB_USERS_PATH     = RELATIVE_DB_USERS_PATH;
		DB_LOG_PATH       = RELATIVE_DB_LOG_PATH;
		STOCKS_PATH       = RELATIVE_STOCKS_PATH;
		STOCKS_DIR_PATH   = RELATIVE_STOCKS_DIR_PATH;
		STOCKS_WEEKS_PATH = RELATIVE_STOCKS_WEEKS_PATH;
		STOCKDB_PATH      = RELATIVE_STOCKDB_PATH;
		STOCKDB_CSV_PATH  = RELATIVE_STOCKDB_CSV_PATH;
		GSPC_PATH         = RELATIVE_GSPC_PATH;
		OPTIONS_PATH      = RELATIVE_OPTIONS_PATH;
		truncate(DB_LOG_PATH, 0);
		return;
	}

	uname(&u);
	if (strcmp(u.sysname, "Linux")) {
		DB_PATH           = UNIX_DB_PATH;
		DB_USERS_PATH     = UNIX_DB_USERS_PATH;
		DB_REPO_PATH      = UNIX_DB_REPO_PATH;
		DB_LOG_PATH       = UNIX_DB_LOG_PATH;
		STOCKS_PATH       = UNIX_STOCKS_PATH;
		STOCKS_DIR_PATH   = UNIX_STOCKS_DIR_PATH;
		STOCKS_WEEKS_PATH = UNIX_STOCKS_WEEKS_PATH;
		STOCKDB_PATH      = UNIX_STOCKDB_PATH;
		STOCKDB_MAG2_PATH = UNIX_STOCKDB_MAG2_PATH;
		STOCKDB_MAG3_PATH = UNIX_STOCKDB_MAG3_PATH;
		STOCKDB_MAG4_PATH = UNIX_STOCKDB_MAG4_PATH;
		STOCKDB_CSV_PATH  = UNIX_STOCKDB_CSV_PATH;
		GSPC_PATH         = UNIX_GSPC_PATH;
		OPTIONS_PATH      = UNIX_OPTIONS_PATH;
		truncate(DB_LOG_PATH, 0);
	}
}

static void init_fs()
{
	if (!fs_file_exists((char *)DB_PATH) || !fs_file_exists((char *)DB_USERS_PATH)) {
		fs_mkdir("db",            0755);
		fs_mkdir("db/users",      0755);
		fs_mkdir("db/quadverse",  0755);
		fs_mkdir("db/watchlists", 0755);
		fs_mkdir("db/uid",        0755);
		fs_mkdir("db/uid/free",   0755);
		fs_mkdir("db/uid/0",      0755);
	}

	if (!fs_file_exists("data/stocks/stockdb/csv")) {
		fs_mkdir("data/stocks/stockdb",         0755);
		fs_mkdir("data/stocks/stockdb/csv",     0755); // yahoo CSV
		fs_mkdir("data/stocks/stockdb/wsj",     0755); // WSJ 1M ticks or today (or previous day if market==NO_MARKET)
		fs_mkdir("data/stocks/stockdb/options", 0755); // yahoo Options + CBOE Options (eventually)
		fs_mkdir("data/stocks/stockdb/mag2",    0755); // data generated by colgen.py
		fs_mkdir("data/stocks/stockdb/mag3",    0755); // data generated by indi.py (python's "ta" package)
		fs_mkdir("data/stocks/stockdb/mag4",    0755); // data generated by internal C code in src/c/stocks...it seems to be missing
	}
}

static void linux_detach()
{
	pid_t sid;
	char  cwd[256];
	char  buf[256];

	getcwd(cwd, sizeof(cwd));
	signal(SIGCHLD,child_handler);
	signal(SIGUSR1,child_handler);
	signal(SIGALRM,child_handler);
	switch (fork()) {
		case 0:
			break;
		default:
			exit(0);
	}

	signal(SIGCHLD,SIG_DFL);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM,SIG_DFL);

	sid = setsid();
	if (sid < 0) {
		perror("setsid");
		exit(-1);
	}
	chdir(cwd);
	freopen( "/dev/null", "r", stdin);
	freopen( "/dev/null", "w", stdout);
	freopen( "/dev/null", "w", stderr);
}

void os_exec_argv(char *argv[])
{
	const char *envp[] = { "PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin", NULL };
	int status;
	int pid;
	switch ((pid=fork())) {
		case 0:
			execve(argv[0], argv, (char **)envp);
			exit(-1);
		default:
			waitpid(pid, &status, 0);
	}
}

void os_exec(char *command)
{
	const char *envp[] = { "PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin", NULL };
	char       *argv[512];
	int         status, pid, argc;

	argc = cstring_split(command, argv, 511, ' ');
	if (!argc)
		return;
	argv[argc] = NULL;

	switch ((pid=fork())) {
		case 0:
			execve(argv[0], argv, (char **)envp);
			exit(0);
		default:
			waitpid(pid, &status, 0);
	}
}

static void *trace_monitor(void *args)
{
	while (waitpid(-1, 0, WNOHANG) > 0) { }
	exit(0);
}

static void init_daemon()
{
	pthread_t thr;
	int ppid, status, tracer_pid;
	
	linux_detach();
	return;

	// died
	tracer_pid = fork();
	if (tracer_pid == 0) {
		sleep(1);
		ppid = getpid()+1;
		ptrace(PTRACE_ATTACH, ppid, NULL, PTRACE_NULL);
		waitpid(ppid, &status, 0);
		ptrace(PTRACE_CONT, ppid, NULL, PTRACE_NULL);
		while (waitpid(ppid, &status, 0)) {
			if (WIFSTOPPED(status))
				ptrace(PTRACE_CONT, ppid, NULL, PTRACE_NULL);
			else
				exit(0);
		}
	} else {
		linux_detach();
		pthread_create(&thr, NULL, trace_monitor, NULL);
		ptrace(PTRACE_ATTACH, tracer_pid, NULL, PTRACE_NULL);
		ptrace(PTRACE_CONT,   tracer_pid, NULL, PTRACE_NULL);
	}
}

static void init_synchronization(struct server *server)
{

}

void init_os(struct server *server)
{
	struct sigaction sh;
	struct sigaction osh;
	struct rlimit    rlim;

	server->nr_vcpu = sysconf(_SC_NPROCESSORS_CONF);

	rlim.rlim_cur   = 262144;
	rlim.rlim_max   = 262144;
	setrlimit(RLIMIT_NOFILE, &rlim);
	rlim.rlim_cur   = 16384 KB;
	rlim.rlim_max   = 16384 KB;
	setrlimit(RLIMIT_STACK,  &rlim);
	setenv("TZ", ":/etc/localtime", 0);

	sh.sa_handler = SIG_IGN;
	sh.sa_flags   = SA_RESTART;
	sigemptyset(&sh.sa_mask);
	if (sigaction(SIGPIPE, &sh, &osh) < 0)
		exit(-1);

	signal(SIGHUP, server_restart);
	init_fs();
	init_synchronization(server);
	if (server->daemon)
		init_daemon();
}
#endif
