#include "transmission.h"
/* Standard Library */
#include <stddef.h>
/* Support Library */
#include <std.h>
#include <err.h>
#include <parse.h>

String parseCommand(message *msg, tx_cmd *raw_data) {
    String msg_cmd = msg->cmd;
    
    String sep = " "; /* token separator */
    String cmd = strtok(msg_cmd, sep); /* First token is the command */
    if (strcmp(cmd, "file_transfer") == 0) {
        /* file_transfer <length> <name> */
        file_transfer dat;
        sep = " ";
        dat.len = (atoi(strtok(NULL, sep)));
        dat.name = strtok(NULL, sep);
        
        raw_data->fileTransfer = &dat;
        return cmd;
    } else if (strcmp(cmd, "user_authenticate") == 0) {
		/* user_authenticate <name> <password> */
        user_authenticate dat;
		sep = " ";
		dat.name = strtok(NULL, sep);
		dat.password = strtok(NULL, sep);
		
		raw_data->userAuthenticate = &dat;
		return cmd;
	} else if (strcmp(cmd, "user_validity") == 0) {
		/* user_authenticate <YES/NO> */
        user_validity dat;
		sep = " ";
		String yesNo = strtok(NULL, sep);
		if (strcmp(yesNo, "YES") == 0)
			dat.isValid = YES;
		else
			dat.isValid = NO;
		
		raw_data->userValidity = &dat;
		return cmd;
	} else if (strcmp(cmd, "user_retry") == 0) {
		/* user_retry <number_of_tries> */
        user_retry dat;
		sep = " ";
		dat.retries = atoi(strtok(NULL, sep));
        
		raw_data->userRetry = &dat;
		return cmd;
	}
	return NULL;
}

int sendFileFD(String file_name, int sockfd) {
	err_debug("Sending file...");
    message msg;
	return_value final = SUCCESS;
    
    /* opening file to be sent */
    int send_file = open(file_name, O_RDONLY, NULL);
    if (send_file == -1) {
		err_sys("open");
        err_log("Local file open failed");
        return FAILURE;
    }
		
    off_t file_size = lseek(send_file, 0, SEEK_END);
    lseek(send_file, 0, SEEK_SET); /* restoring seek location */
    
    /* sending file_transfer command */
    sprintf(msg.cmd, "%s %lu %s", "file_transfer", (long) file_size, file_name);
    sendMessage(&msg, sockfd);
	
	/* Sending File */
	char buf[TX_DATA_PACKETS];
	size_t bytes_sch; /* scheduled bytes */
	size_t bytes_left = file_size;
	size_t bytes_sent = 0;
	
    while (bytes_sent < file_size) {
		if (bytes_left >= TX_DATA_PACKETS)
			bytes_sch = TX_DATA_PACKETS;
		else
			bytes_sch = bytes_left;
		
		/* Reading from local file */
		read(send_file, buf, bytes_sch);
		
		/* Sending to client */
		if (writen(sockfd, buf, bytes_sch) != bytes_sch) {
			err_sys("Sending over socket failed");
            final = FAILURE;
            break;
        }
		
		bytes_sent += bytes_sch;
		bytes_left -= bytes_sch;
    }
    
    close(send_file); /* closing the file descriptor */
    return final;
}

/* routine to receive a file */
int recvFileFD(String file_name, int sockfd) {
//    err_debug("Receiving file...");
    
    /* to check if the original file has been modified */
    int file_changed = FALSE;  /* check whether the file has been changed or not */
    int rec_file; /* used to open the desired file */
    char buf[TX_DATA_PACKETS]; /* used to store incoming packets */
    off_t bytes_recv = 0, file_size;
    return_value final = SUCCESS;
    
    /* Receiving Message */
    message     msg;
    tx_cmd      raw_data;
    file_transfer *file;
    
    recvMessage(&msg, sockfd);
    parseCommand(&msg, &raw_data);  /* parsing messaga */
    file = raw_data.fileTransfer;   /* selecting appropriate structure */
    file_size = file->len;          /* getting file length */
    
	/* Receiving File */
    int bytes_sch; /* scheduled bytes */
	int bytes_left = file_size;
	
    while (bytes_recv < file_size) { /* infinite receive loop */
        if (bytes_left >= TX_DATA_PACKETS)
			bytes_sch = TX_DATA_PACKETS;
		else
			bytes_sch = bytes_left;
		
        if (readn(sockfd, buf, bytes_sch) != bytes_sch) {
            final = FAILURE;
            break;
        }
        /* Barrier to protect against file transmission faliure.
         * In case if file transmission fails, original file 
         * is not over written
         */
        if (file_changed == FALSE) { /* barrier: execute only the first time */
			if ((rec_file = open(file_name, O_WRONLY | O_CREAT, PERMS)) == -1) {
				err_sys("Local file open failed");
				final = FAILURE; break;
			}
			file_changed = TRUE;
		}
		
		/* Writing to local file */
		write(rec_file, buf, bytes_sch);
		
        bytes_recv += bytes_sch;
		bytes_left -= bytes_sch;
	} /* end of while */
	
    if (file_changed == FALSE) {
		close(rec_file);
//        err_log("No file received");
        final = FAILURE;
    } else {
//        err_log("File succesfully received");
        /* Closing the file descriptor. Put it here so that if the
         * file is not changed (and so opened), 'Bus error'
         * does not occur.
         */
        close(rec_file); 
    }
    
    return final;
}

int sendMessage(message *msg, int sockfd) {
    if (writen(sockfd, msg, sizeof(message)) != sizeof(message))
        return FAILURE;
    else
        return SUCCESS;
}

int recvMessage(message *msg, int sockfd) {
    if (readn(sockfd, msg, sizeof(message)) != sizeof(message))
        return FAILURE;
    else
        return SUCCESS;
}

/* Returns number of bytes actually written */
ssize_t writen(int fd, void *buf, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, buf, nleft)) < 0) {
            if (nleft == n)
                return -1; /* error */
            else
                break; /* error, return amount written so far */
        } else if (nwritten == 0)
            break;
        
        nleft   -=  nwritten;
        buf     +=  nwritten;
    }
    return (n - nleft); /* return >= 0 */
}

/* Returns number of bytes actually read */
ssize_t readn(int fd, void *buf, size_t n) {
    size_t  nleft;
    ssize_t nread;
    
    nleft = n;
    while (nleft > 0) {
        if ((nread = read(fd, buf, nleft)) < 0) {
            if (nleft == n)
                return (-1); /* error, return -1 */
            else
                break; /* error, return amount read so far */
        } else if (nread == 0)
            break;  /* EOF */
        nleft   -=  nread;
        buf     +=  nread;
    }
    return (n-nleft); /* return >= 0*/
}
