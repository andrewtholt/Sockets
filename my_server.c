/* simple-server.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

/*****************************************************************************/
/*** simple-server.c                                                       ***/
/***                                                                       ***/
/*****************************************************************************/

/**************************************************************************
*	This is a simple echo server.  This demonstrates the steps to set up
*	a streaming server.
**************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>

#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MY_PORT		9999
#define MAXBUF		1024

char *strsave(char *s)
{
    char           *p;

    if ((p = (char *) malloc(strlen(s) + 1)) != NULL)
        strcpy(p, s);
    return (p);
}

int rawCommand(int tty,char *rx, char *tx) {
    int res;
    
    res=write(tty,rx,strlen(rx)-1);
    bzero(tx,32);
    do {
        res=read(tty,tx,32);
    } while (res < 0);
    
    return(res);
}

/* Arm Watchdog
*/
int armWatchDog(int tty, char *ptr) {
    char resBuffer[32];
    char *resp;
    
    int res=-1;
    
    res=write(tty,"AWD\r",4);

    if( res < 0) {
        perror("write");
    }
    bzero(ptr,32);
    do {
        res=read(tty,ptr,32);
    } while (res < 0);

    /* Should return ARM\r\n */
    
    resp = (char *)strtok(ptr,"\r");
    res = strlen(ptr);
    
    if( res != 3 ) {
        res = -1;
    }
    
    return(res);
}

int getResetReason(int tty, char *ptr) {
    char resBuffer[32];
    int res=-1;
    char *tmp;
    
    res=write(tty,"GRR\r",4);

    if( res < 0) {
        perror("write");
    }
    bzero(resBuffer,32);
    do {
        res=read(tty,resBuffer,32);
    } while (res < 0);

    tmp=(char *)strtok(resBuffer,"\r");
    
    res=strlen(tmp);

    strncpy(ptr,tmp,res);
    return(res);
}

/*
    Returns something like:
    UPT0000027182 (14+2=16 Chars with eol stuff)
*/
int getSystemUptime(int tty,char *ptr) {
    char resBuffer[32];
    int res=-1;
    char *tmp;
    
    res=write(tty,"UPT\r",4);

    if( res < 0) {
        perror("write");
    }

    bzero(resBuffer,32);
    do {
        res=read(tty,resBuffer,32);
    } while (res < 0);

    tmp=(char *)strtok(resBuffer,"\r");
    
    memmove( ptr, (tmp+3),29);
    
    res=strlen(ptr);
    
    if (res != 10) {
        res=-1;
    }
    return(res);

}
int getH8Clock(int tty, char *ptr) {
    char resBuffer[32];
    int res=-1;
    char *tmp;
    
    res=write(tty,"RHC\r",4);
    
    if( res < 0) {
        perror("write");
    }

    bzero(resBuffer,32);
    do {
        res=read(tty,resBuffer,9);
    } while (res < 0);

    tmp=(char *)strtok(resBuffer,"\r");
    memmove( ptr, (tmp+1),8); 
    
    res = strlen(ptr);
    
    if (res != 6 ) {
        res=-1;
    }
    
    return(res);   
}

int setH8Clock(int tty, char *cmd,char *reply) {
    int res=-1;
    char *tmp;
    
    res=write(tty,cmd,strlen(cmd)-1);

    if( res < 0) {
        perror("write");
    }

    bzero(reply,32);
    do {
        res=read(tty,reply,9);
    } while (res < 0);
    
    tmp=(char *)strtok(reply,"\r");
    res=strlen(tmp);
    
    if(res !=3) {
        res=-1;
    }
    return(res);
}

int sh4Off(int tty, char *cmd,char *reply) {
    int res=-1;
    
    res=write(tty,cmd,strlen(cmd)-1);

    if( res < 0) {
        perror("write");
    }

    bzero(reply,32);
    do {
        res=read(tty,reply,9);
    } while (res < 0);
}


int getSystemState(int tty, char *ptr) {
    char resBuffer[32];

    int res=-1;

    res=write(tty,"GSS\r",4);

    if( res < 0) {
        perror("write");
    }
//    printf("Written %d\n",res);

    bzero(resBuffer,32);
    do {
        res=read(tty,resBuffer,9);
    } while (res < 0);

    memmove( ptr, (resBuffer+3),6);
    
    printf("Read %s\n",ptr);
    return(6);
}

int getVersionNumber(int tty, char *ptr) {
    int res=-1;
    
    res=write(tty,"VER\r",4);

    if( res < 0) {
        perror("write");
    }
//    printf("Written %d\n",res);
    
    
    
    do {
        usleep(1000);
        res=read(tty,ptr,32);
    } while (res < 0);

    return(res);
}

/*
    TODO. AT start eat any charcters from SC
*/

