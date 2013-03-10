#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[]) {
	char *str = "HELLO HO$W ARE YOU?";
	char *cp = strchr(str, '$');

	printf("%s\n", cp);

	return 0;
}
