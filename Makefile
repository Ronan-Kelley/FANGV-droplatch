CC=gcc
CFLAGS=-Werror -Wall -Wextra -Isrc
BINNAME=droplatch

build: bin/server.o
	$(CC) $(CFLAGS) $^ -o bin/$(BINNAME)
	
bin/server.o: src/server.c
	$(CC) $(CFLAGS) -c $^ -o bin/$(basename $(@F)).o

run: build
	./bin/droplatch

clean:
	rm -rf bin/*
