#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>



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
	src_addr.nl_pid = pthread_self() << 16 | getpid();

	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = pthread_self() << 16 | getpid();
	nlh->nlmsg_flags = 0;

	data = malloc(sizeof(struct keyvalue));
	data->operation = 4;
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
	if (nlh->nlmsg_len-16==0){
	  printf("Key value stroage is empty\n");
	  file = fopen("keystore.backup", "w");
	  fclose(file);
	  
	} else {
	  file = fopen("keystore.backup", "w");

	  for (int i = 0; i < nlh->nlmsg_len-16; i++){
	    fputc(((char*)NLMSG_DATA(nlh))[i], file);
	  }
	  fclose(file);
	}
	free(nlh);
	free(data);
	close(sock_fd);
}
