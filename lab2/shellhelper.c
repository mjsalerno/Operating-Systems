#include <unistd.h>
#include <sys/types.h>
#include "shellhelper.h"

char* parseEnv(char **envp, char *keyword){
  return NULL;
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

