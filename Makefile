TARGET := futsushu
CC := gcc
SRCS := main.c prompt.c parser.c lexer.c
OBJS := $(SRCS:.c=.o)
HEADERS := $(wildcard *.h)
CFLAGS := -std=c11 -Wall -g
INCS := -I.
LIBS := 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS): $(HEADERS)

clean:
	rm -rf $(TARGET) $(OBJS)
	
.PHONY: clean