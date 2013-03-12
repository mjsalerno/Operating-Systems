#include <stdio.h>
#include <string.h>
#include "command.h"

void addCommand(Command *command, char value){
	if(command->index < MAX_INPUT - 1){
		command->value[command->index] = value;
		command->index++;
		command->size++;
	}else{
		printf("The Command is too large! MAX_INPUT=%d\n", MAX_INPUT);
	}
}

void setCommand(Command *command, char *value){
	int length = strlen(value); 
	if(length < MAX_INPUT - 1){
		strcpy(command->value, value);
		command->size = length;
		command->index = length;
	}else{
		printf("The Command is too large! MAX_INPUT=%d\n", MAX_INPUT);
	}
}

void resetCommand(Command *command){
	command->index = 0;
	command->size = 0;
}
