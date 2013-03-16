#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shellhelper.h"
#include "swish.h"

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

int contains(char *array[], char key){
  for(int i = 0; *(array + i) != '\0'; i++){
    if(strchr (array[i], (int)(key)) != NULL)
      return i;
  }
  return -1;
}

void removeNewline(char *string) {
  for (; *string != '\0'; ++string) {
    if (*string == '\r' || *string == '\n') {
      *string = '\0';
    }

  }
}

void historyDn(int *historyPtr) {
  if(*historyPtr >= (MAX_HISTORY - 1)) {
    *historyPtr = 0;
  } else {
      (*historyPtr)++;
  }
}

void historyUp(int *historyPtr) {
  if(*historyPtr <= 0) {
    *historyPtr = MAX_HISTORY - 1;
  } else {
      (*historyPtr)--;
  }
}

void historyShowDn(int *historyShow, char *historyList[]) {
  if(*historyShow >= (MAX_HISTORY - 1) && *historyList[0] != '\0') {
    *historyShow = 0;
  } else if(*historyList[(*historyShow) + 1] != '\0') {
      (*historyShow)++;
  }

  // printf("historyShow: %d\n", *historyShow);
}

void historyShowUp(int *historyShow, char *historyList[]) {
 if(*historyShow <= 0 && *historyList[MAX_HISTORY - 1] != '\0') {
    *historyShow = MAX_HISTORY - 1;
  } else if( *historyShow >= 1 && *historyList[*historyShow - 1] != '\0'){
      (*historyShow)--;
  } 

  // printf("historyShow: %d\n", *historyShow);
}

void writeHistoryFile(FILE *historyFile, char *historyList[]) {
  if (historyFile == NULL)
    printf("-------------------------THE FILE IS NULL\n");
  
  for (int i = 0; i < MAX_HISTORY; ++i) {
    if (*historyList[i] != '\0') {
      fprintf(historyFile, "%s\n", historyList[i]);
      printf("WROTE: %s\n", historyList[i]);
    }
  }
}

FILE* openHistoryFile(char *rights) {
  char path[MAX_PATH] = {'\0'};
  strcpy(path, getenv("HOME"));
  strcat(path, "/");        
  strcat(path, HISTORY_FILE_NAME); 
  //printf("NEW PATH: %s\n", path);

  return fopen(path, rights);
}

void printWolf(){
  printf("\n");
  printf("                                         ++.                   ``               \n");
  printf("                                         /yys:                 :y/`             \n");
  printf("                                         +yyyys-               +yyy:            \n");
  printf("                                         oyhhhhyo`             syhhys.          \n");
  printf("                                        `yyhhhhhys.           .yyhhhyy-         \n");
  printf("                                        +yyhhohhhyyoooooo++/:-oyhhhhhyy.        \n");
  printf("                            `.-:/++oosssyyhhh`/hhhyyhhhhhhhhhyyyhhhhhhy+        \n");
  printf("                        -osyyyhhhhhhhhhhhhhh+  shhhyyyhhhhhhhhhhhhhhhhyy        \n");
  printf("                         `/yyyhhhhhhhhhhhhhh`  -hhhhhhhhhhhhhhhhhhhhhhyy        \n");
  printf("                    .-/+syyyyyhhhhhhhhhhhhh:    yhhhhhhhhhhhhhhhhhhhhhys        \n");
  printf("               `-/oyyyyhhhhhhhhhhhhhhhhhhho/.`/`ohhhhhhhhhhhhhhhhhhhhhys        \n");
  printf("            ./oyyyhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhyhhhhhyhhhhhhshhhhhhhhhy:       \n");
  printf("         ./syyyhhhhhhhhhysoo+///:::::::::://++oyhhhhhhs//+osyo+yhhhhhhhyy`      \n");
  printf("      `:syyyhhhhhhhhysooooooo+/::::---...:/osyhhhhhhhhhh+/oso+:/hhhhhhhhy+      \n");
  printf("    `/yyyyhhhhhhhhhhhhhhys+/:-..`    `/yhhhhhhhhhhhyyyhhh//hhhy/+yhhhhhhyy      \n");
  printf("   :oooyyyyhhhhhhhhhyo+:-.`    .--:::::/shhhhhyo+/+syhhhhhooyhhhyyhhhhhyy-      \n");
  printf("     -syyhhhhhhhhs+/-......----.--:::+yhhhhs+::::oo++//::/+oyhy+/osyhhhyy:      \n");
  printf("   .oyyhhhhhhhs+::::/++ooosssssssoo+shhhhs//::::::::/+ossoss+/:::/oyhhhhhy+.    \n");
  printf("  /yyyyyhhhysoosyhhhhhhhhhhhhhhhhhhhhhhhhhho::::::+yhhhhy:/syss::::osyhhhhyy+-  \n");
  printf("`oyoyyyhhhhhhhhhhyyyyyysssooossyyhhhhhhhhy//++:::/yyyo/yyyyhy-sy/::::oshhhhhhys:\n");
  printf(":-`oyyhhhhhyyyso+:-.`    `.-. :yyhhhhhhhhhhy+:::::::::+oy++hhhh/y+::::/yyhhhhhyy\n");
  printf("  oyyhhyys+:.`            `.``yyyyyyyhhhhy+::://++//:::::/so-hhsyhy+::/soohhhhy/\n");
  printf(" +yyys/-`                    oys+/yyhhhyo+oyhhhhhhhhhyo+:::+ys+hho.+hooo+yhhyy: \n");
  printf("`y+-`                       `-`  +yhhhhyhhhhhhyyyyyhhhhhyo:::+sh+:osh+yoyhyo-   \n");
  printf("`                               /yyhhhhyso+/::----:/+syyhhy/:::+so:oo:yhyy/     \n");
  printf("                               :yyyo/-.               `+yhhho/::::/+yhhyy.      \n");
  printf("                              -s/-                      .+yyhhhsssshhhys-       \n");
  printf("                              `                           `:oyyhhhhyyo:         \n");
  printf("                                                             `.-:::.`           \n");
  printf("\n");
}

