/*
 * Client Service Provider
 * (c) mitthu
 */
/* Standard Library */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>		/* file handling */
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>	/* Threading library */
/* Support Library */
#include <client.h> /* My Client Header */
#include <authentication.h>

/* Globals */
String old, generic;
String new;
String patch;
int fd;
int ctrlfd;
bool notify_status = YES;       /* controlExchange() */
bool have_generic_file = NO;    /* dataExchange() */
int push_cnt = 0;               /* controlExchange() */

int main(int argc, char *argv[]) {
    initialize("client");
    populateSettingsWithFile("settings.txt"); DEBUG = TRUE;
    /* changing directory */
    if (chdir(getValueForKeyStringValue("client:dir")) == -1)
        printf("Changing directory failed...");
	banner("Client Initiated");
    
    /* Unarchiving variables from settings file */
    String port_number = getValueForKeyStringValue("default:port");
    String server_addr = getValueForKeyStringValue("client:default:server_addr");
	
	/* Initializing Filenames */
    old = getValueForKeyStringValue("client:database:generic");
    new = getValueForKeyStringValue("client:database:newfile");
    patch = getValueForKeyStringValue("client:database:patch");
	generic = old;
	
	/* Registering Signals */
    registerSignal(SIGINT, sigint);
	registerSignal(SIGSEGV, connectionLost);
	registerSignal(PUSH_TO_SERVER, pushToServer); /* used later */
	
	/* client part */
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    /* Changing default server address, if specified */
    if (argc == 2) {
        server_addr = argv[1];
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(server_addr, port_number, &hints, &servinfo)) != 0) {
        err_log("getaddrinfo: %s", gai_strerror(rv));
        return FAILURE;
    }
    /* loop through all the results and connect to the first we can */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            err_sys("client: socket");
            continue;
		}
        if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(fd);
            err_sys("client: connect");
            continue;
		}
        break;
	}
	/* Check if we actually could get a server or not */
    if (p == NULL) {
        err_log("client: failed to connect");
        return FAILURE;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    err_log("Connecting to %s", s);
	
	/* Identifying itself to server */
	message msg;
	msg.id = 1;
	if (send(fd, &msg, sizeof(message), 0) == -1)
		err_sys("send");
	
	/* Setting the control connection */
	if ((ctrlfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		err_sys("client: socket");
	
	if (connect(ctrlfd, p->ai_addr, p->ai_addrlen) == -1) {
		close(ctrlfd);
		err_sys("client: connect");
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    err_log("Control connecting to %s", s);
	
	/* Identifying itself to server */
	msg.id = 1;
	if (send(fd, &msg, sizeof(message), 0) == -1)
		err_sys("send");
	
	freeaddrinfo(servinfo); // all done with this structure

	/* handling connection */
	handleConnection(fd, ctrlfd);
    
	endapp();
    return SUCCESS; 
}

void handleConnection(int fd, int ctrlfd) {
    pthread_t threads[MAX_THREADS];
    
	/* Authentication Block */
	if (getValueForKeyBoolValue("auth:status") == YES) {
		if (interactiveUserValidation(fd) == NO)
			err_fatal("User authentication failed");
		else
			err_log("Authentication Complete...");
	}
	/* Post Authentication Stage */
    pthread_create(&threads[0], NULL, dataExchange, NULL);
    pthread_create(&threads[1], NULL, controlExchange, NULL);
    
    /* Synchronize the completion of each thread.   */
    int i;
    for (i = 0; i < MAX_THREADS; i++) {
        /* Use man pthread_join to see what it does.  */
        pthread_join(threads[i], NULL);
    }
    
    close(fd);
	close(ctrlfd);
}

void *dataExchange(void *arg) {
    err_log("Data Exchange Thread");
    
    /* Receiving Cache File */
    if (recvFileFD(generic, fd) == FAILURE)
        err_fatal("Receiving original file failed");
    else
        have_generic_file = YES;
    
	/* Making 'main' file */
	if(copyFile(generic, new) == FAILURE)
		err_log("Copying cache to main file failed");
	
    /* --------------IMPORTANT-------------- */
    /* Deal with notification */
	err_log("Watching file for changes...");
    while (TRUE) {
        notification("note.txt"); /* Returns when notification occurs */
		err_log("File changed");
        if (notify_status == NO) {
            notify_status = YES;
			continue;       /* Goto next iteration */
        }
		/* If Notification is enabled */
		err_log("Pushing to server");
		raise(PUSH_TO_SERVER);
    }
}

void *controlExchange(void *arg) {
    /* Loop until original file is received */
    while (have_generic_file != YES) {
        
    }
    
    while (TRUE) {
        /* Getting Push Notification */
        if (recvFileFD(patch, ctrlfd) == FAILURE) {
            err_log("Patch receive failed");
        }
        err_log("Patch Received");
        /* Deactivating local notifications */
        notify_status = NO;
        
        /* Applying changes */
        if (applyPatch(old, patch) == FAILURE)
            err_log("Applying patch failed");
        if (applyPatch(new, patch) == FAILURE)
            err_log("Applying patch failed");
		
        /* Deleting patch file */
        if (deleteFile(patch) == FAILURE)
            err_log("Deleting failed");
        
        /* Reactivating notifications */
        notify_status = YES;
        
        push_cnt++;
        err_log("Push Notification Serviced...(%d)", push_cnt);
    }
}

/* get sockaddr, IPv4 or IPv6 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* Signal Handlers */
/* Interrupt Function */
void sigint(int s) {
    err_log("Interrupt received. Closing connections...@ %d", getpid());
	endapp();
	exit(0);
}

/* Pushing patch to server */
void pushToServer(int s) {
	/* Generating Patch */
	genPatch(old, new, patch);
	/* Updating Server Copy */
	if (sendFileFD(patch, fd) == FAILURE)
		err_log("Sending patch failed");
	/* Applying patch locally */
	applyPatch(old, patch);
	/* Remove Patch */
	deleteFile(patch);
}
