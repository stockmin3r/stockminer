#ifndef __WIN_H
#define __WIN_H

#include <stdinc.h>

#define os_dlopen(path)       LoadLibrary((char *)path)
#define os_dlsym(handle, sym) (void *)GetProcAddress(handle, sym)
#define os_dlclose(handle)    FreeLibrary(handle)
#define os_mkdir(name,access) CreateDirectory((const char *)name, NULL)
#define SHUT_WR               SD_SEND
#define SHUT_RD               SD_RECV
#define SHUT_RDWR             SD_BOTH
#define MSG_NOSIGNAL          0
#define MSG_DONTWAIT          0
#define SOCK_CLOEXEC          0 /* O_NOINHERIT */
#define FD_CLOEXEC            O_NOINHERIT
#define O_CLOEXEC             O_NOINHERIT
#define mutex_t               HANDLE
#define SETSOCKOPT(a,b,c,d,e)
#define accept4(a,b,c,d)      accept(a,b,c)
#define gettimeofday(x,y)     nm_gettimeofday(x)
#define ftruncate             _chsize_s
#define open                  _open
#define stat                  xstat
#define xstat                 __stat64
#define fstat                 _fstat64
#define os_getpid             GetCurrentProcessId
#define net_socket_errno      WSAGetLastError()
#define net_socket(x,y,z)     WSASocket(x,y,z,NULL,0,0)

typedef SOCKET socket_t;
typedef int    socklen_t;

struct filemap {
	HANDLE         hMap;     /* Windows CreateFileMapping Handle */
	HANDLE         hFile;    /* Windows CreateFile Handle        */
	uint64_t       filesize; /* filesize */
	int            fd;       /* osh fd */
};

struct dirmap {
	HANDLE           dir;
	WIN32_FIND_DATA  dirent;
	FILETIME         creationTime;
	FILETIME         lastAccessTime;
};

static __inline__ int fs_stat(char *path, struct xstat *sb)
{
	return _stat64(path, sb);
}

static __inline__ int fs_opendir(char *path, struct dirmap *dirmap)
{
	if (!(dirmap->dir=FindFirstFile((const char *)path, &dirmap->dirent)))
		return 0;
	return 1;
}

static __inline__ char *fs_readdir(struct dirmap *dirmap)
{
	if (!FindNextFile(dirmap->dir, &dirmap->dirent))
		return 0;
	if (dirmap->dirent.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		dirmap->filetype = FILETYPE_DIRECTORY;
	return (char *)dirmap->dirent.cFileName;
}

static __inline__ void fs_closedir(struct dirmap *dirmap)
{
	FindClose(dirmap->dir);
}

static __inline__ void fs_usleep(int microseconds)
{
	Sleep(microseconds / 1000);
}

static __inline__ uint64_t fs_filesize(char *filename)
{
	LARGE_INTEGER liFileSize;
}

static __inline__ struct tm *localtime_r(time_t *utc_time, struct tm *result)
{
	localtime_s(result, utc_time);
	return (result);
}

static __inline__ char *ctime_r(time_t *utc_time, char *timestr)
{
	ctime_s(timestr, 32, utc_time);
	return (timestr);
}

static __inline__ char *asctime_r(struct tm *tm, char *timestr)
{
	asctime_s(timestr, 32, tm);
	return (timestr);
}

static __inline__ int fs_truncate(char *path, uint64_t size)
{
	int fd = _open(path, O_RDWR);
	if (fd < 0)
		return -1;
	_chsize_s(fd, size);
	_close(fd);
}

static __inline__ int fs_file_exists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void *memmem              (void *haystack, unsigned long h_size, void *needle, unsigned long n_size);
char *inet_ntop           (int af_family, const void *src, char *dst, socklen_t size);
int   inet_pton           (int af, const char *src, void *dst);
char *strptime            (char *buf, char *fmt, struct tm *tm);
void  os_sleep            (int seconds);
void  mutex_lock          (HANDLE mtx);
void  mutex_unlock        (HANDLE mtx);

#endif
