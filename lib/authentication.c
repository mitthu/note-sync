/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Support Library */
#include <std.h>
#include <err.h>
#include <parse.h>
#include <transmission.h>
#include <authentication.h>

bool isValidUser(String name, String password) {
	char buf[LINE];
	String password_stored;
	
	sprintf(buf, "%s%s", "user:", name);
	if ((password_stored=getValueForKeyStringValue(buf)) == NULL)
		return NO;
	if (strcmp(password, password_stored) == 0)
		return YES;
	else
		return NO;
}

/* Improvement -
 * Get restriction on lengths of username and password.
 */
bool interactiveUserValidation(int sockfd) { /* used by client */
	banner("User Authentication Environment");
	int size = 256;
    char name[size], pass[LINE];
	int tries, max_tries;
	bool user_is_valid = NO;
    message msg;
	tx_cmd raw_type;
	user_validity *valid;
	
    if (recvMessage(&msg, sockfd) == FAILURE) {
        err_log("Couldn't ge maximum retries");
    }
    parseCommand(&msg, &raw_type);
    user_retry *retry = raw_type.userRetry;
    tries = retry->retries;
    max_tries = tries;
    
	for (; tries != 0; tries--) {
		printf("Enter username: "); scanf("%s", name);
		printf("Enter password: "); scanf("%s", pass);
		sprintf(msg.cmd, "%s %s %s", "user_authenticate", name, pass);
		/* sending to server for validation */
		if (sendMessage(&msg, sockfd) == FAILURE)
			err_sys("Sending authentication token message failed");
		/* getting acknowledgement from server */
		if (recvMessage(&msg, sockfd) == FAILURE)
			err_sys("Receiving acknowledgement authentication token message failed");
		parseCommand(&msg, &raw_type); /* fill up structure */
		valid = raw_type.userValidity;
		if (valid->isValid == YES) {
			user_is_valid = YES;
			break;
		}
		banner("Retry...");
	}
    
    if (user_is_valid == NO)
        err_log("Exceeded max. allowed trials (%d)", max_tries);
    
	return user_is_valid;
}

bool giveUserAccess(int sockfd) { /* for server */
    message msg, ack;
    int tries = getValueForKeyIntValue("server:auth:max:retries");
	bool user_is_valid = NO;
	
    sprintf(msg.cmd, "user_retry %d", tries);
    if (sendMessage(&msg, sockfd) == FAILURE)
        err_log("Couldn't send maximum retries");
    
	for (; tries != 0; tries--) {
		if (recvMessage(&msg, sockfd) == FAILURE)
			err_sys("Receiving authentication token message failed");
		tx_cmd raw_type;
		parseCommand(&msg, &raw_type); /* fill up structure */
		user_authenticate *credentials = raw_type.userAuthenticate;
        
		if (isValidUser(credentials->name, credentials->password) == YES) {
			err_log("User Validated");
			user_is_valid = YES;
			
			sprintf(ack.cmd, "user_validity YES");
            if (sendMessage(&ack, sockfd) == FAILURE)
                err_sys("Sending acknowledgement authentication token message failed");
			break;
		} else {
			printf("Invalid user request (%d).\n", tries-1);
			sprintf(ack.cmd, "user_validity NO");
            if (sendMessage(&ack, sockfd) == sizeof(ack))
                err_sys("Sending acknowledgement authentication token message failed");
		}
	}
//    err_log("Coming out of authentication");
	return user_is_valid;
}
