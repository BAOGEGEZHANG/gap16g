CFLAGS += -I ../../include -L ../../lib 

all: nactool

nactool:
	$(CC) $(CFLAGS) -Wall -g nactool.c -lnacapi -o $@
clean:
	rm -f *.o nactool 
