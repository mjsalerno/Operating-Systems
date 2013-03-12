CC    = gcc
CLANG = clang
FLAGS = -Wall -Werror -std=c99 -D_XOPEN_SOURCE -D_POSIX_C_SOURCE=200112L
LIBS  = -lncurses
SWISH = swish 

swish: swish.c command.o swishui.o shellhelper.o command.h shellhelper.h swishui.h
	$(CC) $(FLAGS) -g -o $(SWISH) swish.c swishui.o command.o shellhelper.o $(LIBS)

shellhelper.o: shellhelper.c shellhelper.h
	$(CC) $(FLAGS) $(LIBS) -c shellhelper.c

command.o: command.c command.h
	$(CC) $(FLAGS) -c command.c

swishui.o: swishui.c swishui.h
	$(CC) $(FLAGS) $(LIBS) -c swishui.c

clean:
	rm *.o $(SWISH)
