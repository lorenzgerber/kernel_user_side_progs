/*
 * kvs_restore.c
 *
 *  Created on: Oct 28, 2017
 *      Author: knig
 */
#define MAX_PAYLOAD 1024
#define NETLINK_USER 31
#define PUT 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

struct keyvalue {
	int operation;
	int key;
	char *value;
};

struct keyvalue_pair{
	int key;
	char data[];
};

int restore(struct keyvalue_pair* pair);

int main(int argc, char* argv[]){

	char c;
	struct keyvalue_pair* pair = malloc(sizeof(struct keyvalue_pair)+sizeof(char)*1024);


	FILE *fp = fopen("keystore.backup", "ab+");
	if (fp == NULL) {
		printf("Problem opening file");
		return -1;
	}

	// Read contents from file
	rewind(fp);
	c = fgetc(fp);
	int i = 0;
	while (c != EOF && c != '\0'){
		char* keystring = malloc(sizeof(char)*255);
		char* data = malloc(sizeof(char)*1024);
		while(c != '\0'){
			keystring[i] = c;
			c = fgetc(fp);
			i++;
		}
		keystring[i] = '\0';
		c = fgetc(fp);
		i=0;
		while(c != '\0' && c != EOF){
			data[i] = c;
			c = fgetc(fp);
			i++;
		}
		data[i] = '\0';
		i=0;
		pair->key = atoi(keystring);
		strcpy(pair->data, data);
		restore(pair);
		if(c != EOF){
			c = fgetc(fp);
		}
		free(keystring);
		free(data);
	}
	free(pair);
	fclose(fp);
	return 0;
}

int restore(struct keyvalue_pair* pair){

	struct keyvalue *data;

	sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if(sock_fd<0)
		return -1;

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* self pid */

	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;


	data = malloc(sizeof(struct keyvalue));
	data->key = pair->key;
	data->operation = 0;
	data->value = malloc(sizeof(char)*strlen(pair->data)+1);
	strcpy(data->value, pair->data);
	memcpy(NLMSG_DATA(nlh), data, sizeof(struct keyvalue));


	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	printf("Sending message \"%s\" to kernel\n", data->value);
	sendmsg(sock_fd,&msg,0);
	printf("Waiting for message from kernel\n");

	/* Read message from kernel */
	recvmsg(sock_fd, &msg, 0);
	printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));

	close(sock_fd);
	return 0;
}
