BINS=sender receiver

all:	$(BINS)

sender:	sender.c mdump.o
	$(CC) -g $< mdump.o -o $@

receiver:	receiver.c mdump.o
	$(CC) -g $< mdump.o -o $@

mdump.o:	mdump.c
	$(CC) -c -g $? -o $@

clean:
	rm -f $(BINS) *.o cscope.out
