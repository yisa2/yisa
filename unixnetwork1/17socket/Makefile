.PHONY: clean all
CC=gcc
CFLAGS=-Wall -g
BIN=echocli pollserv echoserv nofile_limit conntest 
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
clean:
	rm -f *.o $(BIN)


