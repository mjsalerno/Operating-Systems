CC    = gcc
CLANG = clang
FLAGS = -Wall -Werror -std=c99 -D_XOPEN_SOURCE -D_POSIX_C_SOURCE=200112L
SWISH = swish 

swish: swish.c command.o shellhelper.o command.h shellhelper.h
	$(CC) $(FLAGS) -g -o $(SWISH) swish.c command.o shellhelper.o

shellhelper.o: shellhelper.c shellhelper.h
	$(CC) $(FLAGS) -c shellhelper.c

command.o: command.c command.h
	$(CC) $(FLAGS) -c command.c

clean:
	rm *.o $(SWISH)
