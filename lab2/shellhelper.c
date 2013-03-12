#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ncurses.h>
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

/**
 * Since the name of the program is always in args[0], use args[0] to execute the program.
 */
void spawn(char **args){
  pid_t pid;
  int status, stdinpipe[2], stdoutpipe[2];
  // Open pipes
  if(pipe(stdinpipe) < 0){
  	printError("Trouble opening the stdin pipe.");
  	return;
  }
  if(pipe(stdoutpipe) < 0){
  	printError("Trouble opening the stdout pipe.");
  	close(stdinpipe[PIPE_READ]);
  	close(stdinpipe[PIPE_WRITE]);
  	return;
  }
  // Fork
  if((pid = fork()) < 0){
  	// Close pipes
    close(stdinpipe[PIPE_READ]);
    close(stdinpipe[PIPE_WRITE]);
    close(stdoutpipe[PIPE_READ]);
    close(stdoutpipe[PIPE_WRITE]);
    printError("Unable to fork child process.\n");
  }
  // If the fork was successful attempt to execute the program.   
  else if(pid == 0){
  	/* This is now in the child */
  	if(dup2(stdinpipe[PIPE_READ], STDIN_FILENO) == -1){ // Redirecting STDIN
  		printError("Trouble using dup2 on the pipe to redirect stdin.\n");
  		return;
  	}
  	if(dup2(stdoutpipe[PIPE_WRITE], STDOUT_FILENO) == -1){ // Redirecting STDOUT
  		printError("Trouble using dup2 on the pipe to redirect stdout.\n");
  		return;
  	}
  	if(dup2(stdoutpipe[PIPE_WRITE], STDERR_FILENO) == -1){ // Redirecting STDERR
      printError("Trouble using dup2 on the pipe to redirect stderr");
      return;
    }
    // Close parent pipes
    close(stdinpipe[PIPE_READ]);
    close(stdinpipe[PIPE_WRITE]);
    close(stdoutpipe[PIPE_READ]);
    close(stdoutpipe[PIPE_WRITE]);
    // Execute
  	if(execvp(args[0], args) < 0){
      printw("An error occurred while trying to start '%s'\n", args[0]);
      refresh();
      exit(1);
    }
  }else{
    /* This is the parent*/
    // Close child pipes
    close(stdinpipe[PIPE_READ]);
    close(stdoutpipe[PIPE_WRITE]); 
    char nchar;
    // Just a char by char read here, you can change it accordingly
    while (read(stdoutpipe[PIPE_READ], &nchar, 1) == 1) {
      printw("%c", nchar);
    }
    printw("\n");
    while(wait(&status) != pid);
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
    printw("Arg: %s\n", va_arg(arguments, char*));
    refresh();
  }  
  // Clean up the list  
  va_end(arguments);
  return NULL;  
}

void printError(char *msg){
	printw("%s", msg);  
	refresh();
}

