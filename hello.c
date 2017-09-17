#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

long processinfo_sys(void){
	return syscall(398);
}
int main (int argc, char *argv[]){
	long int a = processinfo_sys();
	printf("System call returned %ld\n", a);
	return 0;
}
