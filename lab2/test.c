#include <stdio.h>

int main(int argc, char const *argv[])
{
	putenv("HELLO=hi");
	system("printenv | grep hi");
	printf("\n%s\n", getenv("HELLO"));
	return 0;
}
