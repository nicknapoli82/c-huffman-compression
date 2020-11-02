#As simple as I can make :)

CC = clang
CFLAGS = -Wall

SRCS = ./src/main.c ./src/huff.c ./src/bitStream.c ./src/minTree.c ./src/parseArgs.c

MAIN = huff

#base build
default: 
	$(CC) $(CFLAGS) -O2 -o $(MAIN) $(SRCS)

debug: $(OBJS)
	$(CC) $(CFLAGS) -O0 -ggdb3 -o $(MAIN) $(SRCS)
