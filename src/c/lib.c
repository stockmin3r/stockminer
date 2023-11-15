#include <stdinc.h>
#include <conf.h>

/* **********************
 *
 *     SYNCHRONOUS FS
 *
 ***********************/
int64_t fs_write(int fd, void *buf, int64_t count)
{
	char *p = (char *)buf;
	int   i;

	do {
		i = write(fd, p, count);
		if (i == 0) {
			return -1;
		} else if (i == -1) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		count -= i;
		p     += i;
	} while (count > 0);
	return (p-(char *)buf);
}

void fs_writefile(char *path, char *file, int64_t len)
{
	int fd;

	fd = open(path, O_RDWR|O_CREAT, 0644);
	if (fd < 0)
		return;
	write(fd, file, len);
	close(fd);
}

void fs_newfile(char *path, void *file, int64_t len)
{
	int fd;

	fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (fd < 0)
		return;
	write(fd, file, len);
	close(fd);
}

int64_t fs_read(int fd, char *buf, int64_t maxlen)
{
	int64_t count, total = 0;

	while (1) {
		count = read(fd, buf+total, maxlen);
		if (count == -1 && errno == EAGAIN)
			continue;
		if (count <= 0)
			break;
		total  += count;
		maxlen -= count;
		if (maxlen <= 0)
			break;
	}
	return (total);
}

int fs_readline(char *s, int fd)
{
	int  n, len = 0;
	char c      = 0;

	while (1) {
		n = read(fd, &c, 1);
		if (n <= 0)
			return -1;
		if (c == '\n') {
			*s = '\0';
			return (len);
		} else {
			*s++ = c;
			len++;
		}
	}
}

int64_t fs_readfile(char *path, char *buf, int64_t max)
{
	struct stat sb;
	int64_t     nbytes;
	int         fd;

	if (max <= 0)
		return 0;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	fstat(fd, &sb);
	nbytes = read(fd, buf, MIN(sb.st_size,max-1));
	close(fd);
	return (nbytes);
}

int64_t fs_readfile_str(char *path, char *buf, int64_t max)
{
	struct stat sb;
	int64_t    nbytes;
	int         fd;

	if (max <= 0)
		return 0;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	fstat(fd, &sb);
	nbytes = read(fd, buf, MIN(sb.st_size,max-1));
	if (nbytes <= 0)
		return 0;
	buf[nbytes] = 0;
	close(fd);
	return (nbytes);
}

char *fs_mallocfile(char *path, int64_t *filesize)
{
	struct stat sb;
	char       *file;
	int         fd;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		goto out_err;
	fstat(fd, &sb);
	if (!sb.st_size)
		goto out_err;

	file = (char *)malloc(sb.st_size);
	if (!file)
		goto out_err;
	if (read(fd, file, sb.st_size) != sb.st_size)
		goto out_err;
	if (filesize)
		*filesize = sb.st_size;
	close(fd);
	return (file);
out_err:
	if (filesize)
		*filesize = 0;
	return NULL;
}

char *fs_mallocfile_str(char *path, int64_t *filesize)
{
	struct stat sb;
	char       *file;
	int         fd;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return NULL;
	fstat(fd, &sb);
	if (!sb.st_size)
		return NULL;

	file = (char *)malloc(sb.st_size+1);
	if (!file)
		return NULL;
	read(fd, file, sb.st_size);
	if (filesize)
		*filesize = sb.st_size;
	file[sb.st_size] = 0;
	close(fd);
	return (file);
}

void fs_appendfile(char *path, void *file, int64_t filesize)
{
	int fd;

	fd = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
	if (fd < 0)
		return;
	write(fd, file, filesize);
	close(fd);
}

void fs_appendfile_nl(char *path, char *file, int64_t filesize)
{
	int fd;

	fd = open(path, O_RDWR|O_CREAT|O_APPEND, 0644);
	if (fd < 0)
		return;
	write(fd, file, filesize);
	write(fd, "\n", 1);
	close(fd);
}

int64_t fs_readfile_str_mtime(char *path, char *buf, time_t *mtime)
{
	struct stat sb;
	int64_t    nbytes;
	int         fd;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	fstat(fd, &sb);
	*mtime      = sb.st_mtime;
	nbytes      = read(fd, buf, sb.st_size);
	buf[nbytes] = 0;
	close(fd);
	return (nbytes);
}

void fs_printfile(char *path)
{
	char *file = fs_mallocfile_str(path, NULL);

	if (!file)
		return;
	printf("%s\n", file);
	free(file);
}

