
SRC=$(wildcard src/*.c)

.PHONY: all
all: $(SRC)
	gcc -ggdb -o chess $^

