
SRC=$(wildcard src/*.c)

.PHONY: all
all: $(SRC)
	gcc -Wall -ggdb -o chess $^

