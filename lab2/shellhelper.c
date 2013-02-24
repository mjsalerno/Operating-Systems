#include <unistd.h>
#include <sys/types.h>
#include "shellhelper.h"
#include <string.h>
#include <stdio.h>

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
    // If something went wrong while trying to execute to program print out an error and quit.
    fprintf(stderr, "An error occured while trying to start '%s'\n", program);
    abort();
  }
}

