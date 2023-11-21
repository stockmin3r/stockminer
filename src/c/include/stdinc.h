#ifndef __STDINC_H
#define __STDINC_H

//#pragma once

#if defined(__cplusplus)
extern "C" {
#endif


#undef __FreeBSD__
#undef __OpenBSD__
#undef __NetBSD__

#include <build.h>

#ifdef __Linux__
#define __LINUX__   1
#endif

#ifdef __Darwin__
#define __OSX__     1
#endif

#ifdef __FreeBSD__
#define __FREEBSD__ 1
#define __BSD__     1
#endif

#ifdef __OpenBSD__
#define __OPENBSD__ 1
#define __BSD__     1
#endif

#ifdef __LINUX__
	#ifndef _XOPEN_SOURCE
	#define _XOPEN_SOURCE
	#endif
	#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
	#endif
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#if defined __LINUX__ || defined __BSD__ || defined __OSX__
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/termios.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unix.h>
#endif

#ifdef __LINUX__
#include <sys/epoll.h>
#include <sys/inotify.h>
#endif

#ifdef __WINDOWS__
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <ws2tcpip.h>
#include <win.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

/* src/external */
#include <sodium.h>
#include <lmdb.h>
#include <queue.h>
#include <hydrogen.h>
#include <uthash.h>
#include <yyjson.h>

// to be removed
#include <curl/curl.h>


#define likely(x)         __builtin_expect(!!(x), 1)
#define unlikely(x)       __builtin_expect(!!(x), 0)
#define __section(S)      __attribute__((__section__(#S)))
#define __aligned(N)      __attribute__((__aligned__(N)))
#define __inline__ inline __attribute__((always_inline))

#ifndef uint64_t
#define uint64_t unsigned long long
#endif

#ifndef int64_t
#define int64_t long long
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef size_t
#define size_t uint64_t
#endif

typedef uint64_t packet_size_t;
typedef uint16_t port_t;          /* TCP/UDP port type */

#ifndef uid_t
typedef uint32_t uid_t;
#endif

#ifndef KB
#define KB * 1024
#endif

#ifndef MB
#define MB * 1024 * 1024
#endif

#ifndef MIN
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif

#if defined(__cplusplus)
}
#endif
#endif
