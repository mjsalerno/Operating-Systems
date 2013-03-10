/* CSE 306: Sea Wolves Interactive SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shellhelper.h"

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_ARGS  1024
#define MAX_PATH  2048

int setenv(const char *name, const char *value, int overwrite);

int main(int argc, char ** argv, char **envp) {

    bool finished = false;
    char *prompt = "\033[0;32mswish>\033[0m ";
    int debug = 0;
    char cmd[MAX_INPUT];
    char *arguments[MAX_ARGS];
    setbuf(stdout, NULL);
    char wd[MAX_PATH];

    if (argc > 1) {
        if (!strcmp("-d", argv[1])) {
            debug = 1;
            printf("debugging on.\n");
        }
    }

    while (!finished) {        

        // Print out the prompt
        getcwd(wd, MAX_PATH);
        printf("%s[%s]%s %s ", BLUE, wd, NONE, prompt);
        fgets(cmd, MAX_INPUT, stdin);
        strtok(cmd, "\n"); // Thread safe?

        // Evaluate the command
        if (strlen(cmd)) {
            if (debug)printf("RUNNING:%s\n", cmd);
            parseCommand(cmd, arguments, MAX_ARGS);
            if (!strcmp(arguments[0], "exit")) {
                finished = true;
            } else if (!strcmp(arguments[0], "cd")) {
                setenv("OLDPWD", wd, 1);
                printf("oldpwd: %s\n", getenv("OLDPWD"));
                if (!strcmp(arguments[1], "-")) {
                    chdir(getenv("OLDPWD"));
                } else if (!strcmp(arguments[1], "~")) {
                    chdir(parseEnv(envp, "HOME"));
                } else {
                    int val = chdir(arguments[1]);
                    if (val) printf("Sorry but %s does not exist\n", arguments[1]);
                    //STILL NEEDS - IMPLEMENTATION
                }
            } else {
                spawn(arguments);
            }
            if (debug)printf("ENDED: %s (needs return val)\n", cmd);
        }
    }

    return 0;
}