void fs_copy_file(char *src_path, char *dst_path)
{
	char     src_buf[128 KB];
	int64_t filesize;
	int      fd;

	filesize = fs_readfile(src_path, src_buf, sizeof(src_buf));
	if (!filesize)
		return;
	fd       = open(dst_path, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (fd < 0)
		return;
	write(fd, src_buf, filesize);
	close(fd);
}

void fs_copy_big_file(char *src_path, char *dst_path)
{
	char    *file;
	int64_t filesize;
	int      fd;

	file = fs_mallocfile(src_path, &filesize);
	if (!file)
		return;
	fd   = open(dst_path, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (fd < 0)
		return;
	fs_write(fd, (char *)file, filesize);
	close(fd);
}

int fs_line_count(char *filename)
{
	struct filemap filemap;
	char *map, *line, *p;
	int count = 0;

	map = MAP_FILE_RO(filename, &filemap);
	if (!map)
		return 0;
	line = map;
	while ((p=strchr(line, '\n'))) {
		count++;
		line = p + 1;
	}
	UNMAP_FILE(map, &filemap);
	return (count);
}

/* unused */
char **fs_list_directory(char *directory, int *nr_files_out)
{
	struct dirmap  dirmap;
	char         **files, *filename;
	int            nr_files = 0, max_files = 32;

	if (!fs_opendir(directory, &dirmap))
		return NULL;

	files = (char **)malloc(sizeof(char *) * max_files);
	if (!files)
		return NULL;

	while ((filename=fs_readdir(&dirmap)) != NULL) {
		if (*filename == '.')
			continue;
		files[nr_files++] = strdup(filename);
		REALLOC(files, char **, nr_files, max_files);
	}

	if (nr_files_out)
		*nr_files_out = nr_files;
	return (files);
}

/* ****************************
 *
 *     SYNCHRONOUS NETWORKING
 *
 *****************************/
int net_select_waitfd(int sockfd, int seconds)
{
	fd_set fdset;
	struct timeval timeout;

	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);
	timeout.tv_sec  = seconds;
	timeout.tv_usec = 0;
	select(sockfd+1, &fdset, NULL, NULL, &timeout);
	if (!FD_ISSET(sockfd, &fdset))
		return 0;
	return 1;
}

socket_t net_tcp_bind(uint32_t bind_addr, unsigned short port)
{
	struct sockaddr_in serv;
	int sockfd, val = 1;

	sockfd = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_TCP);
	if (sockfd < 0)
		return -1;

	setsockopt(sockfd, SOL_SOCKET,  SO_REUSEADDR, (const char *)&val, sizeof(val));
	SETSOCKOPT(sockfd, SOL_SOCKET,  SO_REUSEPORT, (const char *)&val, sizeof(val));
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,  (const char *)&val, sizeof(val));

	serv.sin_family      = AF_INET;
	serv.sin_port        = htons(port);
	serv.sin_addr.s_addr = bind_addr;
	if (bind(sockfd, (struct sockaddr *)&serv, 0x10) < 0) {
		close(sockfd);
		return -1;
	}
	if (listen(sockfd, 15) < 0) {
		close(sockfd);
		return -1;
	}
	return (sockfd);
}

int net_tcp_connect(const char *dst_addr, unsigned short dst_port)
{
	struct sockaddr_in paddr;
	int dst_fd;

	dst_fd = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_TCP);
	if (dst_fd < 0) {
		perror("socket");
		return -1;
	}
	paddr.sin_family      = AF_INET;
	paddr.sin_port        = htons(dst_port);
	paddr.sin_addr.s_addr = inet_addr(dst_addr);
	if (connect(dst_fd, (struct sockaddr *)&paddr, 0x10) < 0) {
		close(dst_fd);
		return -1;
	}
	return (dst_fd);
}

int net_tcp_connect2(unsigned int dst_addr, unsigned short dst_port)
{
	struct sockaddr_in paddr;
	int dst_fd;

	dst_fd = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_TCP);
	if (dst_fd < 0) {
		perror("socket");
		return -1;
	}
	paddr.sin_family      = AF_INET;
	paddr.sin_port        = htons(dst_port);
	paddr.sin_addr.s_addr = dst_addr;
	if (connect(dst_fd, (struct sockaddr *)&paddr, 0x10) < 0) {
		close(dst_fd);
		return -1;
	}
	return (dst_fd);
}

int net_tcp_connect_host(const char *hostname, int port)
{
	struct in_addr     **addr_list;
	struct in_addr      *addr;
	struct hostent      *hp;
	struct sockaddr_in   serv;
	int                  sockfd;

	hp = gethostbyname(hostname);
	if (!hp)
		return -1;
	addr_list            = (struct in_addr **)hp->h_addr_list;
	addr                 = addr_list[0];
	sockfd               = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_TCP);
	serv.sin_family      = AF_INET;
	serv.sin_port        = htons(port);
	serv.sin_addr.s_addr = addr->s_addr;
	if (connect(sockfd, (struct sockaddr *)&serv, 16) < 0) {
		close(sockfd);
		return -1;
	}
	return (sockfd);
}

