all: server1 server2
	
server1: server1.c
	@gcc -o server1 server1.c

server2: server2.c
	@gcc -o server2 server2.c

.PHONY: clean

clean:	
	@rm -f server1
	@rm -f server2