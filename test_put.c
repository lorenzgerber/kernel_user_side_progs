/*
 * Benchmarking tool for Linux Schedulers
 * Lorenz Gerber, October 2017
 * GPLv3
 */
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>
#include "timer.h"

#define NUMBER_THREADS 10
#define NETLINK_USER 31
#define PUT 0
#define GET 1
#define DELETE 2
#define CLEAR 3

#define MAX_PAYLOAD 1024 /* maximum payload size*/



struct keyvalue {
		int operation;
		int key;
		char *value;
};

// per thread time keeping struct
typedef struct per_thread_time {
  double wall_t_create;
  double wall_t_run;
  double wall_t_finish;
} per_thread_time;

/* Global variables */
void *Pth_empty(void* thread_data);
int array_lengths[NUMBER_THREADS];
char *data_arrays[NUMBER_THREADS];
char *operation[NUMBER_THREADS];
char*op;
struct per_thread_time thread_time [NUMBER_THREADS];

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int kvs_put(int in_key, char* in_value, int my_rank);

int main(int argc, char* argv[]) {

  /* local variables */
  long       thread;
  pthread_t* thread_handles;
  double start, finish;
  int retVal = 0;

  

  thread_handles = malloc(NUMBER_THREADS*sizeof(pthread_t));

  // Create data arrays
  int rank[NUMBER_THREADS];
  for(int i = 0; i < NUMBER_THREADS; i++){
    rank[i] = i;
  }


  // Starting Threads
  for(thread = 0; thread < NUMBER_THREADS; thread++){
    retVal = pthread_create(&thread_handles[thread], NULL, Pth_empty, &rank[thread]);
    GET_WALL_TIME(thread_time[thread].wall_t_create);
    if (retVal){
        fprintf(stderr, "return value pthread_create: %d\n", retVal);
        exit(1);
    }
  }

  // Collecting Threads
  for(thread = 0; thread < NUMBER_THREADS; thread++){
    retVal = pthread_join(thread_handles[thread], NULL);
    if (retVal){
      fprintf(stderr, "return value pthread_join: %d\n", retVal);
      exit(1);
    }
  }

  // Reporting Results  
  for(int i = 0; i < NUMBER_THREADS; i++){
    printf("%d %e %e\n",
        i,
		thread_time[i].wall_t_finish - thread_time[i].wall_t_run,
        thread_time[i].wall_t_finish - thread_time[i].wall_t_create);
	}

  free(thread_handles);

  for(int i = 0; i < NUMBER_THREADS; i++){
    free(data_arrays[i]);
  }

  return 0;
}

/**
 *
 * Thread function
 * Doing read or write, small or large
 * depending on thread rank
 * Benchmarking
 */
void *Pth_empty(void* rank){
  int my_rank = *(int*) rank;

  GET_WALL_TIME(thread_time[my_rank].wall_t_run);
 
  for(int i = 0; i < 100; i++){
    kvs_put(i+100*my_rank, "testos", my_rank);
  }

  GET_WALL_TIME(thread_time[my_rank].wall_t_finish);

  return NULL;
}

int kvs_put(int in_key, char* in_value, int my_rank){


	struct keyvalue *data;

	sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_USER+my_rank);
	if(sock_fd<0)
		return -1;
	printf("sock_fd %d", sock_fd);

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
	data->key = in_key;
	data->operation = 0;
	data->value = malloc(sizeof(char)*strlen(in_value)+1);
	strcpy(data->value, in_value);
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

	free(data->value);
	free(data);
	free(nlh);
	close(sock_fd);
	return 0;
}

