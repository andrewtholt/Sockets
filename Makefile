CCC=g++
# BINS=forked-server modbusd client httpd httpd_1 httpd_2 # echoserv
BINS=forked-server client echoserv

all:	$(BINS)

client:	client.c
	$(CC) -g $? -o $@

modbusd:	modbusd.c
	$(CC) -g $? -o $@

echoserv:	echoserv.o helper.o 
	$(CC) -g echoserv.o helper.o -o echoserv

forked-server:	forked-server.c helper.o
	$(CCC) -g forked-server.c helper.o -o forked-server

echoserv.o:	echoserv.c 
	$(CC) -c -g $? -o $@

helper.o:	helper.c
	$(CC) -c -g $? -o $@

clean:
	rm -f $(BINS) *.o cscope.out

httpd:	httpd.o helper.o
	$(CC) -g httpd.o helper.o -o httpd

httpd_1:	httpd_1.o helper.o
	$(CC) -g httpd_1.o helper.o -o httpd_1

httpd_1.o:	httpd_1.c
	$(CC) -c -g $? -o $@

httpd_2:	httpd_2.o 
	$(CC) -g httpd_2.o -o httpd_2

httpd_2.o:	httpd_2.c
	$(CC) -c -g $? -o $@
	
httpd.o:	httpd.c
	$(CC) -c -g $? -o $@
