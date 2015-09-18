/*
   C ECHO client example using sockets
   */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <unistd.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdint.h>

#include <stdlib.h>

void usage() {
    printf("Usage: psl\n\n");
    printf("\t-h\t\tHelp.\n");
    printf("\t-a <address>\tAddress to send to, default 127.0.0.1.\n");
    printf("\t-p <port>\tPort Number.\n");
    printf("\n");
    exit(0);
}

int main(int argc , char *argv[]) {
    int sock;
    int port=8888;
    int verbose=0;

    struct sockaddr_in server;
    char message[1000];
    char server_reply[2000];
    char address[32];
    char tmp[1024];
    void *t;

    strcpy(address,"127.0.0.1");

    printf("Profibus Sender (Local)\n");

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

    while(1) {
        printf("Enter message : ");
        scanf("%s" , tmp);

        strcat(tmp,"\n");

        printf("%s\n",tmp);
        printf("%d\n",(int)strlen(tmp));

        t=memcpy(&message[1],tmp,strlen(tmp));
        message[0]=(uint8_t)strlen(tmp);

        mdump(tmp,32);

        //Send some data
        if( send(sock , message , strlen(message)+1 , 0) < 0) {
            puts("Send failed");
            return 1;
        }
    }

    close(sock);
    return 0;
}
