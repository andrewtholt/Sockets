/*

*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <unistd.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <mqueue.h>

#define MQ_DEF_MSGSIZE 1024
#define MQ_DEF_MAXMSG 16

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
    int len;
    void *t;
    int rc;

    int sender=-1;

    mqd_t msg;
    struct mq_attr TX;

    TX.mq_msgsize = MQ_DEF_MSGSIZE;
    TX.mq_maxmsg = MQ_DEF_MAXMSG;

    strcpy(address,"127.0.0.1");
    strcpy(queue,"/TX");

    printf("Profibus Sender (Local)\n");

    int   opt;

    while((opt = getopt(argc, argv,"vha:p:m:")) != -1) {
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
                sender=0;
                break;
            case 's':
                sender=-1;
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

    //keep communicating with server
    //
    msg = mq_open( queue, O_RDWR|O_CREAT, 0660, &TX );
    
    if( msg < 0) {
        perror("mq_open");
        exit(-1);
    }


    while(1) {
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

    close(sock);
    return 0;
}
