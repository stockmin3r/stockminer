#ifndef __CDEFS_H
#define __CDEFS_H

#include <stdinc.h>

#ifdef __cplusplus
#define __BEGIN_CPP extern "C" {
#define __END_CPP   }
#endif

#define QMALLOC(ptr, type, size ) \
({                                \
    ptr = (type) malloc(size);    \
    if (!ptr)                     \
        return;                   \
    (type)ptr;                    \
})

#define QMALLOC_RNULL(ptr, type, size ) \
({                                      \
    ptr = (type) malloc(size);          \
    if (!ptr)                           \
        return NULL;                    \
    (type)ptr;                          \
})

#define QMALLOC_RBOOL(ptr, type, size ) \
({                                      \
    ptr = (type) malloc(size);          \
    if (!ptr)                           \
        return false;                   \
    (type)ptr;                          \
})

/*****************************************
 * size: The required size of the buffer *
 * max:  The current  size of the buffer *
 ****************************************/
#define REALLOC(ptr, type, size, max)   \
({                                      \
    int mul = size*2;                   \
    if (mul >= max) {                   \
        if (size >= max)                \
            max  = (size+mul);          \
        else                            \
            max *= 2;                   \
        ptr  = (type)realloc(ptr, max); \
        if (!ptr)                       \
            break;                      \
    }                                   \
    (type)ptr;                          \
})


#ifndef bswap_64
#define bswap_64(x)                             \
    ((((x) & 0xff00000000000000ull) >> 56)      \
   | (((x) & 0x00ff000000000000ull) >> 40)      \
   | (((x) & 0x0000ff0000000000ull) >> 24)      \
   | (((x) & 0x000000ff00000000ull) >> 8)       \
   | (((x) & 0x00000000ff000000ull) << 8)       \
   | (((x) & 0x0000000000ff0000ull) << 24)      \
   | (((x) & 0x000000000000ff00ull) << 40)      \
   | (((x) & 0x00000000000000ffull) << 56))
#endif

#define RESET       "\033[0m"         /* Reset */
#define BLACK       "\033[30m"        /* Black */
#define RED         "\033[31m"        /* Red */
#define GREEN       "\033[32m"        /* Green */
#define YELLOW      "\033[33m"        /* Yellow */
#define BLUE        "\033[34m"        /* Blue */
#define MAGENTA     "\033[35m"        /* Magenta */
#define CYAN        "\033[36m"        /* Cyan */
#define WHITE       "\033[37m"        /* White */
#define BOLDBLACK   "\033[1m\033[30m" /* Bold Black */
#define BOLDRED     "\033[1m\033[31m" /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m" /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m" /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m" /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m" /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m" /* Bold White */

#define __MODULE_HOOK void
#define __MODULE_INIT void

#define SUCCESS true
#define FAILURE false

#endif
