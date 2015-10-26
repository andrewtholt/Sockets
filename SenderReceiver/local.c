/*

*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <unistd.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdint.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <mqueue.h>
#include <errno.h>

#define MQ_DEF_MSGSIZE 1024
#define MQ_DEF_MAXMSG 16

#define MAX_LINE 1024

void usage() {
    printf("Usage: local\n\n");
    printf("\t-h\t\tHelp.\n");
    printf("\t-v\t\tVerbose.\n");
    printf("\t-a <address>\tAddress to send to, default 127.0.0.1.\n");
    printf("\t-p <port>\tPort Number.\n");
    printf("\t-m <message q>\tName of message queue.\n");
    printf("\n");
    printf("\t-r\t\tReceiver.\n");
    printf("\t-s\t\tSender (default).\n");
    printf("\n");
    printf("\tThe value in /proc/sys/fs/mqueue/msgsize_default must be\n" );
    printf("\tThe same on the sending and receiving systems\n");
    printf("\n");
    exit(0);
}

int main(int argc , char *argv[]) {
    int sock;
    int port=8888;
    int verbose=0;
    char queue[32];

    struct sockaddr_in server;
    char message[1000];
    char server_reply[2000];
    char address[32];
    char tmp[1024];
    char buffer[MAX_LINE];

    int len;
    void *t;
    int rc;
    int oflag;

    bool sender=true;

    int n;
    int incoming;
    mqd_t msg;
    struct mq_attr TX;

    strcpy(address,"127.0.0.1");
    strcpy(queue,"/Local");

    printf("Profibus Sender (Local)\n");

    int   opt;

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
            case 'v':
                verbose=-1;
                break;
            case 'm':
                strncpy(queue,optarg,32);
                break;
            case 'r':
                sender=false;
                break;
            case 's':
                sender=true;
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

        rc=mq_getattr(msg, &TX);
        if (rc == 0) {
            printf("mq_msgsize %d\n", TX.mq_msgsize);
        }
    }


    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);

    if (sock == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    TX.mq_msgsize = MQ_DEF_MSGSIZE;
    TX.mq_maxmsg = MQ_DEF_MAXMSG;
    TX.mq_flags = 0;

    if(sender) {
        oflag=O_RDONLY;
    } else {
        oflag=O_WRONLY;
    }
    oflag |= O_CREAT;

    msg = mq_open( queue, oflag, 0660, &TX );

    if( msg < 0) {
        perror("mq_open");
        exit(-1);
    }


    while(1) {
        if ( sender == false) {
            printf("Input ....\n");
            errno = EAGAIN;
            n = -1;

            while( (EAGAIN == errno) && (-1 == n) ) {
                usleep(10);
                n=recv(sock,buffer,1,0);
                incoming=buffer[0];

                if ( errno != EAGAIN) {
                    perror("recv");
                    exit(-2);
                }
            }

            if(verbose) {
                printf("%d bytes recieved ...\n",n);
                printf("... %d bytes expected\n",incoming);
            }

            if( incoming > 0 ) { 
                n=recv(sock,buffer,incoming,0);

                if(n > 0) {
                    mdump(buffer,32);
                }
                rc = mq_send( msg,buffer,incoming,0);
            }
        } else {
            len = mq_receive( msg, &tmp[0], sizeof(tmp), 0);

            if( len < 0) {
                perror("sender");
                exit(-1);
            }

            printf("length = %d\n",len);
            t=memcpy(&message[1],tmp,len);

            message[0]=(uint8_t)len;

            mdump(tmp,32);

            //Send some data
            if( send(sock , message , len+1 , 0) < 0) {
                puts("Send failed");
                return 1;
            }
        }
    }

    close(sock);
    return 0;
}
