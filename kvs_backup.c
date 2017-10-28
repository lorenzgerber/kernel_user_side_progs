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
#define BACKUP 4

#define MAX_PAYLOAD 1024 /* maximum payload size*/

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

int main(int argc, char* argv[]){


	struct keyvalue *data;
	FILE *file;


	if(argc != 1){
		fprintf(stderr, "Usage: kvs_backup\n");
		exit(EXIT_FAILURE);
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


	data = malloc(sizeof(struct keyvalue));
	//data->key = atoi(argv[1]);
	data->operation = 4;
	//data->value = malloc(sizeof(char)*strlen(argv[2]));
	//strcpy(data->value, argv[2]);
	memcpy(NLMSG_DATA(nlh), data, sizeof(struct keyvalue));


	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	printf("Doing the back up thing\n");
	sendmsg(sock_fd,&msg,0);
	printf("Waiting for message from kernel\n");

	/* Read message from kernel */
	recvmsg(sock_fd, &msg, 0);
	printf("length of backup message %d\n", nlh->nlmsg_len);
	printf("Received backup message: %s\n", (char *)NLMSG_DATA(nlh));

	file = fopen("keystore.backup", "w");
	char *test = malloc(sizeof(char)*nlh->nlmsg_len);
	memcpy(test, NLMSG_DATA(nlh), nlh->nlmsg_len);
	for (int i = 0; i <= nlh->nlmsg_len; i++){
		fputc(*(test+i), file);
	}
	fclose(file);


	close(sock_fd);
}
