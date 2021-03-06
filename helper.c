/*

   HELPER.C
   ========
   (c) Paul Griffiths, 1999
Email: mail@paulgriffiths.net

Implementation of sockets helper functions.

Many of these functions are adapted from, inspired by, or
otherwise shamelessly plagiarised from "Unix Network
Programming", W Richard Stevens (Prentice Hall).

*/

#include "helper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


/* Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, int maxlen) {
    ssize_t         n, rc;
    char            c, *buffer;
    int count = 0;

    buffer = (char *)vptr;

    for (n = 1; n < maxlen; n++) {

        if((rc=recv(sockd, &c, 1, 0)) == 1)  {
            //            printf("A\n");
            *buffer++ = c;
            count++;
#ifdef DEBUG
            if( c >= 0x20 && c <=0x7f) {
                printf(":%c: <%02x>",c, c);
            } else {
                printf(":.: <%02x>", c);
            }
#endif
            if (c == '\n') {
                break;
            }
        } else if (rc == 0) {
            printf("B\n");
            if (n == 1)
                return 0;
            else
                break;
        } else {
            if (errno == EINTR)
                continue;
            if( errno == EAGAIN)
                break;
            return -1;
        }
    }

    *buffer = 0;
    return n;
}

/* Write a line to a socket  */

int Writeline(int sockd, const void *vptr, int n) {
    size_t          nleft;
    ssize_t         nwritten;
    const char     *buffer;

    buffer = (char *)vptr;
    nleft = n;

    while (nleft > 0) {
        if ((nwritten = write(sockd, buffer, nleft)) <= 0) {
            if (errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }
        nleft -= nwritten;
        buffer += nwritten;
    }
    return n;
}
