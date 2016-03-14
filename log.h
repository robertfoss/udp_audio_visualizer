#ifndef LOG_H
#define LOG_H

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#ifdef NDEBUG
#  define debug(M)
#  define debugf(M, ...)
#else
#  define debug(M) fprintf(stderr, "DEBUG %s:%d %s(): " M "\n", __FILE__, __LINE__, __func__)
#  define debugf(M, ...) fprintf(stderr, "DEBUG %s:%d %s(): " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#endif


#define LOG_INFO 1
#define LOG_WARN 2
#define LOG_ERR  4
#ifndef LOG_LEVELS
#define LOG_LEVELS (LOG_INFO | LOG_WARN | LOG_ERR)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_ass(M) fprintf(stderr, "[ASSERT] %s:%d %s() " M "\n", __FILE__, __LINE__, __func__)
#define log_assf(M, ...) fprintf(stderr, "[ASSERT] %s:%d %s() " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#if (LOG_LEVELS & LOG_ERR)
#define log_err(M) fprintf(stderr, "[ERROR] %s:%d %s() errno: %s " M "\n", __FILE__, __LINE__, __func__, clean_errno())
#define log_errf(M, ...) fprintf(stderr, "[ERROR] %s:%d %s() errno: %s " M "\n", __FILE__, __LINE__, __func__, clean_errno(), ##__VA_ARGS__)
#else
#define log_err(M)
#define log_err(M, ...)
#endif

#if (LOG_LEVELS & LOG_WARN)
#define log_warn(M) fprintf(stderr, "[WARN] %s:%d %s() errno: %s " M "\n", __FILE__, __LINE__, __func__, clean_errno())
#define log_warnf(M, ...) fprintf(stderr, "[WARN] %s:%d %s() errno: %s " M "\n", __FILE__, __LINE__, __func__, clean_errno(), ##__VA_ARGS__)
#else
#define log_warn(M)
#define log_warnf(M, ...)
#endif

#if (LOG_LEVELS & LOG_INFO)
#define log_info(M) fprintf(stderr, "[INFO] %s:%d %s() " M "\n", __FILE__, __LINE__, __func__)
#define log_infof(M, ...) fprintf(stderr, "[INFO] %s:%d %s() " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define log_info(M)
#define log_infof(M, ...)
#endif

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


#endif//LOG_H