void replaceCommand(char *cmd, Command *command, char* wd, char *prompt){
  // Erase the line
  printf("\r\033[K");
  // Print out the new line
  setCommand(command, cmd);
  printf("%s[%s] %s%s%s%s", BLUE, wd, GREEN, prompt, NONE, command->value);
}


void parseCommand(char *command, char** parsed, int size){
  int i = 0;
  char *index;
  char *token;
  token = strtok(command, " ");
  while(token){
    if(i < size - 1){
      index = strchr(token, '$');      
      parsed[i++] = token;
      if(index != NULL){
        index++;
        parsed[i-1] = getenv(index);
      }
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
  int pipefd[2 * rdSize]/*, fd*/;
  char *args[MAX_ARGS];

  // Create pipes
  for(int i = 0; i < rdSize; i++) {
    if(pipe(pipefd + (i*2)) < 0) {
      printError("An error occurred while trying to create pipes.\n");
      exit(1);
    }
  }
  // Set up variables for forking
  int pid, status;
  // Start spawning children
  for(int i = 0, j = 0; i < cmdSize; i++, j+=2) {
    // Get command to spawn
    parseCommand(commands[i], args, MAX_ARGS);
    printf("%sExecuting c%d - %s%s\n", MAGENTA, i, args[0], NONE);
    // Begin forking
    if((pid = fork()) == 0) {
      setPipes(args, i, j, cmdSize, rdSize, pipefd);
      // Start Child process
      if(execvp(args[0], args) < 0) {
        printf("%sUnable to start the process '%s'%s\n", RED, args[0], NONE);
        // Try as file instead
        close(pipefd[i + STDOUT_FILENO]);
        open(args[0], 'w');
        exit(1);
      }
    }
  }
  // Close Parent pipes
  for(int i = 0; i < 2 * rdSize; i++) {
        close(pipefd[i]);
  }
  // Wait for children
  printf("%sParent is waiting%s\n", MAGENTA, NONE);
  while(wait(&status) != pid); 
}

void setPipes(char **args, int i, int j, int cmdSize, int rdSize, int *pipefd){
  if(i < cmdSize - 1) {
    if(dup2(pipefd[j + 1], STDOUT_FILENO) < 0) {
      printError("An error occurred while trying to use dup2.\n");
      exit(1);
    }
  }
  // Copy pipes
  if(j > 0) {
    if((dup2(pipefd[j - 2], STDIN_FILENO)) < 0) {
      printError("An error occurred while trying to use dup2.\n");
      exit(1);
    }
  }
  // close unused pipes
  for(int i = 0; i < 2 * rdSize; i++) {
    close(pipefd[i]);
  }
}