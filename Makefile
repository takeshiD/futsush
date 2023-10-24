TARGET := futsush
CC := gcc
SRCS := main.c prompt.c parser.c lexer.c
OBJS := $(SRCS:.c=.o)
CFLAGS := -std=c11 -Wall -g
INCS := -I.
LIBS := 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS)