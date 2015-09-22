BINS=local remote

all:	$(BINS)

local:	local.c mdump.o
	$(CC) -g $< mdump.o -lrt -o $@

remote:	remote.c mdump.o
	$(CC) -g $< mdump.o -lrt -o $@

mdump.o:	mdump.c
	$(CC) -c -g $? -o $@

clean:
	rm -f $(BINS) *.o cscope.out
