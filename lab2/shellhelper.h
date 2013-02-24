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
 * Performs fork & exec using the program, and arguments provided.
 * @param program char array containing the name of the binary to be executed.
 * @param args NULL terminated array of char* that contains the argument(s) to the binary.
 * @return Returns the pid of the child that is spawned.
 */
int spawn(char *program, char **args);
#endif
