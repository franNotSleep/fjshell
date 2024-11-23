TARG = fjshell
SRCS = fjshell.c
OBJS = $(SRCS:.c=.o)
CC = gcc
OPTS = -Wall -Werror -g

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(OPTS) $(OBJS) -o $(TARG)

%.o: %.c
	$(CC) $(OPTS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARG)

	

