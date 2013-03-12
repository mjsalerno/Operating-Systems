#ifndef SWISH_H
#define SWISH_H
#include <stdbool.h>
#include "command.h"

#define MAX_ARGS  1024
#define MAX_PATH  2048

/**
 * Initializes the ncurses screen.
 */
void setupScreen();

/**
 * Attemps to parse, and spawn a process based on the command.
 */
void evaluateCommand(char *cmd, bool *running, char* wd, char** envp, FILE *script, bool *readingScript, bool debug);

void getInput(Command *command, char *prompt, char *wd);

void moveLeft(Command *command);
void moveRight(Command *command);
void backspace(Command *command);

#endif
