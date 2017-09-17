#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

long processinfo_sys(int *procs, int* fds, int* sigpends){
	return syscall(398);
}
int main (int argc, char *argv[]){

	int procs = 0;
	int fds = 0;
	int sigpends = 0;


	long int a = processinfo_sys(&procs, &fds, &sigpends);

	printf("Processes: %d\n", procs);
	printf("Watched fds: %d\n", fds);
	printf("Pending Signals: %d\n", sigpends);
	printf("System call returned %ld\n", a);
	return 0;
}
