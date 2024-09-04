CHESS_SRC=$(wildcard src/*.c)
SERVER_SRC=$(wildcard server/*.c)

.PHONY: all
all: chess matchmaker

chess: $(CHESS_SRC)
	gcc -Wall -ggdb -o chess $^

matchmaker: $(SERVER_SRC)
	gcc -Wall -ggdb -o matchmaker $^
