BINS=modbusd echoserv

all:	$(BINS)

modbusd:	modbusd.c
	$(CC) -g $? -o $@

echoserv:	echoserv.o helper.o
	$(CC) -g echoserv.o helper.o -o echoserv

echoserv.o:	echoserv.c
	$(CC) -c -g $? -o $@

helper.o:	helper.c
	$(CC) -c -g $? -o $@

clean:
	rm -f $(BINS) *.o cscope.out
