#ifndef transmission_h
#define transmission_h

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  /* file handling */
#include <unistd.h>  /* file handling */
#include <sys/stat.h> /* for umask() */
#include <sys/types.h>
#include <sys/socket.h>
/* Support Library */
#include "std.h"

#define MSG_CMD_LEN 512
#define TX_DATA_PACKETS 512

/* Transmission Control */
typedef struct {
    char cmd[MSG_CMD_LEN];
	int id; /* used to identify client */
} message;

typedef struct {
    int len_data;
} data;

/* file transfer control */
typedef struct {
    int len;
    String name;
} file_transfer;

/* user authentication control */
typedef struct {
    String name;
	String password;
} user_authenticate;

/* user validity check */
typedef struct {
    bool isValid;
} user_validity;

/* user max retries */
typedef struct {
    int retries;
} user_retry;

typedef union {
    file_transfer *fileTransfer;
	user_authenticate *userAuthenticate;
	user_validity *userValidity;
    user_retry *userRetry;
} tx_cmd;

/* function prototypes */
String parseCommand(message *msg, tx_cmd *raw_data);
int sendFileFD(String file_name, int sockfd);
int recvFileFD(String file_name, int scokfd);
int sendMessage(message *msg, int sockfd); /* returns SUCCESS/FALIURE */
int recvMessage(message *msg, int sockfd); /* returns SUCCESS/FALIURE */
ssize_t writen(int fd, void *buf, size_t nbytes);
ssize_t readn(int fd, void *buf, size_t nbytes);

#endif
