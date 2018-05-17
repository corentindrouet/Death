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
	unsigned int	new_offset;
	U_CHAR		grp_prefix[4];
	int			nb_grp_prefix;
	t_rex_prefix	*rex_prefix;
	t_mod_rm	*ModRM;
	t_sib		*SIB;
	U_CHAR		resize;
	unsigned int			opcode;
	unsigned int			displacement;
	unsigned int			immediate[4];
	int			relative;
	U_CHAR		relative_offset;
	void		*next;
	void		*previous;
	size_t		inst_size;
}				t_instruction;

typedef struct	s_function {
	t_instruction	*start;
	t_instruction	*end;
	size_t			fct_size;
	void			*fct_copy;
	unsigned int	new_offset;
	void			*next;
	void			*previous;
}				t_function;

typedef struct	s_relative_addr {
	t_instruction	*inst;
	t_instruction	*inst_related;
	t_function		*fct_related;
	void			*next;
	void			*previous;
}				t_relative_addr;

typedef struct	s_opcode {
	U_CHAR	prefix;
	U_CHAR	opcode;
	U_CHAR	opcode_extension_reg;
	U_CHAR	opcode_extension_inst;
	char	mnemonic[16];
	U_CHAR	operand[4];
}				t_opcode;

/*
	in order

	Prefix:
		prefix are simple value on 1 byte
	Opcode:
		code of the instruction. 1 byte
	Opcode register extension:
		This field specified if opcode have a marge of register.
		some opcode have a base opcode+register value, like: push register
		push is opcode 0x50, and register code can't exceed 0x7.
		so push will be encoded from 0x50 to 0x57.
	Opcode instruction extension:
		This field specified if opcode have an r/m field extension.
		Some opcode have he same operands, like add/or/.../cmp register, immediate.
		So they can have the same opcode, with an extension on the modrm byte, to specify
		the specific instruction called.
	Operand working:
		All operands are on 1 byte.
		all specification are added on this byte by oring
		0x1  this operand is used, wouhou. if set to 0, this operand is not use.
		0x2  this operand is an immediate value
			0x4  this immediate is a 8 bits value
			0x8  this immediate is a 16 bits value
			0x10 this immediate is a 32 bits value
		0x4  this operand is a relative value
			0x8  this relative value is 8 bits long
			0x10 this relative value is 16/32 bits long (their is a prefix on the instruction to specify the size)
			0x20 this is a relative value, encoded on 4 bytes, independently of a resize prefix
		0x80 this is a specific register
			this register is here > 0x00111000 coded 3 bits

*/

void	disas_text_section(void *text, size_t size);
size_t	find_text_section(void *file_mem, void **text_start);
size_t	file_size(int fd);
t_instruction	*create_instruction(void *mem);
int				verif_prefix_values(char byte);
void	delete_instruction(t_instruction *insts);
void	print_instruction(t_instruction *insts);
t_function	*create_function(t_instruction *start, t_instruction *end);
void	delete_function_lst(t_function **lst);
t_function	*find_functions(t_instruction *insts_lst);
t_opcode	*find_opcode_instruction(U_CHAR opcode_to_find, U_CHAR prefix, U_CHAR opcode_inst_ext, char destroy_table);
t_relative_addr	*create_relative_ref(t_instruction *inst, t_instruction *inst_related, t_function *fct, void *previous);
void		delete_all_rel_ref(t_relative_addr *lst);
void		print_all_ref(t_relative_addr *lst);
t_function	*find_function_related(t_function *fct_lst, unsigned int offset);
t_instruction	*find_instruction_related(t_instruction *inst_lst, unsigned int offset);
t_relative_addr	*find_relative_addr(t_instruction *all_inst_lst, t_function *all_fct_lst);
void	exchange_functions(t_relative_addr *rel_lst, t_function *fct_lst, t_instruction *inst_lst, void *text_sect);

#endif
