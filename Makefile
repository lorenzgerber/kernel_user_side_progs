# Makefile
CC=gcc
CFLAGS= -Wall -std=c99 -Werror -g

all: get put

get: kvs_get.c
	$(CC) $(CFLAGS) -o kvs_get kvs_get.c
put: kvs_put.c
	$(CC) $(CFLAGS) -o kvs_put kvs_put.c

clean:
	rm kvs_get kvs_put
