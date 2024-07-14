SRC:=$(wildcard *.c)
OBJ:=$(SRC:%.c=bin/%.o)
all : $(OBJ)
	gcc $^ -I. -o bin/dtvmrt -Ofast
bin/%.o : %.c
	gcc -c $^ -o $@ -Ofast -I.
clean :
	rm ./bin/*.o
