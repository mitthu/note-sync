#include "notification.h"
/* Standard Library */
#include <fcntl.h>
/* Support Library */
#include "std.h"
#include "err.h"

/* Returns when notification occurs */
void notification(String watch) {
#ifdef	__MACH__	/* Mac OS */
	struct kevent change;    /* event we want to monitor */
	struct kevent event;     /* event that was triggered */
	int kq, nev, fd;
	
	if((fd = open(watch, O_RDONLY)) == -1)
		err_log("Error opening file");
	
	/* create a new kernel event queue */
	if ((kq = kqueue()) == -1)
		err_log("kqueue()");
	
	/* initalize kevent structure */
	EV_SET(&change,
           /* the file we are monitoring */ fd,
           /* we monitor vnode changes */ EVFILT_VNODE,
           /* when the file is written add an event, and then clear the
			condition so it doesn't re- fire */ EV_ADD | EV_CLEAR | EV_ENABLE | EV_ONESHOT,
           /* just care about writes to the file */ NOTE_WRITE | NOTE_EXTEND | NOTE_DELETE,
           /* don't care about value */ 0, 0);
	
	/* Event Trigger */
	nev = kevent(kq, &change, 1, &event, 1, NULL);
	if (nev < 0)
		err_log("kevent()");
	else if (event.flags & EV_ERROR)   /* report any error */
			err_log("EV_ERROR: %s\n", strerror(event.data));
	
	close(fd);
	close(kq);
#endif	/* __MACH__ */

#ifndef	__MACH__	/* All OS except Mac OS */
    int length, i = 0;
	int fd;
	int wd;
	char buffer[BUF_LEN];
	
	fd = inotify_init();
	
	if (fd < 0) {
		perror("inotify_init");
	}
	
	wd = inotify_add_watch(fd, watch, IN_MODIFY | IN_CREATE | IN_DELETE);
    i=0;
	length = read(fd, buffer, BUF_LEN);
	
	if (length < 0)
		err_sys("read");
	
	while (i < length) {
		struct inotify_event *event = (struct inotify_event *) &buffer[i];
        if (strcmp(event->name, watch) == 0)
            if (event->len) {
                checkEventForMask(event, IN_CREATE);
                checkEventForMask(event, IN_DELETE);
                checkEventForMask(event, IN_MODIFY);
            }
		i += EVENT_SIZE + event->len;
	}
	inotify_rm_watch(fd, wd);
	close(fd);
#endif	/* __MACH__ */
}

#ifndef	__MACH__	/* All OS except Mac OS */
void checkEventForMask(struct inotify_event *event, uint32_t check_mask) {
    String describeEvent;
    if(check_mask == IN_CREATE)
        describeEvent = "created";
    else if (check_mask == IN_DELETE)
        describeEvent = "deleted";
    else if (check_mask == IN_MODIFY)
        describeEvent = "modified";
    else if (check_mask == IN_OPEN)
        describeEvent = "opened";
    
    if (event->mask & check_mask) {
        if (event->mask & IN_ISDIR) {
            err_log("The directory %s was %s.\n", event->name, describeEvent);       
        }
        else {
            err_log("The file %s was %s.\n", event->name, describeEvent);
        }
    }
}
#endif	/* __MACH__ */
