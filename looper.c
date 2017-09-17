#include <stdio.h>
#include <unistd.h>

int main(void){
  for (int i = 1; i < 10; i++){
    sleep(5);
  }
  printf("finished \n"); 
  return 0;
}
