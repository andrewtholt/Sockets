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
#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>

/*  Global constants  */
#define MQ_DEF_MSGSIZE 1024
#define MQ_DEF_MAXMSG 16

#define MAX_LINE 1024
#define LISTENQ        (1024)

void usage() {
    printf("Usage: remote\n");
    printf("\t-h\t\tHelp.\n");
    printf("\t-v\t\tVerbose.\n");
    printf("\t-a <address>\tListen on address (not implemented).\n");
    printf("\t-p <port>\tHelp.\n");
    printf("\t-m <message Q name>\tQueue name.\n");
    printf("\n");
    printf("\t-r\t\tReceiver(default).\n");
    printf("\t-s\t\tSender.\n");
    printf("\n");
    printf("Note\n");
    printf("\tThe value in /proc/sys/fs/mqueue/msgsize_default must be\n" );
    printf("\tThe same on the sending and receiving systems\n");
    printf("\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int       list_s;                /*  listening socket          */
    int       conn_s;                /*  connection socket         */
    short int port=8888;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
    char     *endptr;                /*  for strtol()              */
    struct timeval tv;
    int rc;
    int runFlag = 1;
    int verbose=0;
    char address[32];
    char queue[32];
    int i;
    int n;
    int incoming;
    int   opt;
    int len;
    int oflag;

    char tmp[1024];
    char message[1000];
    void *t;

    mqd_t msg;
    struct mq_attr TX;
    struct mq_attr oldTX;

    int sender=0;

    strcpy(address,"127.0.0.1");
    strcpy(queue,"/Remote");

    while((opt = getopt(argc, argv,"vha:p:m:sr")) != -1) {
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
            case 'm':
                strncpy(queue,optarg,32);
                break;
            case 'v':
                verbose=-1;
                break;
            case 'r':
                sender=0;
                break;
            case 's':
                sender=1;
                break;
            default:
                usage();
                break;
        }
    }

    if(verbose) {   
        printf("Address :%s\n",address);
        printf("Port    :%d\n",port);
        printf("Queue   :%s\n",queue);

        if(sender) {
            printf("Sender\n");
        } else {
            printf("Receiver\n");
        }
        printf("\n");
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

    TX.mq_msgsize = MQ_DEF_MSGSIZE;
    TX.mq_maxmsg = MQ_DEF_MAXMSG;
    TX.mq_flags = 0;

    if(sender == 1) {
        oflag = O_RDONLY;
    } else {
        oflag = O_WRONLY;
    }
    oflag |= O_CREAT;

    msg = mq_open( queue, oflag, 0660, &TX);

    if ( msg < 0 ) {
        perror("mq_open");
        exit(-1);
    }

    rc=mq_getattr(msg, &TX);
    if( rc == 0) {
        printf("mq_msgsize %d\n", TX.mq_msgsize);
    }
    while ( 1 ) {

        /*  Wait for a connection, then accept() it  */

        do {
            conn_s = accept(list_s, NULL, NULL);
        } while( conn_s == -1);


        /*  Retrieve an input line from the connected socket
            then simply write it back to the same socket.     */

        runFlag = 1;

        incoming=0;
        printf("Connected ....\n");
        while(runFlag) {

            if(sender == 0) {
                errno = EAGAIN;
                n = -1; 
                while( (EAGAIN == errno) && (-1 == n) ) { 
                    usleep(10);
                    n=recv(conn_s,buffer,1,0);
                    incoming=buffer[0];
                }

                if(verbose) {
                    printf("%d bytes recieved ...\n",n);
                    printf("... %d bytes expected\n",incoming);
                }

                if( incoming > 0 ) {
                    n=recv(conn_s,buffer,incoming,0);

                    if(n > 0) {
                        mdump(buffer,16);
                    }
                }
            } else {
//                len = mq_receive( msg, &tmp[0], sizeof(tmp), 0);
                len = mq_receive( msg, &tmp[0], 1024, 0);

                if( len < 0) {
                    perror("mq_receive");
                    exit(-1);
                }
            }

            printf("length = %d\n",len);
            t=memcpy(&message[1],tmp,len);
            message[0]=(uint8_t)len;
            mdump(tmp,32);

//            if( send(conn_s , message , len+1 , 0) < 0) {
            if( send(conn_s , message , len+1 , MSG_NOSIGNAL) < 0) {
                printf("Send Failed\n");
                runFlag = 0;
            }


            /*
            if ( rc == 0 ) {
                runFlag = 0;
            }
            */
        }

        /*  Close the connected socket  */


        if ( close(conn_s) < 0 ) {
            fprintf(stderr, "ECHOSERV: Error calling close()\n");
            exit(EXIT_FAILURE);
        }
        printf("Disconnected\n");
    }
}
