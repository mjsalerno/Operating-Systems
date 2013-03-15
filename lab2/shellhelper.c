#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
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
		return strstr(at, "=")+1;
	}
	
}

int contains(char *array[], char *key, int size){
  for(int i = 0; i < size; i++){
    if(strcmp(array[i], key) == 0)
      return i;
  }
  return 0;
}

void removeNewline(char *string) {
  for (; *string != '\0'; ++string) {
    if (*string == '\r' || *string == '\n') {
      *string = '\0';
    }
  }
}

void parseCommand(char *command, char** parsed, int size){
  int i = 0;
  char *token;
  token = strtok(command, " ");
  while(token){
    if(i < size - 1){
      parsed[i++] = token;
      token = strtok(NULL, " "); 
    }else{
      printError("Too Many Arguments provided.\n");
    }
  }
  // Append the NULL as the last argument in the array
  parsed[i] = NULL;
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

void printError(char *msg){
	fprintf(stderr, "%s%s%s\n", RED, msg, NONE);
}

/**
 * Since the name of the program is always in args[0], use args[0] to execute the program.
 */
void spawn(char **args){
  pid_t pid;
  int status;
  // Check pid error code
  if((pid = fork()) < 0){
    printError("Unable to fork child process.\n");
  }
  // If the fork was successful attempt to execute the program.   
  else if(pid == 0){
      /* This is now in the child */
      if(execvp(args[0], args) < 0){
          fprintf(stderr, "%sAn error occured while trying to start '%s'%s\n", RED, args[0], NONE);
          exit(1);
      }    
  }else{
      /* This is the parent*/
      while(wait(&status) != pid); 
  }
}

void spawnRedirect(char **commands, int cmdSize, REDIRECT_TYPE *redirects, int rdSize)
{
  int pipefd[2 * rdsize];
  
}

void setChildRedirection(REDIRECT_TYPE *redirects, int *pipefd, int ri){
    if(redirects[ri] != REDIRECT_NONE){
        switch(redirects[ri]){
            case PIPE:
                printf("%sSetting up the pipe operator in the child.%s\n", MAGENTA, NONE);
                
                break;
            case REDIRECT_LEFT:
                printf("%sSetting up the redirect left operator in the child.%s\n", MAGENTA, NONE);
                break;
            case REDIRECT_RIGHT:
                printf("%sSetting up the redirect right operator in the child.%s\n", MAGENTA, NONE);
                break;
            default:
                printf("%sUnknown redirection operator in child '%d'%s\n", RED, redirects[ri], NONE);
                break;
        }
    }else{
        printf("%sNo redirection operator.%s\n", MAGENTA, NONE);
    }
}

void setParentRedirection(REDIRECT_TYPE *redirects, int *pipefd, int ri){
    if(redirects[ri] != REDIRECT_NONE){
        switch(redirects[ri]){
            case PIPE:
                printf("%sSetting up the pipe operator in the parent.%s\n", MAGENTA, NONE);
                
                break;
            case REDIRECT_LEFT:
                printf("%sSetting up the redirect left operator in the parent.%s\n", MAGENTA, NONE);
                break;
            case REDIRECT_RIGHT:
                printf("%sSetting up the redirect right operator in the parent.%s\n", MAGENTA, NONE);
                break;
            default:
                printf("%sUnknown redirection operator in parent'%d'%s\n", RED, redirects[ri], NONE);
                break;
        }
    } 
}