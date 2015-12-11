#ifndef err_h
#define err_h

/* System Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>

/* Interface */
#define err_debug(...) m_err_debug(__FILE__, __LINE__,##__VA_ARGS__)

int err_log(const char *format, ...);
int err_info(const char *format, ...);
int err_sys(const char *format, ...);
void err_fatal(const char *format, ...);

/* Support sunctions */
int err_generic(const char *type, const char *err);
int m_err_debug(const char *file, const int line, const char *format, ...);

/* Error Numbers */
typedef enum {
    mENODATA,
    mENOENTRY
    } m_error; 
m_error global_error;

#endif
