#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include "swish.h"
#include "command.h"
#include "shellhelper.h"
#include "swishui.h"

int main(int argc, char ** argv, char **envp){
  bool running = true;
  bool debug = false;
  bool readingScript = false;
  char *prompt = "swish> ";
  char wd[MAX_PATH];
  Command command = {0, 0};
  FILE *script = NULL;
  // Initialize the ncurses screen    
  setupScreen();
  // Parse Args
 	if (argc > 1) {
    if (!strcmp("-d", argv[1])) {
      debug = true;
      printw("debugging on.\n");
      refresh();
    }else{
      script = fopen(argv[1], "r");
      readingScript = true;
      do{
        running = (NULL == fgets(command.value, MAX_INPUT, script));
        removeNewline(command.value);
      }while (*command.value == '#');
      if (debug) printw("cmdFromFile:%s\n", command.value);
    }
  }
  // Start in a loop
  while(running){
    if(!readingScript){
      getcwd(wd, MAX_PATH);
      printw("[%s] %s", wd, prompt);
    }
 		// Get input from keyboard
 		getInput(&command, prompt, wd);
 		// Adjust pointer to correct spot.
 		command.index = command.size;
 		moveCursorX(stdscr, command.size - command.index);
 		// Give the current command a null
 		addCommand(&command, '\0');
 		printw("\n"); // Print the newline that got consumed by the getch() loop   
    refresh();
    // Parse the command.
    evaluateCommand(command.value, &running, wd, envp, script, &readingScript, debug);
    // Reset Command
    resetCommand(&command);
    // Make sure that the screen gets redrawn.
    refresh();
  }
  // If the script is running allow the user to kill the screen.
  if(readingScript){
    printw("Press any key to continue.\n");
    refresh();
    getch();
  }
  endwin();
  return 0;
}

void setupScreen(){
	initscr();
  noecho();
  raw();
  scrollok(stdscr, true);
  keypad(stdscr, true);
}

void evaluateCommand(char *cmd, bool *running, char* wd, char** envp, FILE *script, bool *readingScript, bool debug){
  char *arguments[MAX_ARGS];
  
  if (strlen(cmd)) {
    if (debug){
      printw("RUNNING:%s\n", cmd);
      refresh();
    }
    parseCommand(cmd, arguments, MAX_ARGS);
    //if (debug)printf("RUNNING after parse:%s\n", cmd);
    if (!strcmp(arguments[0], "exit")) {
        *running = false;

    } else if (!strcmp(arguments[0], "cd")) {
        if (!strcmp(arguments[1], "-")) {
            chdir(getenv("OLDPWD"));

        } else if (!strcmp(arguments[1], "~")) {
            setenv("OLDPWD", wd, 1);
            if (debug) printw("oldpwd: %s\n", getenv("OLDPWD"));
            chdir(parseEnv(envp, "HOME"));

        } else {
            setenv("OLDPWD", wd, 1);
            if (debug) printw("oldpwd: %s\n", getenv("OLDPWD"));
            int val = chdir(arguments[1]);
            if (val) printw("Sorry but %s does not exist\n", arguments[1]);
        }

    } else if (!strcmp(arguments[0], "set")) {
        setenv(arguments[1], arguments[3], 1);
        printf("envset: %s\n", getenv(arguments[1]));

    } else if (!strcmp(arguments[0], "echo")) { //TODO: get this working for $ not in the first arg
        char *cp = strchr(arguments[1], '$');
        if (cp != NULL) {
            cp++;
            printw("%s = %s\n", cp, getenv(cp));
        } else {
            spawn(arguments);
        }

    } else {
        spawn(arguments);
    }

    if (debug){
        printw("ENDED: %s (needs return val)\n", cmd);
        refresh();
    }
}

if (*readingScript) {
    do {
        *running= (NULL == fgets(cmd, MAX_INPUT, script));
        removeNewline(cmd);
    } while (*cmd == '#');

    if (debug) printw("cmdFromFile:%s\n", cmd);
}

/*
  if(strlen(cmd)){
      if(debug){
      	printw("RUNNING:%s\n", cmd);
      	refresh();
      }
      parseCommand(cmd, arguments, MAX_ARGS);
      if(!strcmp(arguments[0], "exit")){
          *running = false;
      }else if(!strcmp(arguments[0], "cd")){
          setenv("OLDPWD", wd, 1);
          printw("oldpwd: %s\n", getenv("OLDPWD"));
          if(!strcmp(arguments[1], "-")) {
              chdir(getenv("OLDPWD"));
          }else if (!strcmp(arguments[1], "~")){
              chdir(parseEnv(envp, "HOME"));
          }else{
              int val = chdir(arguments[1]);
              if (val){
              	printw("Sorry but %s does not exist\n", arguments[1]);
              } 
              //STILL NEEDS - IMPLEMENTATION
          }
      }else{
          spawn(arguments);
      }
      if (debug)printw("ENDED: %s (needs return val)\n", cmd);
  }
  */
}

void getInput(Command *command, char *prompt, char *wd){
	int ch;
	while((ch = getch()) != '\n'){
		switch(ch){
			case KEY_UP:
				resetCursorX(stdscr);
				clrtoeol();
				refresh();
				setCommand(command, "History up!");
				printw("[%s] %s%s", wd, prompt, command->value);
				break;
			case KEY_DOWN:
				resetCursorX(stdscr);
				clrtoeol();
				refresh();
				setCommand(command, "History down!");
				printw("[%s] %s%s", wd, prompt, command->value);
				break;
			case KEY_LEFT:
				moveLeft(command);
				break;
			case KEY_RIGHT:
				moveRight(command);
				break;
			case KEY_BACKSPACE:
				backspace(command);
				break;
			case KEY_DC:
				// Consume delete key
				break;
			case '\t':
				// Consume Tab Key
				break;
			case 27:
				// Consume Escape Key
				break;
			default:
				printw("%c", ch);
				//insch(ch);
				//moveCursorX(stdscr, 1);
				addCommand(command, ch);
				break;
		}
	}
}

void moveLeft(Command *command){
	if(command->index > 0){
    command->index --;
    moveCursorX(stdscr, -1);
  }
}

void moveRight(Command *command){
	if(command->index < command->size){
    command->index++;
    moveCursorX(stdscr, 1);
  }  
}

void backspace(Command *command){
	if(command->index > 0){
		// Place the index in the correct spot
		command->index--;
		// Shift Everything Over
		for(int i = command->index + 1; i < command->size; i++){
			command->value[i - 1] = command->value[i];
		}
		// Adjust the end of the string
		command->size--;
		command->value[command->size] = '\0';
		// Reflect Changes to the user in terminal
    moveCursorX(stdscr, -1);
    refresh();
    delch();
  }
}

