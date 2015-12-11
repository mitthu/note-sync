#ifndef server_h
#define server_h
/* Notes -
 * 1) Pushes data over 'ctrlfd'.
 * 2) SIGUSR1 - Notifies server, sent by child, that a new version
 * is received.
 * 3) SIGUSR2 - Server sends this to all childern (and itself)
 * indicating that all children except the one who generated the
 * patch should push the patch over 'datafd'.
 */


#include <all.h>

#define MAXDATASIZE getValueForKeyIntValue("data_length")
#define BACKLOG 10 // how many pending connections queue will hold
#define MAX_THREADS 2
#define TO_SERVER SIGUSR1
#define TO_CHILD SIGUSR2

/* Function Declaration */
void handleChild(int childfd, int ctrlfd);
int pushPatch(void);	/* Pushes patch over 'ctrlfd' */

/* Signal Handlers */
void sigchld_handler(int s);/* SIGCHLD */
void sigintMain(int s);		/* SIGINT */
void sigintChild(int s);	/* SIGINT */
void notifyServer(int s);	/* SIGUSR1 */
void notifyForSync(int s);	/* SIGUSR2 */

/* Others */
void *get_in_addr(struct sockaddr *sa);

/* Structures */
typedef struct { /* wraps data for threads */
    int datafd, ctrlfd; /* file/socket descriptor */
} twrap;

#endif
