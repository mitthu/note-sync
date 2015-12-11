/*
 * Main Contolling Server Instance,
 * (c) mitthu
 */
/* Standard Library */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>		/* Signal Handling */
#include <fcntl.h>		/* for file handling */
/* Support Library */
#include <server.h>
#include <all.h>

/* Globals */
String old, generic;
String new;
String patch;
int child_fd;
int child_ctrlfd;
bool push_patch = YES;
pid_t main_pid;

int main(void)
{
    initialize("server");
    DEBUG = FALSE;
    /* changing directory */
    if (chdir(getValueForKeyStringValue("server:dir")) == -1)
        printf("Changing directory failed...");
	banner("Server Initiated");
	/* test block */
	bool auth = getValueForKeyBoolValue("auth:status");
	if (auth == YES) {
		printf("Authentication enabled\n");
	} else {
		printf("Authentication closed\n");
	}
	
    /* Unarchiving variables from settings file */
    String port_number = getSettingForKeyStringValue("default:port");
    
	/* Initializing Filenames */
    old = getValueForKeyStringValue("server:database:generic");
    new = getValueForKeyStringValue("server:database:newfile");
    patch = getValueForKeyStringValue("server:database:patch");
	generic = old;
	main_pid = getpid();
	
    /* Registering Signals */
    registerSignal(SIGINT, sigintMain);			/* Modify in Child */
	registerSignal(SIGCHLD, sigchld_handler);
	registerSignal(TO_SERVER, notifyServer);	/* Ignore in child */
	registerSignal(TO_CHILD, SIG_IGN);			/* Register in child */
    
	int sockfd, new_fd, ctrlfd; // listen on sock_fd, new connection on new_fd, new connection on ctrlfd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, port_number, &hints, &servinfo)) != 0)
		err_fatal("getaddrinfo: %s\n", gai_strerror(rv));
	
	/* loop through all the results and bind to the first we can */
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			err_sys("server: socket");
			continue;
		}
	
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
			err_fatal("setsockopt");
		
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			err_sys("server: bind");
			continue;
		}
		break;
	}

	if (p == NULL)
		err_fatal("Failed to bind");
	
	freeaddrinfo(servinfo); // all done with this structure
	
	if (listen(sockfd, BACKLOG) == -1)
		err_fatal("listen");
	
	err_log("Waiting for connections...");
    
	while(TRUE) { // main accept() loop
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		err_log("Got connection from %s", s);
		
		/* Receiving Client ID */
		int numbytes;
		char buf[LINE]; message *msg;
		if ((numbytes = recv(new_fd, buf, LINE, 0)) == -1)
			err_sys("recv");
		msg = (message *) buf;
		
		/* Accepting Control Connection */
		/* Assumption - No other connection comes before */
		sin_size = sizeof(their_addr);
		ctrlfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (ctrlfd == -1) {
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family,
				  get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		err_log("Got contol connection from %s", s);
		
		/* Receiving Client ID */
		if ((numbytes = recv(new_fd, buf, LINE, 0)) == -1)
			err_sys("recv");
		msg = (message *) buf;
		
		/* Forking child process */
		if (!fork()) { // this is the child process
				close(sockfd); // child doesn't need the listener
				handleChild(new_fd, ctrlfd);
			}
			close(ctrlfd); // parent doesn't need this
			close(new_fd); // parent doesn't need this
	}
    
    endapp();
	return 0;
}

void handleChild(int fd, int ctrlfd) {	
	/* Setting Globals */
	child_fd		=	fd;
	child_ctrlfd	=	ctrlfd;
	NAME = "server.child";
	
	/* Registering Signals */
	registerSignal(SIGINT, sigintChild);		/* Different from Main */
	registerSignal(TO_SERVER, SIG_IGN);			/* Used by Main Process */
	registerSignal(TO_CHILD, notifyForSync);	/* Register in child */
	registerSignal(SIGSEGV, connectionLost);	/* Connection closed by peer */
	
	/* Authentication Block */
	if (getValueForKeyBoolValue("auth:status") == YES)
		if (giveUserAccess(fd) == NO)
			err_fatal("Trials exceeded the limit (%d). Closing all connections.\n", getValueForKeyIntValue("server:auth:max:retries"));

	err_log("Authentication Complete...");
	/* Post Authentication Stage */
    
    /* Sending Original file */
    if (sendFileFD(generic, child_fd) == FAILURE)
        err_fatal("Sending original file failed");
	while (TRUE) {
        /* Receiving Patch */
		if (recvFileFD(patch, child_fd) == FAILURE) {
			err_log("Receiving patch failed");
			continue;
		}
		applyPatch(old, patch);
		push_patch = NO;	/* Don't self push the patch again */
		/* Notify Main Process */
		kill(main_pid, TO_SERVER);
    }
    
    close(fd);
    exit(0);
}

/* Sending Push Notification */
int pushPatch(void) {
	if (sendFileFD(patch, child_ctrlfd) == FAILURE) {
        err_log("Patch sending failed");
		return FAILURE;
	}
	return SUCCESS;
}

/* get sockaddr, IPv4 or IPv6 */
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* Signals */
/* Service Interrupt - Main */
void sigintMain(int s) {
    err_log("Main received Interrupt. Shutting down...@ %d", getpid());
	endapp();
    exit(0);
}

/* Service Interrupt - Child */
void sigintChild(int s) {
    err_log("Child received Interrupt. Shutting down...@ %d", getpid());
    exit(0);
}

/* Reaping Zombie Children */
void sigchld_handler(int s) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

/* Received by server...
 * Notify that some child generated a patch */
void notifyServer(int s) {
	kill(0, TO_CHILD); /* Server to All Process Group Members */
}

/* Received by clients...
 * Notify that patch needs to be pushed */
void notifyForSync(int s) {
	if (push_patch == NO) {
		push_patch = YES;
	} else {
		err_log("Notification for push received...");
		pushPatch();
	}
}
