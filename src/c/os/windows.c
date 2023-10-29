#include <stdinc.h>
#include <conf.h>

#ifdef __WINDOWS__

void init_os(struct config *args)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
}

void thread_create(void *(*thread)(void *), void *args)
{
	void *TID;
	TID = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread, args, 0, NULL);
}
void mutex_lock(mutex_t mtx)
{
	WaitForSingleObject(mtx, INFINITE);
}
void mutex_unlock(mutex_t mtx)
{
	ReleaseMutex(mtx);
}

void *memmem(void *haystack, unsigned long h_size, void *needle, unsigned long n_size)
{
	unsigned long i,j=0;
	for (i=0;i<h_size;i++) {
		if (j==n_size)
			return (haystack+i-1);
		for (j=0;j<n_size;j++)
			if (*(unsigned char *)(haystack+i+j)!=*(unsigned char *)(needle+j))
				break;
	}
	return (NULL);
}

int fs_open(char *path, struct filemap *filemap, int flags)
{
	struct stat sb;
	int fd = open(path, flags, 0644);
	if (fd <= 0)
		return -1;
	fstat(fd, &sb);
	filemap->filesize = sb.st_size;
	filemap->fd = fd;
	return fd;
}

void *MAP_FILEMAP_RW(struct filemap *filemap)
{
	HANDLE hFile = (HANDLE)_get_osfhandle(filemap->fd);
	HANDLE hMap;
	char *lpBasePtr;

	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

    hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hMap == 0) {
        CloseHandle(hFile);
        return NULL;
    }

	lpBasePtr = MapViewOfFile(hMap,FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
    if (lpBasePtr == NULL) {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return NULL;
    }
	filemap->hMap     = hMap;
	filemap->hFile    = hFile;
	return (lpBasePtr);
}

char *MAP_FILE_RO(char *path, struct filemap *filemap)
{
    HANDLE        hFile;
    HANDLE        hMap;
    LPVOID        lpBasePtr;
	LARGE_INTEGER liFileSize;

    hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE) {
		printf("createfile error\n");
		return NULL;
	}

    if (!GetFileSizeEx(hFile, &liFileSize)) {
        CloseHandle(hFile);
        return NULL;
    }

    if (liFileSize.QuadPart == 0) {
        CloseHandle(hFile);
        return NULL;
    }

    hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap == 0) {
        CloseHandle(hFile);
        return NULL;
    }

	lpBasePtr = MapViewOfFile(hMap,FILE_MAP_READ, 0, 0, 0);
    if (lpBasePtr == NULL) {
s        CloseHandle(hMap);
        CloseHandle(hFile);
        return NULL;
    }
	filemap->hMap     = hMap;
	filemap->hFile    = hFile;
	filemap->filesize = liFileSize.QuadPart;
	return ((char *)lpBasePtr);
}

char *MAP_FILE_RW(char *path, struct filemap *filemap)
{
    HANDLE        hFile;
    HANDLE        hMap;
    LPVOID        lpBasePtr;
	LARGE_INTEGER liFileSize;

    hFile = CreateFile(path, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

    if (!GetFileSizeEx(hFile, &liFileSize)) {
        CloseHandle(hFile);
        return NULL;
    }

    if (liFileSize.QuadPart == 0) {
        CloseHandle(hFile);
        return NULL;
    }

    hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hMap == 0) {
        CloseHandle(hFile);
        return NULL;
    }

	lpBasePtr = MapViewOfFile(hMap,FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
    if (lpBasePtr == NULL) {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return NULL;
    }
	filemap->hMap     = hMap;
	filemap->hFile    = hFile;
	filemap->filesize = liFileSize.QuadPart;
	return ((char *)lpBasePtr);
}

void UNMAP_FILE(char *lpBasePtr, struct filemap *filemap)
{
	UnmapViewOfFile(lpBasePtr);
	CloseHandle(filemap->hMap);
	CloseHandle(filemap->hFile);
}

void os_sleep(int seconds)
{
	Sleep(seconds * 1000);
}

char *inet_ntop(int af_family, const void *src, char *dst, socklen_t size)
{
	struct sockaddr_storage ss;
	unsigned long sz = size;

	memset(&ss, 0, sizeof(ss));
	ss.ss_family = af_family;
	switch(af_family) {
		case AF_INET:
			((struct sockaddr_in *)&ss)->sin_addr   = *(struct in_addr *)src;
			break;
		case AF_INET6:
			((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
			break;
		default:
			return NULL;
	}
	return (WSAAddressToString((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &sz) == 0) ? dst : NULL;
}

int inet_pton(int af, const char *src, void *dst)
{
	struct sockaddr_storage ss;
	int size = sizeof(ss);
	char src_copy[INET6_ADDRSTRLEN+1];

	memset(&ss, 0, sizeof(ss));
	strncpy (src_copy, src, INET6_ADDRSTRLEN);
	src_copy[INET6_ADDRSTRLEN] = 0;

	if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
		switch(af) {
			case AF_INET:
				*(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
				return 1;
			case AF_INET6:
				*(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
				return 1;
		}
	}
	return 0;
}

void net_socket_block(socket_t sockfd)
{
    uint64_t nb = 1;
    ioctlsocket(sockfd, FIONBIO, &nb);
}
void net_socket_nonblock(socket_t sockfd)
{
    uint64_t nb = 0;
    ioctlsocket(sockfd, FIONBIO, &nb);
}

void os_gettimeofday(struct timeval *tv)
{
	uint64_t  intervals;
	FILETIME  ft;

	GetSystemTimeAsFileTime(&ft);
	intervals   = ((uint64_t) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	intervals  -= 116444736000000000;
	tv->tv_sec  = (long) (intervals / 10000000);
	tv->tv_usec = (long) ((intervals % 10000000) / 10);
}

void os_exec(char *path, char *argv[])
{


}

#endif
