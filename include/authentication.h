#ifndef transmission_h
#define transmission_h

/* System Includes */
#include <stdio.h>
/* Support Library */
#include <std.h>

/* Interface Function Prototypes */
bool interactiveUserValidation(int sockfd); /* for client */
bool giveUserAccess(int sockfd); /* for server */
bool isValidUser(String name, String password);

/* Function Prototypes */

#endif
