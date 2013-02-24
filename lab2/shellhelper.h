#ifndef SHELLHELPER_H
#define SHELLHELPER_H
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
