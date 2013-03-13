#ifndef SWISH_H
#define SWISH_H
#include <stdbool.h>
#include "command.h"

#define MAX_ARGS  1024
#define MAX_PATH  2048

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
void evaluateCommand(char *cmd, bool *running, char* wd, char** envp, FILE *script, bool *readingScript, bool debug, REDIRECT_TYPE *redirects, int redirectIndex, bool two);

void getInput(Command *command, char *prompt, char *wd);

/**
 * Searchs the given string for redirection operators(PIPE_TYPE).
 */
void searchRedirect(char* cmd, REDIRECT_TYPE *redirects);

void moveLeft(Command *command);
void moveRight(Command *command);
void backspace(Command *command, char* prompt, char* wd);

#endif
