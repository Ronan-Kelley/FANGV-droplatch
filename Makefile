CC=gcc
CFLAGS=-Werror -Wall -Wextra -Isrc
SERVERNAME=droplatch
CLIENTNAME=droplatch-client

build: server client

server: bin/server.o bin/client_server_shared.o
	$(CC) $(CFLAGS) $^ -o bin/$(SERVERNAME)
	
client: bin/client.o bin/client_server_shared.o
	$(CC) $(CFLAGS) $^ -o bin/$(CLIENTNAME)

bin/server.o: src/server.c
	$(CC) $(CFLAGS) -c $^ -o bin/$(basename $(@F)).o

bin/client.o: src/client.c
	$(CC) $(CFLAGS) -c $^ -o bin/$(basename $(@F)).o

bin/client_server_shared.o: src/client_server_shared.c
	$(CC) $(CFLAGS) -c $^ -o bin/$(basename $(@F)).o

#########
# clean #
#########
clean:
	rm -rf bin/*
