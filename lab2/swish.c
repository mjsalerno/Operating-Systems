/* CSE 306: Sea Wolves Interactive SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "shellhelper.h"

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_ARGS  1024
#define MAX_PATH  2048

int main (int argc, char ** argv, char **envp) {

  bool finished = false;
  char *prompt = "\033[0;32mswish>\033[0m ";
  char cmd[MAX_INPUT];
  char *arguments[MAX_ARGS];
  setbuf(stdout, NULL);

  while (!finished) {
    char wd[MAX_PATH];

    // Print out the prompt
    getcwd(wd, MAX_PATH);  
    printf("%s[%s]%s %s ", BLUE, wd, NONE, prompt);
    fgets(cmd, MAX_INPUT, stdin);
    strtok(cmd, "\n"); // Thread safe?
    
    // Evaluate the command
    if(strlen(cmd)){
      parseCommand(cmd, arguments, MAX_ARGS);
      if(!strcmp(arguments[0], "exit")){
        finished = true;      
      }else if(!strcmp(arguments[0], "cd")){
        int val = chdir(arguments[1]);
        if(val) printf("Sorry but %s does not exist\n", arguments[1]);
        //STILL NEEDS ~ AND - IMPLEMENTATION
      }else{
        spawn(arguments);
      }
    }       
  }

  return 0;
}
