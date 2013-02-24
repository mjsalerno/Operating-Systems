#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include "shellhelper.h"

char* parseEnv(char **envp, char *keyword){
  char *cp, *at;
  int i;
  
  for(at = *envp, i = 0; envp+i != NULL; at = (*(envp)+i), i++){
	  cp = strstr(at, keyword);
	  
	  if(cp != NULL)
		break;  
	}
	
	if(envp+i == '\0')
		return NULL;
	else{
		// USE i AND return char pointer from envp
		return strstr(at, "=")+1;
	}
	
}

int spawn(char *program, char **args){
  pid_t pid;
  pid = fork();
  if(pid != 0)
    return pid;
  else{
    execvp(program, args);
    // If something went wrong while trying to execute the program, print out an error and call abort.
    fprintf(stderr, "An error occured while trying to start '%s'\n", program);
    abort();
  }
}

char** argsBuilder(char *filename, int num, ...){
  va_list arguments;
  int i;
  // Build the list of args  
  va_start(arguments, num);
  // Loop through the arguments
  for(i = 0; i < num; i++){
    printf("Arg: %s", va_arg(arguments, char*));
  }  
  // Clean up the list  
  va_end(arguments);  
}

