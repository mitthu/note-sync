#include "std.h"
/* Standard Library */
#include <sys/stat.h>	/* for umask */
#include <fcntl.h>	/* File locking */
#include <unistd.h>	/* for getpid() */
#include <signal.h>	/* for signal handling functions */
/* Support Library */
#include "err.h"
#include "parse.h" /* to connect the global function pointers */
#include "sync.h"

void initialize(String what) {
	/* Setting appropriate function pointers*/
    getValueForKey = getSettingForKey;
    getValueForKeyIntValue = getSettingForKeyIntValue;
    getValueForKeyStringValue = getSettingForKeyStringValue;
	getValueForKeyBoolValue = getSettingForKeyBoolValue;
	/* Setting Global Variables */
	DEBUG = 0;
	NAME = what;
	/* open the log file for data entry */
	/* NOT USING LOG_FILE */
	char file[256];
	pid_t self_pid = getpid();
	sprintf(file, "%s.%d.log", NAME, self_pid);
	if((LOGFD = open(file, O_RDWR | O_CREAT | O_APPEND, PERMS)) == -1)
		perror("Opening log file failed.");
	bannerInLog("Routine Initialized (%s) @ %d", NAME, self_pid);
	umask(S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH); /* changing umask */
	/* Populate Settings */
	populateSettingsWithFile("settings.txt");
	/* Setting up Database Parameters */
	DB_BASE = getValueForKeyStringValue("database:basename");
	sprintf(DB_RM, "%s.* *cache*", DB_BASE);
}

void endapp() {
#ifdef  __MACH__    /* Mac OS */
    if (fork()) /* Parent Process terminated, as kqueue fd's 
                 * are not shared by a fork */
        exit(0);
#endif
    if (strcmp(NAME, "client") == 0) {
        err_log("Removing Files");
        deleteFile(DB_RM);
    }
	if (strcmp(NAME, "server") == 0)
		deleteFile(getValueForKeyStringValue("server:database:patch"));
	close(LOGFD);
}

void banner(String advertisement, ...) {
	char buf[LINE];
    va_list arg;        
    va_start(arg, advertisement);
    vsprintf(buf, advertisement, arg);
    va_end (arg);
	
	printf("%s\n", SMALL_SEP);
	printf("| %s\n", buf);
	printf("%s\n", SMALL_SEP);
}

void bannerInLog(String advertisement, ...) {
	char buf[LINE];
    va_list arg;        
    va_start(arg, advertisement);
    vsprintf(buf, advertisement, arg);
    va_end (arg);
	
	err_log("%s", SMALL_SEP);
	err_log("| %s", buf);
	err_log("%s", SMALL_SEP);
}

int lockFile(int fd) {
	struct flock lock; 
	/* set the parameters for the write lock */ 
	lock.l_type = F_WRLCK; 
	lock.l_whence = SEEK_SET; 
	lock.l_start = 0; 
	lock.l_len = 10;	/* entire file */
	
	if(fcntl(fd, F_SETLKW, &lock) == -1) {
		err_sys("Lock file failed");
		return FAILURE;
	}
	return SUCCESS;
}

int unlockFile(int fd) {
	struct flock lock;
	/* set the parameters for the write lock */ 
	lock.l_type = F_UNLCK; 
	lock.l_whence = SEEK_SET; 
	lock.l_start = 0; 
	lock.l_len = 0;	/* entire file */
	
	if(fcntl(fd, F_SETLKW, &lock) == -1) {
		err_sys("Unlocking file failed");
		return FAILURE;
	}
	return SUCCESS;
}

int lockFileNoWait(int fd) {
	struct flock lock; 
	/* set the parameters for the write lock */ 
	lock.l_type = F_WRLCK; 
	lock.l_whence = SEEK_SET; 
	lock.l_start = 0; 
	lock.l_len = 10;	/* entire file */
	
	if(fcntl(fd, F_SETLK, &lock) == -1) {
		err_sys("Lock file failed");
		return FAILURE;
	}
	return SUCCESS;
}

void registerSignal(int sig, void (*handler)(int)) {
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(sig, &sa, NULL) == -1)
		err_fatal("Handling signal (%d) failed", sig);
}

void connectionLost(int s) {
	err_log("Connection lost... @ %s (%d)", NAME, (int) getpid());
	endapp();
	exit(1);
}
