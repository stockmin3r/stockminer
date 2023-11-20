#ifndef __HTTP_REQUESTS_H
#define __HTTP_REQUESTS_H

#define HTTP_MAINPAGE       "HTTP/1.1 200 OK\r\n"                                 \
                            "Content-type: text/html\r\n"                         \
                            "Connection: keep-alive\r\n"                          \
                            "Content-Encoding: gzip\r\n"                          \
                            "Last-Modified: Wed, 28 Apr 2021 02:28:12 GMT\r\n"    \
                            "Cache-Control: max-age=0, must-revalidate\r\n"       \
                            "Access-Control-Allow-Origin: *\r\n"                  \
                            "ETag: \"        \"\r\n"                              \
                            "Content-Length:      \r\n\r\n"                       \

#define HTTP_HTML           "HTTP/1.1 200 OK\r\n"                                 \
                            "Connection:keep-alive\r\n"                           \
                            "Content-Type:text/html\r\n"                          \
                            "cache-control:public,max-age=5600000\r\n"            \
                            "ETag: \"        \"\r\n"                              \
                            "Content-Encoding:gzip\r\n"                           \
                            "Content-Length:      \r\n\r\n"                       \

#define HTTP_GZIP2          "HTTP/1.1 200 OK\r\n"                                 \
                            "Connection:keep-alive\r\n"                           \
                            "Content-Type:application/gzip\r\n"                   \
                            "Content-Encoding:gzip\r\n"                           \
                            "Cache-Control:max-age=3153600\r\n"                   \
                            "Content-Length: %ld\r\n\r\n"                        \


#define HTTP_JAVASCRIPT     "HTTP/1.1 200 OK\r\n"                                 \
                            "Connection:keep-alive\r\n"                           \
                            "Content-Type:application/javascript\r\n"             \
                            "Content-Encoding:gzip\r\n"                           \
                            "Cache-Control:max-age=3153600\r\n"                   \
                            "Content-Length:       \r\n\r\n"                      \

#define HTTP_WASM           "HTTP/1.1 200 OK\r\n"                                 \
                            "Content-Type: application/wasm\r\n"                  \
                            "cache-control: public,max-age=0,must-revalidate\r\n" \
                            "Cache-Control: max-age=3153600\r\n"                  \
                            "Content-Length: %d\r\n\r\n"                          \


#define HTTP_CSS            "HTTP/1.1 200 OK\r\n"                      \
                            "Connection:keep-alive\r\n"                \
                            "Content-Type:text/css\r\n"                \
                            "Content-Encoding:gzip\r\n"                \
                            "Cache-Control:max-age=3153600\r\n"        \
                            "Content-Length:       \r\n\r\n"           \

#define JSON_RSP            "HTTP/1.1 200 OK\r\n"                      \
                            "Content-type:application/json\r\n"        \
                            "Content-Length:        \r\n\r\n"          \

#define HTTP_IMAGE          "HTTP/1.1 200 OK\r\n"                      \
                            "Content-Type:image/png\r\n"               \
                            "Connection:keep-alive\r\n"                \
                            "Cache-Control:max-age=3153600\r\n"        \
                            "Content-Length: %lld\r\n\r\n"             \

#define HTTP_IMAGE2         "HTTP/1.1 200 OK\r\n"                      \
                            "Content-Type:image/png\r\n"               \
                            "Connection:keep-alive\r\n"                \
                            "Cache-Control:max-age=3153600\r\n"        \
                            "Content-Length:           \r\n\r\n"       \

#define HTTP_PDF            "HTTP/1.1 200 OK\r\n"                      \
                            "Content-Type:application/pdf\r\n"         \
                            "Connection:keep-alive\r\n"                \
                            "Cache-Control:max-age=3153600\r\n"        \
                            "Content-Length:           \r\n\r\n"       \

#define HTTP_FONT           "HTTP/1.1 200 OK\r\n"                      \
                            "Connection:keep-alive\r\n"                \
                            "Content-Type: font/woff2\r\n"             \
                            "cache-control:public,max-age=5600000\r\n" \
                            "Content-Length: %lld\r\n\r\n"             \

#define HTTP_GET_VIDEO      "HTTP/1.1 200 OK\r\n"                      \
                            "Content-Type: video/mp4\r\n"              \
                            "Connection: close\r\n"                    \
                            "%s\r\n"                                   \
                            "Cache-Control: max-age=31536000\r\n"      \
                            "Content-Length: %d\r\n\r\n"               \

#define HTTP_GZIP           "HTTP/1.1 200 OK\r\n"                      \
                            "Connection: keep-alive\r\n"               \
                            "Content-Type: text/html\r\n"              \
                            "Content-Encoding: gzip\r\n"               \
                            "Content-Length:           \r\n\r\n"       \

#define ROBOTS_TXT          "HTTP/1.1 200 OK\r\n"                      \
                            "Connection:close\r\n"                     \
                            "Content-Type:text/plain\r\n"              \
                            "Content-Length:    \r\n\r\n"              \
                            "User-Agent: *\nAllow: /\n"                \

#define HTML_OFFSET             159      /* AboutPage Length: offset */
#define HTML_ETAG_OFFSET        110      /* AboutPage ETag:   offset */
#define JAVASCRIPT_OFFSET       147      /* JS Length:        offset */
#define CSS_OFFSET              133      /* CSS Length:       offset */
#define JSON_RSP_OFFSET         63       /* JSON RSP Length:  offset */
#define IMG_OFFSET              111      /* Image Length:     offset */
#define PDF_OFFSET              117      /* PDF Length:       offset */
#define GZIP_OFFSET             106      /* gzip Length:      offset */

#define SIZEOF_MAINPAGE         sizeof(HTTP_MAINPAGE)-1
#define SIZEOF_HTML             sizeof(HTTP_HTML)-1
#define SIZEOF_JAVASCRIPT       sizeof(HTTP_JAVASCRIPT)-1
#define SIZEOF_CSS              sizeof(HTTP_CSS)-1
#define SIZEOF_JSON_RSP         sizeof(JSON_RSP)-1

#define REDIRECT_HTTPS      "HTTP/1.1 301 Moved Permanently\r\n"           \
                            "Location: https://localhost\r\n\r\n"          \

#define REDIRECT_LOGIN      "HTTP/1.1 302 Found\r\n"                       \
                            "Location: /login\r\n\r\n"                     \

#define REDIRECT_ERROR      "HTTP/1.1 302 Found\r\n"                       \
                            "Location: /error\r\n\r\n"                     \

#define REDIRECT_HOME       "HTTP/1.1 301 Moved Permanently\r\n"           \
                            "Location: /\r\n\r\n"                          \

#define HTTP_404            "HTTP/1.1 404 Not Found\r\n\r\n"
#define HTTP_304            "HTTP/1.1 304 Not Modified\r\n\r\n"

#endif
