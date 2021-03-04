.PHONY: clean all
CC=gcc
CFLAGS=-Wall -g
BIN=echocli echoserv getiplist
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
clean:
	rm -f *.o $(BIN)


