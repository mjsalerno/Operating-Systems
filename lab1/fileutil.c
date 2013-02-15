#include "mysyscall.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

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

int countNL(char *buffer){
	int nlCount = 0;
	char *cp = buffer;
	while(*cp != '\0'){
		if(*cp == '\n') nlCount++;
			cp++;
		}
		return nlCount;
}

int findNull(char *buffer){
	char *cp;
	int i = 0;
	for(cp = buffer; *cp != '\0'; cp++){
		i++;
	}
	return i;
}

void intToStr(char* str, int j){
	char *cp = str;
	int temp = 0;
	int i = 0;
	
	if(j == 0){
		str[0] = '0';
		str[1] = '\0';
	}
	
	for(i = 0; j > 0; i++){
		*cp = (j % 10) + '0';
		cp++;
		j /= 10;
	}	
	i--;
	
	while((i - j) > 0){
		str[i] ^= str[j];
		str[j] ^= str[i];
		str[i--] ^= str[j++];
	}
}

void copyBuff(char *buff1, char *buff2, int len){
	len--;
	for(len; len >= 0; len--){
		*buff2 = *buff1;
		buff2++;
		buff1++;
	}
}

int isDosStyle(char *buffer){
	return *(buffer + newlineDist(buffer) + 1) == '\r'; 
}

int newlineDist(char *cp){
	int count = 0;
	while(*cp != '\n' && *cp != '\r' && *cp != '\0'){
		cp++;
		count++;
	}
	return count;
}

void reverseStr(char *str, int len){
	int at = 0;
	while((len - at) > 0){
		str[len] ^= str[at];
		str[at] ^= str[len];
		str[len--] ^= str[at++];
	}
}

void printline(char *string);
int stringlength(char *string);

int main(int argc, char **argv) {
  int return_code = argc;

  // Exercise 6: Your code here.
	//MY_SYSCALL6(192, 0, 4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1,0);
  int c = 0, d = 0, h = 0, r = 0, u = 0;
  if(argc != 4){
	printline("\nInvalid args\n");
    	return 1;
  }
  if(contains('c', argv[1])) c = 1;
  if(contains('d', argv[1])) d = 1;
  if(contains('h', argv[1])) h = 1;
  if(contains('r', argv[1])) r = 1;
  if(contains('u', argv[1])) u = 1;
  
  //printf("\nc = %d\nd = %d\nh = %d\nr = %d\nu = %d\n",c,d,h,r,u);
  
  if(h){
	  printline("\n-c: Count the newlines in the file and output this on stderr (file handle 2)\n");
	  printline("-d: Convert the output to use DOS-style newlines (carriage-return, line feed).\n\tDisplay an error message and help if used with -u.\n");
	  printline("-h: Print a help message and exit.\n");
	  printline("-r: Reverse the contents of the file, on a line-by-line basis.\n");
	  printline("-u: Convert the output to use Unix-style newlines (line feed only).\n\tDisplay an error message and help if used with -d.\n");
	  return 0;
  }
  
  //OPEN FILE
  //--------------------------------------------------------------------
  char buffer[1024] = {'\0'};
  int fd;
	
	if(argv[2][0] == '-'){
		fd = 0;
	}else {
		fd = MY_OFILE(argv[2], O_RDONLY, 0);
	}
	
	if(fd < 0){
		printline("\nno such file\n");
		return 1;
	}	
	
	MY_RFILE(fd, buffer, 1024);
	MY_CFILE(fd);
  //--------------------------------------------------------------------
  
  if(c){
	  
	char intStr[11] = {'\0'};
	int nlCount = countNL(buffer);
	
	intToStr(intStr, nlCount);
	printline("\nnew lines in file: ");
	printline(intStr);
	printline("\n");
  }
  
  if(d){
	if(isDosStyle(buffer)){
		printline("\nthis file is already DOS style.\n");
	}  
	char buffer2[1024] = {'\0'};
	char *cp1 = buffer;
	char *cp2 = buffer2;	
	
	int i = 0;
	
	while(*cp1 != '\0'){
		if(*cp1 == '\n'){
			*cp2 = '\r';
			cp2++;
			*cp2 = '\n';
			cp2++;
			cp1++;			
		}else{
			*cp2 = *cp1;
			cp2++;
			cp1++;
		}
	}
	copyBuff(buffer2, buffer, 1024);
  }
  
  
  if(u){
	  
	  if(!isDosStyle(buffer)){
		printline("\nthis file is already Unix style.\n");
	}
	  
	char buffer2[1024] = {'\0'};
	char *cp1 = buffer;
	char *cp2 = buffer2;	
	
	int i = 0;
	
	while(*cp1 != '\0'){
		if(*cp1 == '\r' && *(cp1 + 1) == '\n'){
			*cp2 = '\n';
			cp2++;
			cp1+=2;			
		}else{
			*cp2 = *cp1;
			cp2++;
			cp1++;
		}
	}
	copyBuff(buffer2, buffer, 1024);
  }
  
  if(r){
	  int nlCount = countNL(buffer);
	  int at = 0;
	  int len = 0;
	  
	  for(;nlCount > 0; nlCount--){
		  
		  len = newlineDist(buffer+at) - 1;
		  
		  //printf("\nNew Line Count: %d\nDistance: %d\n", nlCount, len);
		  
		  reverseStr(buffer+at, len);
		  at += (len+1);
		  if(buffer[at] == '\r'){
			   at += 2;
		   }else{
			   at++;
		   }	  
	  }
  }

//WRITE OUT FILE
//----------------------------------------------------------------------
if(c && !h && !d && !u && !r) return 0;
if(argv[3][0] == '-'){
		fd = 1;
	}else{
	
		fd = MY_OFILE(argv[3], O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP);	
	}
	
	if(MY_WFILE(fd, buffer, findNull(buffer)) < 0){
		printline("\nerror writing to file.\n");
	}
	MY_CFILE(fd);
//----------------------------------------------------------------------


  // Exit.  Until this is implemented,
  // your program will hang or give a segmentation fault.
  MY_SYSCALL1(0, return_code);

  return return_code;
}

void printline(char *string){
	int len = stringlength(string);
	MY_PRINT(string, len + 1);
}

int stringlength(char *string){
	int i = 0;
	char *c;
	
	for(c = string; *c != '\0'; c++){
		i++;
	}
	
	return i;
}
