.PHONY: clean all
CC=gcc
CFLAGS=-Wall -g
BIN=tmp1 addr echoserver echocli echoserver2 p2pcli p2psrv
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ 
clean:
	rm -f *.o $(BIN)


