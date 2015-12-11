#ifndef client_h
#define client_h

/* Notes -
 * 1) SIGUSR1 - Generating and pushing patch to server.
 */

#include "all.h"

/* definitions */
#define MAXDATASIZE getValueForKeyIntValue("data_length")
#define MAX_THREADS 2
#define PUSH_TO_SERVER SIGUSR1

/* function declarations */
void handleConnection(int datafd, int ctrlfd);
void *dataExchange(void *arg); /* Data Thread */
void *controlExchange(void *arg); /* Control Thread */

/* Signal Handlers */
void sigint(int s);			/* SIGINT */
void pushToServer(int s);	/* SIGUSR1 */

/* Other Connection Oriented */
void *get_in_addr(struct sockaddr *sa);

/* Structures */
typedef struct { /* wraps data for threads */
    int datafd, ctrlfd; /* file/socket descriptor */
} twrap;

#endif
