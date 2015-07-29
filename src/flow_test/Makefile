flow_test: flow_test.o
	$(CC) -g flow_test.o -L../../lib -lnacapi -lpthread -lcrypto -lssl    -o  flow_test
flow_test.o: 
	$(CC) -c -g flow_test.c -I../../include -g  -o flow_test.o 
clean:
	rm -f *.o flow_test  
