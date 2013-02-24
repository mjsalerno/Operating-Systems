#ifndef SHELLHELPER_H
#define SHELLHELPER_H

#define RED       "\033[0;31m"        /* 0 -> normal ;  31 -> red */
#define CYAN      "\033[1;36m"        /* 1 -> bold ;  36 -> cyan */
#define GREEN     "\033[4;32m"        /* 4 -> underline ;  32 -> green */
#define BLUE      "\033[9;34m"        /* 9 -> strike ;  34 -> blue */
#define BLACK     "\033[0;30m"
#define BROWN     "\033[0;33m"
#define MAGENTA   "\033[0;35m"
#define GRAY      "\033[0;37m"
#define NONE      "\033[0m"           /* to flush the previous property */

/**
 * Searchs the array containing the enviroment variables and looks for the provided keyword.
 * @param envp NULL terminated array of char* that contains all the enviroment variables.
 * @param keyword char array containing a sequence of characters to search for.
 * @return Returns a pointer to the char array if it is found, else NULL.
 */
char* parseEnv(char **envp, char *keyword);

/**
 * Performs fork & exec using the arguments provided.
 * @param args NULL terminated array of char* that contains the argument(s) to the binary.
 */
void spawn(char **args);

/**
 * Parses a string a puts the results in the parsed array.
 */
void parseCommand(char *command, char** parsed, int size);

/**
 * ### EXPERIMENTAL ###
 * Creates an array of char* that is NULL terminated. Helper function for use of execvp.
 * @param filename char array containing the name of the binary to be executed.
 * @param num int containing the number of variable arguments to be provided.
 * @return Returns a NULL terminated char array of char* that contains the arguments to be passed to a binary. 
 */
char** argsBuilder(char *filename, int num, ...);
#endif
