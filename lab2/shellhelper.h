#ifndef SHELLHELPER_H
#define SHELLHELPER_H
/**
 * Searchs the array containing the enviroment variables and looks for the provided keyword.
 * @return Returns a pointer to the char array if it is found, else NULL.
 */
char* parseEnv(char **envp, char *keyword);

/**
 * Performs fork & exec using the program, and arguments provided.
 * @return Returns the pid of the child that is spawned.
 */
int spawn(char *binary, char **args);
#endif
