KBIN_DIR = $(DESTDIR)/usr/local/bin

ALL: test_reg32

test_reg32: test_reg32.o
	$(CC)  test_reg32.o -L../../lib -lnacapi  -o $@
#test_reg16: test_reg16.o
#	$(CC) -m32 test_reg16.o -L../../lib -lnacapi  -o $@
test_reg32.o:
	$(CC)  -c test_reg32.c -I../../include -o test_reg32.o
#test_reg16.o:
#	$(CC) -m32 -c test_reg16.c -I../../include -o test_reg16.o
clean:
	rm -f *.o log* test_reg32 test_reg16
