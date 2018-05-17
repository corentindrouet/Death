EXEC		=	Death_disas
CC			=	gcc
CFLAGS		=	-Wall -Wextra -Werror
SRC			=	disassembler.c \
				instruction.c \
				functions.c \
				find_relative_addr.c
OBJ			=	$(SRC:.c=.o)
PARSOR_SRC	=	ft_atoi_base.c \
				parser.c
PARSOR_EXEC	=	parsor
DL_SRC		=	opcode_html
DL_PARSED	=	opcode_table


all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
	curl -o $(DL_SRC) http://ref.x86asm.net/coder64.html
	$(CC) -o $(PARSOR_EXEC) $(PARSOR_SRC)
	./$(PARSOR_EXEC) $(DL_SRC) $(DL_PARSED)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ) $(DL_SRC)

fclean: clean
	rm -rf $(EXEC) $(DL_PARSED) $(PARSOR_EXEC)

re: fclean all

.PHONY: all clean fclean re