int net_tcp_connect_timeout(unsigned int ip, int port, int seconds)
{
	struct timeval     timeout;
	struct sockaddr_in serv;
	fd_set             rdset, wdset, edset;
	int                sockfd;

	sockfd               = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC, IPPROTO_TCP);
	serv.sin_family      = AF_INET;
	serv.sin_port        = htons(port);
	serv.sin_addr.s_addr = ip;

	net_socket_nonblock(sockfd);
	if (connect(sockfd, (struct sockaddr *)&serv, 16) < 0) {
		if ((errno != 0) && (errno != EINPROGRESS))
			return -1;
	}
	FD_ZERO(&rdset);
	FD_ZERO(&wdset);
	FD_ZERO(&edset);
	FD_SET(sockfd, &rdset);
	FD_SET(sockfd, &wdset);
	FD_SET(sockfd, &edset);
	timeout.tv_sec  = seconds;
	timeout.tv_usec = 0;
	net_socket_block(sockfd);
	select(sockfd+1, &rdset, &wdset, &edset, &timeout);
	return (sockfd);
}

int64_t net_send(int fd, void *buf, int64_t count)
{
	char *p = (char *)buf;
	int   i;

	do {
		i = send(fd, p, count, MSG_NOSIGNAL);
		if (i == 0)
			return -1;
		if (i == -1) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		count -= i;
		p     += i;
	} while (count > 0);
	return (p-(char *)buf);
}

int64_t net_recv(int sockfd, char *buf, int64_t size, int timeout)
{
	int64_t nbytes, count = 0;

	while (1) {
		nbytes = recv(sockfd, buf+count, size, MSG_DONTWAIT);
		if (nbytes < 0) {
			if (errno == EAGAIN) {
				if (!net_select_waitfd(sockfd, timeout))
					break;
				continue;
			}
			return 0;
		}
		if (nbytes <= 0)
			return (count);
		size  -= nbytes;
		count += nbytes;
		if (size <= 0)
			break;
		if (!net_select_waitfd(sockfd, timeout))
			break;
	}
	return (count);
}

in_addr_t net_get_hostname(char *hostname)
{
	struct hostent *hp = gethostbyname(hostname);
	struct in_addr ipaddr;
	if (!hp)
		return 0;
	ipaddr = *(struct in_addr *)(hp->h_addr_list[0]);
	return ipaddr.s_addr;
}

/* *****************
 *
 *     CSTRINGS
 *
 *****************/
char *cstring_inject(char *target, char *payload, char *pattern, int *output_size)
{
	char *pPattern     = strstr(target, pattern);
	char *output       = NULL;
	int   payload_len  = strlen(payload);
	int   target_len   = strlen(target);
	int   pattern_len  = strlen(pattern);
	int   pattern_off  = (pPattern-target);
	int   leftover_len = (target_len-pattern_off);
	int   output_len   = (target_len+payload_len+leftover_len+1);

	if (!pPattern || !payload_len || !target_len || !pattern_off || output_len <= 0)
		return NULL;

//	printf("payload_len: %d target_len: %d pattern_off: %d leftover: %d output_len: %d\n", payload_len, target_len, pattern_off, leftover_len, output_len);
	output = malloc(output_len+1);
	if (!output)
		return NULL;

	memcpy(output, target, pattern_off);
	memcpy(output+pattern_off, payload, payload_len);
	memcpy(output+pattern_off+payload_len, pPattern+pattern_len, leftover_len);
	output[output_len] = 0;
	if (output_size)
		*output_size = output_len;
	return output;
}

void cstring_strstr_replace(char *str, char *pattern)
{
	char *p, *endp;
	int pattern_len = strlen(pattern), string_len = strlen(str);

	endp = str + string_len;
	while ((p=strstr(str, pattern)) != NULL) {
		memmove(p, p+pattern_len, endp-p-pattern_len);
		str[string_len-pattern_len] = 0;
		str         = p + 1;
		endp       -= pattern_len;
		string_len -= pattern_len;
	}
}

void cstring_strchr_replace(char *str, char replace_char, char to_char)
{
	char *p;
	while ((p=strchr(str, replace_char))) {
		*p++ = to_char;
		str  = p;
	}
}

int64_t cstring_remove_line(char *text, int lineno, int64_t textsize)
{
	char    *newstart, *p;
	char    *line = text;
	int64_t  newsize;
	int      x;

	if (textsize <= 0)
		textsize = strlen(text);

	newstart = line;
	for (x=0; x<lineno; x++) {
		p = strchr(line, '\n');
		if (!p)
			return 0;
		newstart = line;
		line = p + 1;
	}
	newsize = textsize-(line-newstart);
	memmove(newstart, line, newsize);
	text[newsize] = 0;
	return (newsize);
}

