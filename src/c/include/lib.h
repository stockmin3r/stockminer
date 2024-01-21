#ifndef __LIB_H
#define __LIB_H

int64_t        fs_read                 (int, char *, int64_t);
int            fs_readline             (char *s, int fd);
int64_t        fs_readfile             (char *path, char *buf, int64_t max);
int64_t        fs_readfile_str         (char *path, char *buf, int64_t max);
int64_t        fs_write                (int fd, void *buf, int64_t count);
char          *fs_mallocfile           (char *path, int64_t *filesize);
char          *fs_mallocfile_str       (char *path, int64_t *filesize);
void           fs_writefile            (char *path, char *file, int64_t len);
void           fs_newfile              (char *path, void *file, int64_t len);
void           fs_appendfile           (char *path, void *file, int64_t filesize);
void           fs_appendfile_nl        (char *path, char *file, int64_t filesize);
int            fs_line_count           (char *filename);
void           fs_copy_file            (char *src, char *dst);
void           fs_copy_big_file        (char *src, char *dst);
void           fs_printfile            (char *path);
char         **fs_list_directory       (char *directory, int *nr_files_out);
int64_t        net_send                (int fd, void *buf, int64_t count);
int64_t        net_recv                (int sockfd, char *buf, int64_t size, int timeout);
socket_t       net_tcp_bind            (unsigned int bind_addr, unsigned short port);
int            net_tcp_connect         (const char *dst_addr,  unsigned short dst_port);
int            net_tcp_connect2        (unsigned int dst_addr, unsigned short dst_port);
int            net_tcp_connect_host    (const char *hostname, int port);
int            net_tcp_connect_timeout (unsigned int ip, int port, int seconds);
int            net_select_waitfd       (int, int);
in_addr_t      net_get_hostname        (char *hostname);
char          *cstring                 (char *str, char c, int pos);
char          *cstring_inject          (char *target, char *payload, char *pattern, int *output_size);
void           cstring_excise          (char *str, char *pattern);
int            cstring_itoa            (char *s, uint64_t n);
int64_t        cstring_remove_line     (char *text, int lineno, int64_t textsize);
int64_t        cstring_count_chars     (char *str, char c);
int64_t        cstring_line_count      (char *);
int            cstring_split           (char *msg, char *argv[], int max, char c);
void           cstring_strip_char      (char *ptr, char c);
void           cstring_strchr_replace  (char *str, char replace_char, char to_char);
char          *memdup                  (char *mem, int64_t size);
void          *zmalloc                 (int64_t size);
unsigned short random_short            (void);
uint32_t       random_int              (void);
uint64_t       random_long             (void);
double         random_double           (void);
void           random_string_len       (char *str, int64_t len);
void           random_string           (char *str);
void           random_string_int       (char *str);
void           random_cookie           (char *cookie);
void           INSERTION_SORT          (int array[], int n);
void           DOUBLE_SORT_LOHI        (double array[], int n);
void           DOUBLE_SORT_HILO        (double array[], int n);


#endif
