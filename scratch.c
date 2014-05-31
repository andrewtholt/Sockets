int main(int argc, char *argv[ ])
{
    char   *service = "echo";
    struct sockaddr_in fsin;
    int    msock;
    fd_set rfds;
    fd_set afds;
    int    alen;
    int    fd, nfds;
    msock = passiveTCP(service, QLEN);
    
    nfds = getdtablesize();
    FD_ZERO(&afds);
    FD_SET(msock, &afds);
    while (1) {
        memcpy(&rfds, &afds, sizeof(rfds));
        if(select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0) {
            errexit("select: %s\n", strerror(errno));
        }
        
        if(FD_ISSET(msock, &rfds))
        {
            int ssock;
            alen = sizeof(fsin);
            ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
            if(ssock < 0)
                errexit("accept: %s\n", strerror(errno));
            FD_SET(ssock, &afds);
        }
        for(fd=0; fd < nfds; ++fd)
            if(fd != msock && FD_ISSET(fd, &rfds))
                if(echo(fd) == 0)
                {
                    (void) close(fd);
                    FD_CLR(fd, &afds);
                }
    }
}
int echo(int fd)
{
    char buf[BUFSIZ];
    int  cc;
    cc = read(fd, buf, sizeof buf);
    if(cc < 0)
        errexit("echo read: %s\n", strerror(errno));
    if(cc && write(fd, buf, cc) < 0)
        errexit("echo write: %s\n", strerror(errno));
    return cc;
}
