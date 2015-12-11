#include "err.h"
/* Standard Library */
#include <fcntl.h>	/* file i/o */
/* Support Library */
#include "std.h"
#include "transmission.h"

/* Notes -
 * 1) Add perror() type lines for err_sys() and err_fatal().
 */
/* error level - generic */
int err_generic(const char *type, const char *err) {
	time_t raw;
	struct tm *now;
    int buf_size = 256;
	char buf[buf_size];
	char data[LINE];
	
	if(time(&raw) == -1)
		perror("couldn't fetch system time");
	now = localtime(&raw);
    strftime(buf, buf_size, "%b %d, %I:%M:%S %p %Z, %Y", now);
	
	if (strcmp(type, "SYS") == 0) {
		perror(err);
	}
	/* Putting Data in buffer */
	sprintf(data, "%s @ %d (%s) | %s: %s\n", buf, getpid(), NAME, type, err);
	if(strcmp(type, "LOG") == 0)
		fprintf(stderr, "%s", data);
	lockFile(LOGFD);
	writen(LOGFD, data, strlen(data)*sizeof(char));
	unlockFile(LOGFD);
	return 0;
}

/* error level - logging
 */
int err_log(const char *format, ...) {
    char buf[LINE];
    va_list arg;        
    va_start(arg, format);
    vsprintf(buf, format, arg);
    va_end (arg);

	err_generic("LOG", buf);
	return 0;
}

/* error level - debug */
int m_err_debug(const char *file, const int line, const char *format, ...) {
	if(DEBUG) {
		char buf1[LINE], buf2[LINE];
        va_list arg;        
        va_start(arg, format);
        vsprintf(buf1, format, arg);
        va_end(arg);
        
        sprintf(buf2, "%s:%d, %s", file, line, buf1);
		err_generic("DEBUG", buf2);
	}
	return 0;
}

/* error level - info
 * same as log level but shown on standard error device */
int err_info(const char *format, ...) {
    char buf[LINE];
    va_list arg;        
    va_start(arg, format);
    vsprintf(buf, format, arg);
    va_end (arg);
    
	err_generic("INFO", buf);
	return 0;
}

/* used for non-fatal errors */
int err_sys(const char *format, ...) {
	char buf[LINE];
    va_list arg;        
    va_start(arg, format);
    vsprintf(buf, format, arg);
    va_end (arg);
    
	err_generic("SYS", buf);
	return 0;
}

void err_fatal(const char *format, ...) {
	char buf[LINE];
    va_list arg;        
    va_start(arg, format);
    vsprintf(buf, format, arg);
    va_end (arg);
	
	err_generic("FATAL", buf);
	endapp();
	exit(1);
}
