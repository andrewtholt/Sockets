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

#include "helper.h"           /*  our own helper functions  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*  Global constants  */

#define ECHO_PORT          (2002)
#define MAX_LINE           (1000)

// ssize_t Writeline(int sockd, const void *vptr, size_t n) 

ssize_t WriteFile(int sockd, char *file) {
    FILE *op;
    char buffer[1024];
    char *rc;

    op=fopen(file,"r");
    if( (FILE *)NULL != op ) {
        strcpy(buffer,"HTTP/1.1 200 OK\n");

        Writeline(sockd,buffer,strlen(buffer));
        while( !feof(op) ) {
            rc=fgets(buffer,1024,op);
            Writeline(sockd,buffer,strlen(buffer));
        }
    } else {
        strcpy(buffer,"HTTP/1.0 404 Not Found\n");
        Writeline(sockd,buffer,strlen(buffer));
    }
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
    char *ptr;

    /*  Get port number from the command line, and
        set to default port if no arguments were supplied  */

    if ( argc == 2 ) {
        port = strtol(argv[1], &endptr, 0);
        if ( *endptr ) {
            fprintf(stderr, "ECHOSERV: Invalid port number.\n");
            exit(EXIT_FAILURE);
        }
    }
    else if ( argc < 2 ) {
        port = ECHO_PORT;
    }
    else {
        fprintf(stderr, "ECHOSERV: Invalid arguments.\n");
        exit(EXIT_FAILURE);
    }
    printf("Started.\n");

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
    tv.tv_sec = 1;
    tv.tv_usec= 0;

    rc=setsockopt(list_s, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval));
    printf("setsocketopt=%d\n",rc);

    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    while ( 1 ) {

        /*  Wait for a connection, then accept() it  */

        do {
            conn_s = accept(list_s, NULL, NULL);

            if (conn_s == -1) {
                perror("accept");
                sleep(1);
            }
        }
        while( conn_s == -1);
        /*  Retrieve an input line from the connected socket
            then simply write it back to the same socket.     */

        runFlag = 1;
        while(runFlag) {
            count++;

            rc=Readline(conn_s, buffer, MAX_LINE-1);
            printf("count=%d\trc=%d\n",count,rc);

            ptr=strtok(buffer," ");

            if( (char *)NULL != ptr) {
                char *fileName ;
                char *proto ;
                printf("Request=>%s<\n", ptr);

                if(!strcmp(ptr,"GET")) {
                    char file[1024];
                    fileName = (char *)strtok(NULL," ");
                    proto    = (char *)strtok(NULL," ");

                    printf("\t>%s<\n", fileName);
                    printf("\t>%s<\n", proto);

                    if( !strcmp(fileName,"/")) {
                        strcpy(file,"/var/www/httpd/index.html");
                    } else {
                        sprintf(file,"/var/www/httpd%s",fileName);
                    }
                    printf("\t>%s<\n", file);
                    WriteFile(conn_s, file);
                }
            }

            if ( rc == 0 ) {
                runFlag = 0;
            }
            /*
            sleep(1);
            if( rc > 0) {
                Writeline(conn_s, buffer, strlen(buffer));
            }
            */
        }

        /*  Close the connected socket  */

        if ( close(conn_s) < 0 ) {
            fprintf(stderr, "ECHOSERV: Error calling close()\n");
            exit(EXIT_FAILURE);
        }
    }
}
