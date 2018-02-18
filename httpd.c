/*
 *
 *   HTTPD.C
 *   ==========
 *
 *   Based on echosrv.c by (c) Paul Griffiths, 1999
 *   Email: mail@paulgriffiths.net
 *
 * Simple TCP/IP httpd server.
 *
 */

#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

#include "helper.h"           /*  our own helper functions  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/*  Global constants  */

#define MAX_LINE           (1000)

// Global variables

int debug;

// ssize_t Writeline(int sockd, const void *vptr, size_t n) 

void writeHeader(int sockd, int htmlStatus, int contentLength) {

    char buffer[255];

    sprintf(buffer,"HTTP/1.1 %d OK\n",htmlStatus);
    Writeline(sockd,buffer,strlen(buffer));

    if( debug ) {
        printf("%s", buffer );
    }

    sprintf(buffer,"Content-Type: text/html\n");
    Writeline(sockd,buffer,strlen(buffer));

    if( debug ) {
        printf("%s", buffer );
    }

    sprintf(buffer,"Content-Length: %d\n",contentLength);
    Writeline(sockd,buffer,strlen(buffer));

    if( debug ) {
        printf("%s", buffer );
    }

    sprintf(buffer,"\n\n");
    Writeline(sockd,buffer,strlen(buffer));

    if( debug ) {
        printf("%s", buffer );
    }

}

int writeFile(int sockd, char *file, int htmlStatus) {
    FILE *op;
    char buffer[1024];
    char *rc;
    struct stat st;
    int size;

    op=fopen(file,"r");

    if( (FILE *)NULL != op ) {
        stat(file, &st);
        size = st.st_size;

        writeHeader(sockd,200,size);

        while( !feof(op) ) {
            rc=fgets(buffer,1024,op);
            Writeline(sockd,buffer,strlen(buffer));

            if( debug ) {
                printf("%s", buffer );
            }
        }
        strcpy(buffer,"\n\n");
        Writeline(sockd,buffer,strlen(buffer));

        if( debug ) {
            printf("%s", buffer );
        }
    } else {
        htmlStatus = 404;
    }

    return( htmlStatus);
}

int parsePutGet(char *ptr ) {
    int parsed;
    int i;
    char *tmp;

    i=0;
    do {
        if( i == 0) {
            tmp = (char *)strtok(ptr,"&");
        } else {
            tmp = (char *)strtok(NULL," &/"); 
        }

        if ( tmp != (char *)NULL) {
            if(!strcmp(tmp,"HTTP")) {
                parsed=1;
            } else {
                printf("\n---->%s<\n",tmp);
                parsed=0;
                i++;
            }
        }
    } while( (tmp != (char *)NULL) && (parsed == 0)) ; 

    printf("\n=============================\n");

    return i;
}

void httpResponse(int sock, int code ) {
    char buffer[255];
    int size=0;

    sprintf(buffer,"HTTP/1.1 %d OK\n",code);
    Writeline(sock,buffer,strlen(buffer));

    sprintf(buffer,"Content-Type: text/html\n");
    Writeline(sock,buffer,strlen(buffer));

    if( debug ) {
        printf("%s", buffer );
    }

    sprintf(buffer,"Content-Length: %d\n",size);
    Writeline(sock,buffer,strlen(buffer));

    if( debug ) {
        printf("%s", buffer );
    }

    sprintf(buffer,"\n\n");
    Writeline(sock,buffer,strlen(buffer));

}


int main(int argc, char *argv[]) {
    int       list_s;                /*  listening socket          */
    int       conn_s;                /*  connection socket         */
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
    char opBuffer[255];
    char     *endptr;                /*  for strtol()              */
    struct timeval tv;
    int rc;
    int count=0;
    int runFlag = 1;
    char *ptr;
    int len=0;
    int i;
    int parsed;

    char *tmp;

    debug=1;

    /*  Get port number from the command line, and
     *        set to default port if no arguments were supplied  */

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
     *        zero, and fill in the relevant data members   */

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);


    /*  Bind our socket addresss to the 
     *        listening socket, and call listen()  */

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
     *        to client requests and echo input  */

    while ( 1 ) {

        /*  Wait for a connection, then accept() it  */

        do {
            conn_s = accept(list_s, NULL, NULL);

            if (conn_s == -1) {
                //                perror("accept");
                sleep(1);
            }
        }
        while( conn_s == -1);
        /*  Retrieve an input line from the connected socket
         *            then simply write it back to the same socket.     */

        runFlag = 1;
        while(runFlag) {
            count++;

            bzero(buffer,MAX_LINE);
            rc=Readline(conn_s, buffer, MAX_LINE-1);
            len=strlen(buffer);

            //            printf("Buf=>%s<\n",buffer);
            //            printf("len=%d\n",len);

            ptr=strtok(buffer," \r\n");

            if( (strlen(buffer) > 0)) {
                if( (char *)NULL != ptr) {
                    char *fileName ;
                    char *proto ;
                    //                    printf("Request=>%s<\n", ptr);

                    if(!strcmp(ptr,"GET")) {
                        char file[1024];

                        printf("\nGET =========================\n");
                        tmp = (char *)strtok(NULL,"?");

                        if( (tmp[0] == '/') && (tmp[1] != 0x00 ) ) {
                            printf("Filename\n");
                            strcpy(file,"/var/www/html/default.html");
                            writeFile(conn_s, file,404);
                            runFlag=0;
                        } else {
                            tmp = (char *)strtok(NULL," ");
                            i=parsePutGet(tmp);
                            httpResponse(conn_s, 200 );
                        }

                    } else if(!strcmp(ptr,"PUT")) {
                        printf("\nPUT =========================\n");
                        tmp = (char *)strtok(NULL,"?");
                        tmp = (char *)strtok(NULL," ");
                        i=parsePutGet(tmp);
                        printf("i=%d\n",i);

                        httpResponse(conn_s, 200 );

                        printf("\n=============================\n");
                    } else if(!strcmp(ptr,"HEAD")) {
                        writeHeader(conn_s, 200, 0);
                        runFlag = 0;
                    } else if(!strcmp(ptr,"POST")) {
                        printf("\nPOST=========================\n");
                        httpResponse(conn_s, 501 );
                        printf("\n=============================\n");
                        runFlag = 0;
                    }
                } else {
                    runFlag=0;
                }

            }
            if ( rc == 0 ) {
                runFlag = 0;
            }
        }

        /*  Close the connected socket  */

        if(debug) {
            printf("CLOSE\n");
        }

        if ( close(conn_s) < 0 ) {
            fprintf(stderr, "ECHOSERV: Error calling close()\n");
            exit(EXIT_FAILURE);
        }
    }
}
