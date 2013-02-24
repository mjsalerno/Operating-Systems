#include "shellhelper.h"
#include <string.h>
#include <stdio.h>

char* parseEnv(char **envp, char *keyword){
  char *cp, *at;
  int i;
  
  for(at = *envp, i = 0; envp+i != NULL; at = (*(envp)+i), i++){
	  cp = strstr(at, keyword);
	  
	  if(cp != NULL)
		break;  
	}
	
	if(envp+i == '\0')
		return NULL;
	else{
		// USE i AND return char pointer from envp
		return strstr(at, "=")+1;
	}
	
}

int main (int argc, char ** argv, char **envp) {
	
	char *cp = parseEnv(envp, "PATH");
	
	printf("\nGOT: %s\n", cp);
	//printf("\nGOT: %s\n", strstr("HELLO HI", "L"));
	
	return 0;
}
