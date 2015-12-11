#ifndef notification_h
#define notification_h

/* Notes -
 * 1) Conditional Compilation for Mac and Linux platforms implemented.
 */

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

/* Support Library */
#include <std.h>
#include <err.h>

/* Interface Function Declarations */
void notification(String watch);    /* Returns when notification occurs */

#ifdef __MACH__	/* Mac OS */
/* System Includes */
#include <sys/event.h>
#include <unistd.h>
#endif	/* __MACH__ */

#ifndef __MACH__	/* All OS except Mac OS */
/* System Includes */
#include <sys/inotify.h>
/* Definitions */
#define EVENT_SIZE  (sizeof (struct inotify_event))
#define BUF_LEN     (LINE * (EVENT_SIZE + 16))
/* Function Declaration */
void checkEventForMask(struct inotify_event *event, uint32_t check_mask);
#endif	/* __MACH__ */

#endif	/* notification_h */
