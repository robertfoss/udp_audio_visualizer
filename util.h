#ifndef UTIL_H
#define UTIL_H

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t ret_code;
#define RET_OK  (ret_code) 0
#define RET_ERR (ret_code) 1

#define DIV_ROUND(top, bottom) ((top + bottom - 1) / (bottom))

#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) \
__extension__({ typeof (a) _a = (a), _b = (b); _a > _b ? _a : _b; })

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) \
__extension__({ typeof (a) _a = (a), _b = (b); _a < _b ? _a : _b; })

#define UNUSED(x) (void)(x)

#ifdef NDEBUG
#  define debug(M)
#  define debugf(M, ...)
#else
#  define debug(M) fprintf(stderr, "DEBUG %s:%d %s(): " M "\n", __FILE__, __LINE__, __func__)
#  define debugf(M, ...) fprintf(stderr, "DEBUG %s:%d %s(): " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_ass(M) fprintf(stderr, "[ASSERT] %s:%d %s() " M "\n", __FILE__, __LINE__, __func__)
#define log_assf(M, ...) fprintf(stderr, "[ASSERT] %s:%d %s() " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define log_err(M) fprintf(stderr, "[ERROR] %s:%d %s() errno: %s " M "\n", __FILE__, __LINE__, __func__, clean_errno())
#define log_errf(M, ...) fprintf(stderr, "[ERROR] %s:%d %s() errno: %s " M "\n", __FILE__, __LINE__, __func__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M) fprintf(stderr, "[WARN] %s:%d %s() errno: %s " M "\n", __FILE__, __LINE__, __func__, clean_errno())
#define log_warnf(M, ...) fprintf(stderr, "[WARN] %s:%d %s() errno: %s " M "\n", __FILE__, __LINE__, __func__, clean_errno(), ##__VA_ARGS__)

#define log_info(M) fprintf(stderr, "[INFO] %s:%d %s() " M "\n", __FILE__, __LINE__, __func__)
#define log_infof(M, ...) fprintf(stderr, "[INFO] %s:%d %s() " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#define STR(s) #s
#define DIE(M) log_ass(M); \
    exit(1)
#define DIEF(M, ...) log_ass(M, ##__VA_ARGS__); \
    exit(1)

#define ASSERT(err) if(!(err)) {\
        DIE(STR(err));\
    }


#endif//UTIL_H
