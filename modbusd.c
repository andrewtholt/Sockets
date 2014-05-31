
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#define TRUE   1
#define FALSE  0

void setnonblocking(int sock) {
	int opts;
    
	opts = fcntl(sock,F_GETFL);
	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0) {
		perror("fcntl(F_SETFL)");
		exit(EXIT_FAILURE);
	}
	return;
}

int main(void)
{
//    extern int errno;
    int n;
    int x;
    int i;
    int nfds;
    int inv_id;
    int proto_id;
    int length;
    int uid;
    int exitFlag = 0;
    
    int on=1;
    
    int opt=TRUE;
    
    char buff[1024];

    fd_set rfds;
    fd_set afds;
    
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(-1 == SocketFD)
    {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }
    
    nfds = getdtablesize();
    FD_ZERO(&afds);
    FD_SET(SocketFD, &afds);
    
    setsockopt(SocketFD,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(opt));
    x=fcntl(SocketFD, F_GETFL,0);	// Get socket flags
    
    
    memset(&stSockAddr, 0, sizeof(stSockAddr));
    
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1502);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
    

    if(-1 == bind(SocketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
    {
        perror("error bind failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    
    if(-1 == listen(SocketFD, 10))
    {
        perror("error listen failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    
    for(;;)
    {
        printf("Waiting on accept\n");
        int ConnectFD = accept(SocketFD, NULL, NULL);
        setnonblocking(ConnectFD);
        exitFlag=0;
        printf("Done waiting\n");
        
        
        if(0 > ConnectFD)
        {
            perror("error accept failed");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }
        
        /* perform read write operations ... 
         read(ConnectFD,buff,size)*/
        
        
        
        while( exitFlag == 0) {
            printf("Input ....\n");
            errno = EAGAIN;
            n = -1;
            while( (EAGAIN == errno) && (-1 == n) ) {
                usleep(10);
                n=recv(ConnectFD,buff,1,0);
                
            } 
            
            printf("%d bytes recieved\n",n);
            
            if ( n  > 0 ) {
                for(i=0;i<n;i++) {
                    printf("<%02x>", buff[i]);
                }
                printf("\n");
            } 
            
            if(n<0 ) {
                perror("What ?");
                printf("%d\n",errno);
                errno=0;
                usleep(1);
            }
            
            if( n == 0 ) {
                printf("Client dropped connection\n");
                if (-1 == shutdown(ConnectFD, SHUT_RDWR))
                {
                    perror("can not shutdown socket");
                    close(ConnectFD);
                    exit(EXIT_FAILURE);
                }
                close(ConnectFD);
                exitFlag=1;
            } else {
                printf("Nothing recieved.\n");
            }
        }
    }
    return EXIT_SUCCESS;  
}