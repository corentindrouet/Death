EXEC		=	Death_disas
CC			=	gcc
CFLAGS		=	-Wall -Wextra -Werror
SRC			=	disassembler.c

OBJ			=	$(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ)

fclean: clean
	rm -rf $(EXEC)

re: fclean all

.PHONY: all clean fclean re
