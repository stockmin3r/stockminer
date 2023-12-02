#ifndef __UNIX_H
#define __UNIX_H

#include <stdinc.h>

#define __SYS_gettid          186
#define __SYS_PRCTL           38
#define sys_gettid()          syscall(__SYS_gettid)
#define sys_prctl()           syscall(__SYS_PRCTL)

#define os_dlopen(path)       dlopen((char *)path, RTLD_NOW)
#define os_dlsym(handle, sym) dlsym(handle, sym)
#define os_dlclose(handle)    dlclose(handle)
#define os_sleep              sleep
#define os_getpid             getpid
#define fs_mkdir(name,access) mkdir((const char *)name, access)
#define fs_truncate           truncate
#define SETSOCKOPT            setsockopt
#define xstat                 stat
#define mutex_t               pthread_mutex_t
#define mutex_lock(mtx)       pthread_mutex_lock(mtx)
#define mutex_unlock(mtx)     pthread_mutex_unlock(mtx)
#define net_socket_errno      errno
#define fs_pipe_write         write
#define fs_pipe_read          read
#define FILETYPE_DIRECTORY    DT_DIR

extern struct termios sterm;

#define KEY_ENTER      0
#define UP_KEY        -2
#define DOWN_KEY      -3
#define BACKSPACE_KEY -4
#define KEY_NONE      -5

#define ADMIN_PIPE_PATH "/var/run/stockminer/admin.fifo"

#ifdef __LINUX__
#define PTRACE_NULL NULL
#endif

#if defined __LINUX__ || defined __BSD__ || defined __OSX__
#define net_socket socket
typedef int        socket_t;
typedef int        event_fd_t;
typedef int        pipe_t;
#endif

#if defined __BSD__ || defined __OSX__
#define PTRACE_CONT   PT_CONTINUE
#define PTRACE_ATTACH PT_ATTACH
#define PTRACE_NULL   0
#endif

#ifdef __OSX__
#define SOCK_CLOEXEC          0 /* unused */
#define MSG_NOSIGNAL          0 /* unused */
#define net_socket(domain,type,protocol)         \
({                                               \
    int __sockfd = socket(domain,type,protocol); \
    fcntl(sockfd,F_SETFD,FD_CLOEXEC);            \
    return (sockfd);                             \
})
#endif

struct filemap {
	char          *map;      /* memory address */
	uint64_t       filesize; /* filesize */
	int            fd;       /* fd */
};

struct dirmap {
	DIR           *dir;
	struct dirent *dirent;
	char          *dirname;
	int            filetype;
};

void   term_reset      (void);
void   term_erase      (int n);
void   term_noecho     (void);
int    fs_readline_tty (char *s, int *len);
pipe_t fs_open_pipe    (const char *path);
void   init_paths      (void);

static __inline__ int fs_stat(char *path, struct xstat *sb)
{
	return stat(path, sb);
}

static __inline__ int fs_opendir(char *path, struct dirmap *dirmap)
{
	if (!(dirmap->dir=opendir((const char *)path)))
		return 0;
	return 1;
}

static __inline__ char *fs_readdir(struct dirmap *dirmap)
{
	if (!(dirmap->dirent=readdir(dirmap->dir)))
		return NULL;
	dirmap->filetype = dirmap->dirent->d_type;
	return dirmap->dirent->d_name;
}

static __inline__ void fs_closedir(struct dirmap *dirmap)
{
	closedir(dirmap->dir);
}

static __inline__ void os_usleep(int microseconds)
{
	usleep(microseconds);
}

static __inline__ uint64_t fs_filesize(char *filename)
{
	struct stat sb;
	if (stat(filename, &sb) == -1)
		return 0;
	return sb.st_size;
}

static __inline__ bool fs_file_exists(char *path)
{
	struct stat sb;

	if (stat(path, &sb) == -1)
		return false;
	return true;
}

#endif
