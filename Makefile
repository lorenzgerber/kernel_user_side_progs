# Makefile
CC=gcc
CFLAGS= -Wall -std=c99 -Werror -g

all: get put delete backup

get: kvs_get.c
	$(CC) $(CFLAGS) -o kvs_get kvs_get.c
put: kvs_put.c
	$(CC) $(CFLAGS) -o kvs_put kvs_put.c
delete: kvs_delete.c
	$(CC) $(CFLAGS) -o kvs_delete kvs_delete.c
backup: kvs_backup.c
	$(CC) $(CFLAGS) -o kvs_backup kvs_backup.c

clean:
	rm kvs_get kvs_put kvs_delete kvs_backup
