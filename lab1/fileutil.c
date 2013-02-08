#include "mysyscall.h"
#include <stdio.h>

void util_start(void);

asm (".global util_start\r\n"
     "  .type util_start,@function\r\n"
     ".global main\r\n"
     "  .type main,@function\r\n");

/* Get the argc and argv values in the right registers */
asm ("util_start:\r\n"
     "movl %esp, %eax\r\n"
     "addl $4, %eax\r\n"
     "pushl %eax\r\n"
     "subl $4, %eax\r\n"
     "pushl (%eax)\r\n"
     "  call main\r\n");

int contains(char c, char *str){
	char* temp;
	for(temp = str; *temp != '\0'; temp++){
		if(*temp == c) return 1;
	}
	return 0;
}

int
main(int argc, char **argv) {
  int return_code = argc;

  // Exercise 6: Your code here.
	//contains('h',"hello");

  int c = 0, d = 0, h = 0, r = 0, u = 0;
  if(argc != 4){
	MY_PRINT("\nInvalid args\n", 14);
    	return 1;
  }
  if(contains('c', argv[1])) c = 1;
  if(contains('d', argv[1])) d = 1;
  if(contains('h', argv[1])) h = 1;
  if(contains('r', argv[1])) r = 1;
  if(contains('u', argv[1])) u = 1;

	printf("got here\n");

  printf("\nc = %d\nd = %d\nh = %d\nr = %d\nu = %d\n",c,d,h,r,u);


  // Exit.  Until this is implemented,
  // your program will hang or give a segmentation fault.
  MY_SYSCALL1(0, return_code);

  return return_code;
}
