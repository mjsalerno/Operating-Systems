#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
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

/**
 * Since the name of the program is always in args[0], use args[0] to execute the program.
 */
void spawn(char **args){
  pid_t pid;
  int status;
  // Attempt to fork
  if((pid = fork()) < 0){
    fprintf(stderr, "Unable to fork child process.\n");
  }
  // If the fork was successful attempt to execute the program.   
  else if(pid == 0){
    if(execvp(args[0], args) < 0){
      fprintf(stderr, "An error occured while trying to start '%s'\n", args[0]);
      exit(1);
    }
  }
  else{
    // Wait for the child process to finish
    while(wait(&status) != pid) ;
  }
}

char** argsBuilder(char *filename, int num, ...){
  va_list arguments;
  int i;
  // Build the list of args  
  va_start(arguments, num);
  // Loop through the arguments
  for(i = 0; i < num; i++){
    printf("Arg: %s\n", va_arg(arguments, char*));
  }  
  // Clean up the list  
  va_end(arguments);
  return NULL;  
}

