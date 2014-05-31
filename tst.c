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
/*
    Returns something like:
    UPT0000027182 (14+2=16 Chars with eol stuff)
*/
int getSystemUptime(int tty,char *ptr) {
    char resBuffer[32];
    int res=-1;
    
    res=write(tty,"UPT\r",4);

    if( res < 0) {
        perror("write");
    }
    printf("Written %d\n",res);

    bzero(resBuffer,32);
    do {
        res=read(tty,resBuffer,32);
    } while (res < 0);

    
    memmove( ptr, (resBuffer+3),29);
    return(strlen(ptr));

}
int getH8Clock(int tty, char *ptr) {
    char resBuffer[32];
    int res=-1;
    
    res=write(tty,"RHC\r",4);
    
    if( res < 0) {
        perror("write");
    }

    bzero(resBuffer,32);
    do {
        res=read(tty,resBuffer,9);
    } while (res < 0);

    memmove( ptr, (resBuffer+1),8); 
    return(strlen(ptr));   
}

int setH8Clock(int tty, char *cmd,char *reply) {
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
    printf("Written %d\n",res);

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
    
    do {
        usleep(1000);
        res=read(tty,ptr,32);
    } while (res < 0);

    return(res);
}

void cmd_purge(tty) {
    char buffer[32];
    int txLen=0;
    int res=0;
    
    strcpy(buffer,"VER\r");
    
    txLen=write(tty,buffer,4);

    do {
        usleep(1000);
        res=read(tty,buffer,32);
    } while (res < 0);
    usleep(1000);
}
/*
    TODO. AT start eat any charcters from SC
*/

int main(int argc, char *argv[])
{   
    char *ttyDev=(char *)NULL;
    int tty=-1;
    int rxLen;
    char buffer[32];
    char *cmd;

    if(ttyDev == (char *)NULL) {
//        ttyDev = strsave("/dev/ttyUSB0");");
        ttyDev = strsave("/dev/tty.usbserial-A600drA9");
    } 
    
    chmod(ttyDev,S_IRUSR|S_IWUSR);
//    (void)umask(S_IRUSR|S_IWUSR);
    tty = open( ttyDev , O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(tty == -1) {
        perror("Error opening serial port ");
        exit(-1);
    }
    
//    printf("Sleeping...\n");
    fflush(stdout);
    usleep(1);
//    printf("... done\n");

//    cmd_purge(tty);
    
    cmd=(char *)basename(argv[0]);
    bzero(buffer,32);

    if(!strcmp(cmd,"sc_version")) {
        rxLen = getVersionNumber(tty,buffer);
    } else if(!strcmp(cmd,"sc_uptime")) {
        rxLen = getSystemUptime(tty,buffer);
    }
    
    printf("%s",buffer);

    
	close(tty);
	return 0;
}

