#ifndef COMMAND_H 
#define COMMAND_H 

#define MAX_INPUT 1024
struct Command{
	int index;
	int size;
	char value[MAX_INPUT];
};
typedef struct Command Command;

void addCommand(Command *command, char value);
void resetCommand(Command *command);
void setCommand(Command *command, char *value);
#endif
