#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "swish.h"
#include "command.h"
#include "shellhelper.h"

int main(int argc, char ** argv, char **envp) {
    bool running = true;
    bool debug = false;
    bool readingScript = false;
    char *prompt = "swish> ";
    char wd[MAX_PATH];
    Command command = {0, 0};
    FILE *script = NULL;
    char cmdFromFile[MAX_INPUT];
    char *cp;
    setbuf(stdout, NULL);

    // Parse Args
    if (argc > 1) {
        if (!strcmp("-d", argv[1])) {
            debug = true;
            printf("debugging on.\n");
        } else {
            if (debug) printf("-----------------\nrunning a script\n-----------------\n");
            script = fopen(argv[1], "r");
            readingScript = true;
            do {
                running = (NULL != fgets(cmdFromFile, MAX_INPUT, script));
                removeNewline(cmdFromFile);
            } while (*cmdFromFile == '#');
            if (NULL != (cp = strchr(cmdFromFile, '#'))) {
                *cp = '\0';
            }
            setCommand(&command, cmdFromFile);
            if (debug) printf("-----------------\ncmdFromFile:%s\n-----------------\n", command.value);
        }
    }
    // Start in a loop
    while (running) {
        /* Vars for handling the splitting of the command args */
        char *cmds[MAX_INPUT];
        char *cmd;
        int cmdsIndex = 0; 
        REDIRECT_TYPE redirects[MAX_INPUT];

        if (!readingScript) {
            getcwd(wd, MAX_PATH);
            printf("%s[%s]%s %s%s", BLUE, wd, GREEN, prompt, NONE);
            // Get input from keyboard
            getInput(&command, prompt, wd);
            // Adjust pointer to correct spot.
            command.index = command.size;
            // Give the current command a null
            addCommand(&command, '\0');
            printf("\n"); // Print the newline that got consumed by the getch() loop   
        }
        // support redirection
        int rdSize = searchRedirect(command.value, redirects);
        cmd = strtok(command.value, "<|>");        
        while(cmd){
            cmds[cmdsIndex++] = cmd;
            cmd = strtok(NULL, "<|>");
        }
        // Evalue the user input
        evaluateCommand(cmds, cmdsIndex, &running, wd, envp, script, &readingScript, debug, redirects, rdSize);
        // Reset Command
        resetCommand(&command);
        // Scripting Support
        if (readingScript) {
            do {
                running = (NULL != fgets(cmdFromFile, MAX_INPUT, script));
                removeNewline(cmdFromFile);
            } while (*cmdFromFile == '#');
            if (NULL != (cp = strchr(cmdFromFile, '#'))) {
                *cp = '\0';
            }
            setCommand(&command, cmdFromFile);
            if (debug) printf("-----------------\ncmdFromFile:%s\n-----------------\n", command.value);
        }
    }
    return 0;
}

void evaluateCommand(char **cmd, int cmdSize, bool *running, char* wd, char** envp, FILE *script, bool *readingScript, bool debug, REDIRECT_TYPE *redirects, int rdSize) {
    char *arguments[MAX_ARGS];
    
    // Something went wrong stop evaluating.
    if(cmdSize <= 0){
        return;
    }
    // Check to see how many commands we need to evaluate
    if(!rdSize){
        if (strlen(*cmd)) {
            if (debug) {
                printf("RUNNING:%s\n", *cmd);
            }
            parseCommand(*cmd, arguments, MAX_ARGS);
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

            if (debug) {
                printf("ENDED: %s (needs return val)\n", *cmd);
            }
        }
    } else {
        spawnRedirect(cmd, cmdSize, redirects, rdSize);
    }

}

void getInput(Command *command, char *prompt, char *wd) {
    int ch;
    while ((ch = fgetc(stdin)) != '\n') {
        // Handle Arrow Keys
        if (ch == 27 || ch == 127) {
            if (ch == 127) {
                backspace(command, prompt, wd);
            } else {
                if ((ch = fgetc(stdin)) == '[') {
                    switch (ch = fgetc(stdin)) {
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
        } else if (ch >= 32 && ch <= 126) {
            addCommand(command, (char) ch);
            printf("%c", (char) ch);
        } else {
            printf("Special Key: %d\n", ch);
        }
    }
}

int searchRedirect(char* cmd, REDIRECT_TYPE *redirects){
    int count = 0;
    for(int i = 0; i < strlen(cmd); i++){
        switch(cmd[i]){
            case PIPE:
                redirects[count++] = PIPE;
                break;
            case REDIRECT_LEFT:
                redirects[count++] = REDIRECT_LEFT;
                break;
            case REDIRECT_RIGHT:
                redirects[count++] = REDIRECT_RIGHT;
                break;
        }
    }
    //printf("There is %d redirect(s)\n", count);
    redirects[count] = REDIRECT_NONE;
    return count;
}

void moveLeft(Command *command) {
    if (command->index > 0) {
        command->index--;
        printf("\b");
    }
}

void moveRight(Command *command) {
    if (command->index < command->size) {
        command->index++;
        printf("\033[1C");
    }
}

void backspace(Command *command, char* prompt, char* wd) {
    if (command->index > 0) {
        // Place the index in the correct spot
        command->index--;
        // Shift Everything Over
        for (int i = command->index + 1; i < command->size; i++) {
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

