/*

   ECHOSERV.C
   ==========
   (c) Paul Griffiths, 1999
Email: mail@paulgriffiths.net

Simple TCP/IP echo server.

*/


#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/*  Global constants  */

#define ECHO_PORT          (8080)
#define MAX_LINE           (1000)
#define LISTENQ        (1024)

void usage() {
    printf("Usage: receiver\n");
    printf("\t-h\t\tHelp.\n");
    printf("\t-a <address>\tListen on address (not implemented).\n");
    printf("\t-p <port>\tHelp.\n");
    printf("\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int       list_s;                /*  listening socket          */
    int       conn_s;                /*  connection socket         */
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
    char     *endptr;                /*  for strtol()              */
    struct timeval tv;
    int rc;
    int count=0;
    int runFlag = 1;
    int verbose=0;
    char address[32];
    int i;
    int n;
    int incoming;

    int   opt;

    while((opt = getopt(argc, argv,"ha:p:")) != -1) {
        switch(opt) {
            case 'h':
                usage();
                break;
            case 'a':
                strncpy(address,optarg,32);
                break;
            case 'p':
                port=atoi( optarg );
                break;
            case 'v':
                verbose=-1;
                break;
            default:
                usage();
                break;
        }
    }   

    /*  Get port number from the command line, and
        set to default port if no arguments were supplied  */

    /*  Create the listening socket  */

    if ( (list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        fprintf(stderr, "ECHOSERV: Error creating listening socket.\n");
        exit(EXIT_FAILURE);
    }


    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);


    /*  Bind our socket addresss to the 
        listening socket, and call listen()  */

    if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
        fprintf(stderr, "ECHOSERV: Error calling bind()\n");
        exit(EXIT_FAILURE);
    }

    if ( listen(list_s, LISTENQ) < 0 ) {
        fprintf(stderr, "ECHOSERV: Error calling listen()\n");
        exit(EXIT_FAILURE);
    }
    tv.tv_sec = 0;
    tv.tv_usec= 100;

    rc=setsockopt(list_s, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval));
    printf("setsocketopt=%d\n",rc);

    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    while ( 1 ) {

        /*  Wait for a connection, then accept() it  */

        do {
            conn_s = accept(list_s, NULL, NULL);
        } while( conn_s == -1);


        /*  Retrieve an input line from the connected socket
            then simply write it back to the same socket.     */

        runFlag = 1;

        count=0;
        incoming=0;
        while(runFlag) {

            printf("Input ....\n");
            errno = EAGAIN;
            n = -1; 
            while( (EAGAIN == errno) && (-1 == n) ) { 
                usleep(10);
                n=recv(conn_s,buffer,1,0);
                if( count == 0) {
                    incoming=buffer[0];
                }
            }

            printf("%d bytes recieved\n",n);

            if( (count == 0) && (incoming > 0) ) {
                n=recv(conn_s,buffer,incoming,0);
            }
            count++;

            mdump(buffer,16);

            if ( rc == 0 ) {
                runFlag = 0;
            }
            //            sleep(1);
            /*
               if( rc > 0) {
               Writeline(conn_s, buffer, strlen(buffer));
               if (strlen(buffer) > 0) {
               printf("buffer >%s<\n", buffer);
               }
               }
               */
            //            sleep(1);
        }

        /*  Close the connected socket  */

        if ( close(conn_s) < 0 ) {
            fprintf(stderr, "ECHOSERV: Error calling close()\n");
            exit(EXIT_FAILURE);
        }
    }
}
