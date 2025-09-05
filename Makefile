TARGET = nano
FLAGS = -ggdb -Wall
CC = gcc

OBJ = nano.o cd.o pwd.o ext.o vars.o parser.o frk.o echo.o redirection_parser.o

.PHONY: all
all: $(TARGET)


$(TARGET): $(OBJ)
	 $(CC) $(FLAGS) -o $@ $^

%.o : %.c
	$(CC) $(FLAGS) -c -o $@ $< 



.PHONY: clean
clean:
	@rm *.o $(TARGET)
