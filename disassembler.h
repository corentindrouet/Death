#ifndef DISASSEMBLER_H
# define DISASSEMBLER_H
# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include <sys/mman.h>
# include <elf.h>
# include <string.h>
# include <stdlib.h>
# define U_CHAR unsigned char

typedef struct	s_rex_prefix {
	U_CHAR		*byte;
	U_CHAR		is_64;
	U_CHAR		sib_extension;
	U_CHAR		reg_extension;
	U_CHAR		dest_reg_extension;
}				t_rex_prefix;

typedef struct	s_mod_rm {
	U_CHAR		*byte;
	U_CHAR		direct;
	U_CHAR		reg;
	U_CHAR		rm;
}				t_mod_rm;

typedef struct	s_sib {
	U_CHAR		*byte;
	U_CHAR		scale;
	U_CHAR		index;
	U_CHAR		base;
}				t_sib;

typedef struct	s_instruction {
	U_CHAR		*instruction;
	unsigned int	inst_offset;
	U_CHAR		grp_prefix[4];
	int			nb_grp_prefix;
	t_rex_prefix	*rex_prefix;
	t_mod_rm	*ModRM;
	t_sib		*SIB;
	U_CHAR		resize;
	unsigned int			opcode;
	unsigned int			displacement;
	unsigned int			immediate[4];
	unsigned int			relative;
	void		*next;
	void		*previous;
	size_t		inst_size;
}				t_instruction;

typedef struct	s_function {
	t_instruction	*start;
	t_instruction	*end;
	void			*next;
	void			*previous;
}				t_function;

typedef struct	s_opcode {
	U_CHAR	prefix;
	U_CHAR	opcode;
	U_CHAR	opcode_extension_reg;
	U_CHAR	opcode_extension_inst;
	char	mnemonic[16];
	U_CHAR	operand[4];
}				t_opcode;

void	disas_text_section(void *text, size_t size);
size_t	find_text_section(void *file_mem, void **text_start);
size_t	file_size(int fd);
t_instruction	*create_instruction(void *mem);
int				verif_prefix_values(char byte);
void	delete_instruction(t_instruction *insts);
void	print_instruction(t_instruction *insts);
t_function	*create_function(t_instruction *start, t_instruction *end);
void	delete_function_lst(t_function **lst);
void	find_functions(t_instruction *insts_lst);
t_opcode	*find_opcode_instruction(U_CHAR opcode_to_find, U_CHAR prefix, U_CHAR opcode_inst_ext, char destroy_table);

#endif
