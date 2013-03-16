#ifndef SWISH_H
#define SWISH_H
#include <stdbool.h>
#include "command.h"

#define MAX_ARGS  	 1024
#define MAX_PATH  	 2048
#define MAX_HISTORY  5
#define HISTORY_FILE_NAME ".history"

enum REDIRECT_TYPE{
	PIPE = '|',
	REDIRECT_LEFT = '<',
	REDIRECT_RIGHT = '>',
	REDIRECT_NONE  = '0'
};

typedef enum REDIRECT_TYPE REDIRECT_TYPE;

/**
 * Attemps to parse, and spawn a process based on the command.
 */
void evaluateCommand(char **cmd, int cmdSize, bool *running, char* wd, char** envp, FILE *script, bool *readingScript, bool debug, char *historyList[], int historyPtr,REDIRECT_TYPE *redirects, int rdSize);


void getInput(Command *command, char *prompt, char *wd, int historyPtr, char *historyList[]);

/**
 * Searchs the given string for redirection operators(PIPE_TYPE).
 * @return Returns the number of redirects in the command.
 */
int searchRedirect(char* cmd, REDIRECT_TYPE *redirects);

void moveLeft(Command *command);
void moveRight(Command *command);
void backspace(Command *command, char* prompt, char* wd);
void writeHistoryFile(FILE *historyFile, char *historyList[]);
#endif
