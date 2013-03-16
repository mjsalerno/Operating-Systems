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
    FILE *historyFile;
    char *historyList[MAX_HISTORY];
    int historyPtr = 0;
    int historyShown = 0;
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

    //Read History
    if(!readingScript) {
        historyFile = openHistoryFile("r");
        if(historyFile == NULL && debug) printf("---------------THE FILE WAS NULL\n");
        
        for (int i = 0; i < MAX_HISTORY; ++i) {
            historyList[i] = (char *) malloc(sizeof(char) * MAX_INPUT);
            *historyList[i] = '\0';
        }

        int i = 0;
        if (historyFile != NULL) {
            for (i = 0; (i < MAX_HISTORY) && (NULL != fgets(historyList[i], MAX_INPUT, historyFile)); ++i){
                removeNewline(historyList[i]);
            }
            i--;
            fclose(historyFile);
            // for (int i = 0; i < 10; ++i){
            //    printf("HISTORY %d:%s\n", i, historyList[i]);
            // }
        }
        historyPtr = i;
        historyShown = i;
        
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
            getInput(&command, prompt, wd, historyShown, historyList);
            // Adjust pointer to correct spot.
            command.index = command.size;
            // Give the current command a null
            addCommand(&command, '\0');
            printf("\n"); // Print the newline that got consumed by the getch() loop   

            if(strcmp(command.value, "exit") != 0){
                strcpy(historyList[historyPtr], command.value);
                if (debug) printf("added command to historyList: %s\n", historyList[historyPtr]);
            }
            
            historyDn(&historyPtr);
            historyShown = historyPtr;
            // historyPtr++;
        }
        // support redirection
        int rdSize = searchRedirect(command.value, redirects);
        cmd = strtok(command.value, "<|>");        
        while(cmd){
            cmds[cmdsIndex++] = cmd;
            cmd = strtok(NULL, "<|>");
        }
        // Evalue the user input
        evaluateCommand(cmds, cmdsIndex, &running, wd, envp, script, &readingScript, debug, historyList, historyPtr,redirects, rdSize);
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
            if (!running) fclose(script);
            if (debug) printf("-----------------\ncmdFromFile:%s\n-----------------\n", command.value);
        }
    }

    // for (int i = 0; i < 10; ++i){
    //     printf("HISTORY %d:%s\n", i, historyList[i]);
    // }

    //Write historyList to the history file.
    if(!readingScript) {
        if (debug){ printf("%sABOUT TO OPEN HISTORY FILE%s\n", CYAN, NONE);}        
        historyFile = openHistoryFile("w");     
        if(historyFile == NULL) printf("---------------THE FILE WAS NULL\n");
        if (debug){ printf("%sOPENED HISTORY FILE%s\n", CYAN, NONE);}
        writeHistoryFile(historyFile, historyList);
        // fflush(historyFile);
        // fclose(historyFile);
        /*
        for (int i = 0; i < MAX_HISTORY; ++i) {
            free(historyList[i]);
        }
        */
    }

    return 0;
}

void evaluateCommand(char **cmd, int cmdSize, bool *running, char* wd, char** envp, FILE *script, bool *readingScript, bool debug, char *historyList[], int historyPtr, REDIRECT_TYPE *redirects, int rdSize) {
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
            

            if (arguments[1] == NULL || !strcmp(arguments[1], "~")) {
                setenv("OLDPWD", wd, 1);
                if (debug) printf("oldpwd: %s\n", getenv("OLDPWD"));
                chdir(parseEnv(envp, "HOME"));

            } else if (!strcmp(arguments[1], "-")) {
                chdir(getenv("OLDPWD"));
            } else {
                setenv("OLDPWD", wd, 1);
                if (debug) printf("oldpwd: %s\n", getenv("OLDPWD"));
                int val = chdir(arguments[1]);
                if (val) printf("Sorry but %s does not exist\n", arguments[1]);
            }

        } else if (!strcmp(arguments[0], "set")) {
            setenv(arguments[1], arguments[3], 1);
            printf("envset: %s\n", getenv(arguments[1]));

        } else if (!strcmp(arguments[0], "echo")) {
            int index = contains(arguments, '$');
            char *cp = NULL;
            if(index > -1) cp = arguments[index];
            if (cp != NULL) {
                cp++;
                printf("%s\n", getenv(cp));
            } else {
                spawn(arguments);
            }

        } else if(!strcmp(arguments[0], "history")) {
            for (int i = 0; i < MAX_HISTORY && *historyList[i] != '\0'; ++i) {
                printf("%s\n", historyList[i]);
            }            
        } else if(!strcmp(arguments[0], "wolfie")){
            printWolf();
        } else if(!strcmp(arguments[0], "cls")){
            strcpy(arguments[0], "clear");
            spawn(arguments);
        } else if(!strcmp(arguments[0], "clear")){
            for (int i = 0; i < MAX_HISTORY; ++i) {
                *historyList[i] = '\0';
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



void getInput(Command *command, char *prompt, char *wd, int historyShown, char *historyList[]) {
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
                            // printf("Up Arrow was pressed\n");
                            historyShowUp(&historyShown, historyList);
                            replaceCommand(historyList[historyShown], command, wd, prompt);
                            break;
                        case 'B':
                            // printf("Down Arrow was pressed\n");
                            historyShowDn(&historyShown, historyList);
                            replaceCommand(historyList[historyShown], command, wd, prompt);
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