int cstring_itoa(char *s, uint64_t n)
{
	char buf[32] = {0};
	int i = sizeof(buf)-2;
	int rem = 0;
	do {
		rem = n%10;
		buf[i--] = (rem < 10) ? rem + '0' : rem + 'a' - 10;
	} while(n/=10);
	memcpy(s, &buf[i+1], sizeof(buf)-i-2);
	return (sizeof(buf)-i-1);
}

int cstring_split(char *msg, char *argv[], int max, char c)
{
	char *p, *arg = msg;
	int argc = 0;

	if (!msg || *msg == '\0')
		return 0;

	while ((p=strchr(arg, c))) {
		*p++ = 0;
		if (*p == '\0')
			break;
		argv[argc++] = arg;
		if (argc >= max)
			break;
		arg = p;
		argv[argc] = p;
	}
	if (argc == 0)
		argv[0] = msg;
	return (argc+1);
}

int64_t cstring_count_chars(char *str, char c)
{
	char    *p = str;
	int64_t  count = 0;

	while (*p != '\0') {
		if (*p++ == c)
			count++;
	}
	return (count);
}

int64_t cstring_line_count(char *str)
{
	char    *line, *p;
	int64_t  count = 0;

	line = str;
	while ((p=strchr(line, '\n'))) {
		count++;
		line = p+1;
	}
	return count;
}

void cstring_strip_char(char *ptr, char c)
{
	char *p = strchr(ptr, c);
	if (p)
		*p = 0;
}

char *memdup(char *mem, int64_t size)
{
	char *m = (char *)malloc(size);
	if (!m)
		return NULL;
	memcpy(m, mem, size);
	return (m);
}

void *zmalloc(int64_t size)
{
	void *ptr;

	if (size <= 0)
		return NULL;

	ptr = malloc(size);
	if (!ptr)
		return NULL;
	memset(ptr, 0, size);
	return (ptr);
}

/* *****************
 *
 *     RANDOM
 *
 *****************/
unsigned short random_short()
{
	int fd;
	unsigned short r;

	fd = open("/dev/urandom", O_RDONLY);
	read(fd, &r, 2);
	close(fd);
	return (r);
}

uint32_t random_int()
{
	int fd, r;

	fd = open("/dev/urandom", O_RDONLY);
	read(fd, &r, 4);
	close(fd);
	return (r);
}

uint64_t random_long()
{
	uint64_t r;
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	read(fd, &r, 8);
	close(fd);
	return (r);
}

double random_double()
{
	double r;
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	read(fd, &r, 8);
	close(fd);
	if (r < 0)
		return -r;
	return (r);
}

void random_string_len(char *str, int64_t len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
	if (len <= 0 || len >= 32 KB) {
		str[0] = 0;
		return;
	}
    for (int i = 0; i < len; ++i)
        str[i] = alphanum[random_int() % (sizeof(alphanum) - 1)];
    str[len] = 0;
}

void random_string(char *str)
{
    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";
    for (int i = 0; i < 7; ++i)
        str[i] = alphanum[random_int() % (sizeof(alphanum) - 1)];
    str[7] = 0;
}

void random_cookie2(char *cookie)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < 8; ++i)
        cookie[i] = alphanum[random_int() % (sizeof(alphanum) - 1)];
}

void random_string_int(char *str)
{
    static const char alphanum[] =
        "0123456789";
    for (int i = 0; i < 8; ++i)
        str[i] = alphanum[random_int() % (sizeof(alphanum) - 1)];
}

void random_cookie(char *cookie)
{
	unsigned char random_buf[COOKIE_SIZE];
	hydro_random_buf(random_buf, sizeof(random_buf));
	base64_encode(random_buf, 10, cookie);
	cookie[COOKIE_SIZE] = 0;
}

/* *********************
 *
 *     SORT ALGORITHMS
 *
 **********************/
void INSERTION_SORT(int array[], int n)
{
	int i, key, j;

	for (i = 1; i < n; i++) {
		key = array[i];
		j = i - 1;
		while (j >= 0 && array[j] > key) {
			array[j + 1] = array[j];
			j = j - 1;
		}
		array[j + 1] = key;
	}
}

void DOUBLE_SORT_LOHI(double array[], int n)
{
	int i, j;
	double key;

	for (i = 1; i < n; i++) {
		key = array[i];
		j = i - 1;
		while (j >= 0 && array[j] > key) {
			array[j + 1] = array[j];
			j = j - 1;
		}
		array[j + 1] = key;
	}
}

void DOUBLE_SORT_HILO(double array[], int n)
{
	int i, j;
	double key;

	for (i = 1; i < n; i++) {
		key = array[i];
		j = i - 1;
		while (j >= 0 && array[j] < key) {
			array[j + 1] = array[j];
			j = j - 1;
		}
		array[j + 1] = key;
	}
}
