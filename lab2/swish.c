/* CSE 306: Sea Wolves Interactive SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "shellhelper.h"

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_ARGS  1024
#define TRUE  1
#define FALSE 0

int main (int argc, char ** argv, char **envp) {

  int finished = 0;
  char *prompt = "\033[0;32mswish>\033[0m ";
  char cmd[MAX_INPUT];
  char *arguments[MAX_ARGS];
  
  while (!finished) {
    char *cursor;
    char last_char;
    int rv;
    int count;
    
    rv = write(1, prompt, strlen(prompt));
    if (!rv) { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, cursor = cmd, last_char = 1; rv && (++count < (MAX_INPUT-1)) && (last_char != '\n'); cursor++) { 
      rv = read(0, cursor, 1);
      last_char = *cursor;
    } 
    *(cursor-1) = '\0';

    if (!rv) { 
      finished = 1;
      break;
    }
    
    parseCommand(cmd, arguments, MAX_ARGS);
    if(!strcmp(arguments[0], "exit")){
      finished = TRUE;      
    }else{
      spawn(arguments);
    }       
    // Execute the command, handling built-in commands separately 
    // Just echo the command line for now
    // write(1, cmd, strnlen(cmd, MAX_INPUT));
  }

  return 0;
}