int main(int argc, char *argv[])
{   int sockfd;
	struct sockaddr_in self;
	char netRxBuffer[MAXBUF];
    char netTxBuffer[MAXBUF];
    int  opt;
    int  localhost=-1;
    int raw=0;
    uid_t me;
    int port = MY_PORT;
    
    char *ttyDev=(char *)NULL;
    
    me = getuid();
    
    while((opt = getopt(argc,argv,"d:nrhp:")) != -1) {
        switch(opt) {
            case 'h':
                printf("Help\n\n");
                printf("Usage: %s -d <device>|-n|-h\n",argv[0]);
                printf("\t-d <device>\n");
                printf("\t-n\tNetwok\n");
                printf("\t-r\tRaw.  Pipe directly to & from SC.\n");
                printf("\t-h\tHelp\n");
                printf("\n");
                exit(0);
                break;
            case 'n':
                printf("Network\n");
                localhost=0;
                break;
            case 'r':
                printf("Raw\n");
                raw=-1;
                break;
            case 'd':
                printf("Device.\n");
                ttyDev=strsave(optarg);
                break;
            case 'p':
                printf("Poer\n");
                port = atoi( optarg );
                break;
            default:
                printf("Default\n");
                break;
        }
    }

    if(me != 0) {
        printf("\n\tNeed to be root to run this program, sorry.\n\n");
//        exit(0);
    }
    ssize_t rx,tx;
    
    int tty=-1;
    printf("Port is %d\n", port);
    
    int runFlag = -1;
    int rxLen = -1;

    
	/*---Create streaming socket---*/
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		exit(errno);
	}

	/*---Initialize address/port structure---*/
	bzero(&self, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(MY_PORT);
    if(localhost == 0) {
        self.sin_addr.s_addr = INADDR_ANY;
    } else {
        self.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
	/*---Assign a port number to the socket---*/
    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
	{
		perror("socket--bind");
		exit(errno);
	}

	/*---Make it a "listening socket"---*/
	if ( listen(sockfd, 20) != 0 )
	{
		perror("socket--listen");
		exit(errno);
	}

    if(ttyDev == (char *)NULL) {
        ttyDev = strsave("/dev/ttyUSB0");
    } 
    
    chmod(ttyDev,S_IRUSR|S_IWUSR);
//    (void)umask(S_IRUSR|S_IWUSR);
    tty = open( ttyDev , O_RDWR | O_NOCTTY | O_NONBLOCK|O_EXCL);
    if(tty == -1) {
        perror("Error openeing serial port ");
        exit(-1);
    }
    rxLen=read(tty,netRxBuffer,MAXBUF);
    
	/*---Forever... ---*/
	while (1)
	{	int clientfd;
		struct sockaddr_in client_addr;
		int addrlen=sizeof(client_addr);

		/*---accept a connection (creating a data pipe)---*/
		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
		printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        while(runFlag) {
            rx = recv(clientfd, netRxBuffer, MAXBUF, 0);
            /* TODO - map input to uppaer case */
            
            if (rx == 0) {
                runFlag = 0;
            } else {
                bzero(netTxBuffer,MAXBUF);
                
                if(raw) {
                    rxLen=rawCommand(tty,netRxBuffer,netTxBuffer);
                } else {    
                    if(!strncmp(netRxBuffer,"AWD",3)) {
                        rxLen = armWatchDog(tty,netTxBuffer);
                    } else if(!strncmp(netRxBuffer,"VER",3)) {
                        rxLen=getVersionNumber(tty,netTxBuffer);
                    } else if(!strncmp(netRxBuffer,"GRR",3)) {   
                        rxLen=getResetReason(tty,netTxBuffer);
                    } else if(!strncmp(netRxBuffer,"GSS",3)) {   
                        rxLen=getSystemState(tty,netTxBuffer);
                    } else if(!strncmp(netRxBuffer,"UPT",3)) { 
                        rxLen=getSystemUptime(tty,netTxBuffer);
                    } else if(!strncmp(netRxBuffer,"RHC",3)) {
                        rxLen=getH8Clock(tty, netTxBuffer) ;
                    } else if(!strncmp(netRxBuffer,"SHC",3)) {
                        rxLen=setH8Clock(tty,netRxBuffer,netTxBuffer);
                    } else {
                        strcpy(netTxBuffer,"What?\n");
                    }
                }
                
                if( rxLen > 0) {
                    tx=send(clientfd, netTxBuffer, rxLen, 0);
                } else {
                    tx=send(clientfd,"CMD_ERROR",9,0);
                }
                tx=send(clientfd,"\r\n",2,0);

            }
        }
        runFlag=-1;
		/*---Close data connection---*/
		close(clientfd);
	}

	/*---Clean up (should never get here!)---*/
	close(sockfd);
	return 0;
}

