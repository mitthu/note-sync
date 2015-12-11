#ifndef std_h
#define std_h

/* System Includes */
#include <stdio.h>
#include <sys/stat.h>	/* for PERMS */

/* Decalring new data types */
typedef char* String;
typedef enum {
    TRUE = 1,
    FALSE = 0
} bool_cond;
typedef enum {
    YES = 1,
    NO = 0
} bool;
typedef enum {
    SUCCESS = 1,
    FAILURE = -1
} return_value;

/* Definitions */
#ifndef NULL
#define NULL (char *) 0
#endif

#define FILENAME	256
#define LINE		4096

/* global variables */
int m_logfd;
short m_debug;	/* Make '0'/'NO' to turn debugging off */
String m_name;	/* Name of Program */
String m_db_base;	/* Base name of all files generated */
char m_db_rm[FILENAME];	/* Removal String */

#define LOGFD m_logfd
#define DEBUG m_debug
#define NAME m_name
#define DB_BASE m_db_base
#define DB_RM m_db_rm
#define LOG_FILE "complete.log"
#define SMALL_SEP "--------"
#define PERMS (S_IRWXU | S_IRWXG | S_IRWXO)

/* function declaration */
void initialize(String what);
void endapp();
void banner(String advertisement, ...);
void bannerInLog(String advertisement, ...);

/* Write Locks */
int lockFile(int fd);	/* With Deadlock Prevention */
int unlockFile(int fd);
int lockFileNoWait(int fd);	/* No Deadlock Prevention */

/* Signals - Any errors are all considered fatal */
void registerSignal(int sig, void (*handler)(int));

/* Signal Handlers */
void connectionLost(int s);	/* SIGSEGV */

#endif
