#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "swish.h"
#include "command.h"
#include "shellhelper.h"

int main(int argc, char ** argv, char **envp){
    bool running = true;
    bool debug = false;
    bool readingScript = false;
    char *prompt = "swish> ";
    char wd[MAX_PATH];
    Command command = {0, 0};
    FILE *script = NULL;
    setbuf(stdout, NULL);
    // Parse Args
 	if (argc > 1) {
        if (!strcmp("-d", argv[1])) {
            debug = true;
            printf("debugging on.\n");
        }else{
            script = fopen(argv[1], "r");
            readingScript = true;
            do{
                running = (NULL == fgets(command.value, MAX_INPUT, script));
                removeNewline(command.value);
            }while (*command.value == '#');
            if (debug) printf("cmdFromFile:%s\n", command.value);
        }
    }
    // Start in a loop
    while(running){
        if(!readingScript){
            getcwd(wd, MAX_PATH);
            printf("%s[%s]%s %s%s", BLUE, wd, GREEN, prompt, NONE);
        }
 		// Get input from keyboard
 		getInput(&command, prompt, wd);
 		// Adjust pointer to correct spot.
 		command.index = command.size;
 		// Give the current command a null
 		addCommand(&command, '\0');
 		printf("\n"); // Print the newline that got consumed by the getch() loop   
        // Parse the command.
        evaluateCommand(command.value, &running, wd, envp, script, &readingScript, debug);
        // Reset Command
        resetCommand(&command);
    }
    // If the script is running allow the user to kill the screen.
    return 0;
}

void evaluateCommand(char *cmd, bool *running, char* wd, char** envp, FILE *script, bool *readingScript, bool debug){
  char *arguments[MAX_ARGS];
  
  if (strlen(cmd)) {
    if (debug){
      printf("RUNNING:%s\n", cmd);
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
            if (debug) printf("oldpwd: %s\n", getenv("OLDPWD"));
            chdir(parseEnv(envp, "HOME"));

        } else {
            setenv("OLDPWD", wd, 1);
            if (debug) printf("oldpwd: %s\n", getenv("OLDPWD"));
            int val = chdir(arguments[1]);
            if (val) printf("Sorry but %s does not exist\n", arguments[1]);
        }

    } else if (!strcmp(arguments[0], "set")) {
        setenv(arguments[1], arguments[3], 1);
        printf("envset: %s\n", getenv(arguments[1]));

    } else if (!strcmp(arguments[0], "echo")) { //TODO: get this working for $ not in the first arg
        char *cp = strchr(arguments[1], '$');
        if (cp != NULL) {
            cp++;
            printf("%s = %s\n", cp, getenv(cp));
        } else {
            spawn(arguments);
        }

    } else {
        spawn(arguments);
    }

    if (debug){
        printf("ENDED: %s (needs return val)\n", cmd);
    }
}

if (*readingScript) {
    do {
        *running= (NULL == fgets(cmd, MAX_INPUT, script));
        removeNewline(cmd);
    } while (*cmd == '#');

    if (debug) printf("cmdFromFile:%s\n", cmd);
  }
}

void getInput(Command *command, char *prompt, char *wd){
	int ch;
	while((ch = fgetc(stdin)) != '\n'){
    // Handle Arrow Keys
    if(ch == 27 || ch == 127){
      if(ch == 127){
        backspace(command, prompt, wd);            
      }else{
        if((ch = fgetc(stdin)) == '['){
            switch(ch = fgetc(stdin)){
                case 'A':
                    printf("Up Arrow was pressed\n");
                    break;
                case 'B':
                    printf("Down Arrow was pressed\n");
                    break;
                case 'C':
                    moveRight(command);
                    break;
                case 'D':
                    moveLeft(command);
                    break;
                default:
                    printf("Unknown key was pressed.\n");
                    break;
            }
        }
      }
    }
    else if(ch >= 32 && ch <= 126){
      addCommand(command, (char)ch);
      printf("%c", (char)ch);  
    }else{

      printf("Special Key: %d\n", ch);
    }
  }
  /*
  while((ch = fgetc(stdin)) != '\n'){
		switch(ch){
			case KEY_UP:
				setCommand(command, "History up!");
				printf("[%s] %s%s", wd, prompt, command->value);
				break;
			case KEY_DOWN:
				setCommand(command, "History down!");
				printf("[%s] %s%s", wd, prompt, command->value);
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
	}*/
}

void moveLeft(Command *command){
	if(command->index > 0){
    command->index --;
    printf("\b");
  }
}

void moveRight(Command *command){
	if(command->index < command->size){
    command->index++;
    printf("\033[1C");
  }  
}

void backspace(Command *command, char* prompt, char* wd){
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
        // Erase the line
        printf("\r\033[K");
        // Print out the new line
        printf("%s[%s] %s%s%s%s", BLUE, wd, GREEN, prompt, NONE, command->value);
  }
}

