CC = gcc
COMPILE = $(CC) $(FLAGS)

all: client server

client: client.c
	$(COMPILE) -o client client.c

server: server.c
	$(COMPILE) -o server server.c

clean:
	rm -f client
	rm -f server
