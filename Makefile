CC = gcc
CFLAGS = -Wall -Wextra

server: index.c network.c utils.c
	$(CC) $(CFLAGS) index.c network.c utils.c -o server

clean:
	rm -f server
