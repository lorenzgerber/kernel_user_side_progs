#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define NETLINK_USER 31
#define PUT 0
#define GET 1
#define DELETE 2
#define CLEAR 3

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

/*operation: 0 = store, 1 = lookup, 2 = delete, 3 = clear */
struct keyvalue {
		int operation;
		int key;
		char value[100];
};

int main(int argc, char* argv[]){

	if(argc != 3){
		perror("USAGE: kvs_put [key] [value]\n");
	}

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

	struct keyvalue *test;
	test = malloc(sizeof(struct keyvalue));
	test->key = atoi(argv[1]);
	test->operation = 0;
	strcpy(test->value, argv[2]);
	memcpy(NLMSG_DATA(nlh), test, sizeof(struct keyvalue));


	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	printf("Sending message \"%s\" to kernel\n", test->value);
	sendmsg(sock_fd,&msg,0);
	printf("Waiting for message from kernel\n");

	/* Read message from kernel */
	recvmsg(sock_fd, &msg, 0);
	printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));

	close(sock_fd);
}