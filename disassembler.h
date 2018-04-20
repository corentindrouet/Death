#ifndef DISASSEMBLER_H
# define DISASSEMBLER_H
# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include <sys/mman.h>
# include <elf.h>
# include <string.h>
# include <stdlib.h>

typedef struct	s_rex_prefix {
	unsigned char		*byte;
	unsigned char		is_64;
	unsigned char		sib_extension;
	unsigned char		reg_extension;
	unsigned char		dest_reg_extension;
}				t_rex_prefix;

typedef struct	s_mod_rm {
	unsigned char		*byte;
	unsigned char		direct;
	unsigned char		reg;
	unsigned char		rm;
}				t_mod_rm;

typedef struct	s_sib {
	unsigned char		*byte;
	unsigned char		scale;
	unsigned char		index;
	unsigned char		base;
}				t_sib;

typedef struct	s_instruction {
	unsigned char		*instruction;
	unsigned char		grp_prefix[4];
	int					nb_grp_prefix;
	t_rex_prefix	*rex_prefix;
	t_mod_rm	*ModRM;
	t_sib		*SIB;
	unsigned char		resize;
	unsigned int			opcode;
	unsigned int			displacement;
	unsigned int			immediate;
	void		*next;
	void		*previous;
	size_t		inst_size;
}				t_instruction;

void	disas_text_section(void *text, size_t size);
size_t	find_text_section(void *file_mem, void **text_start);
size_t	file_size(int fd);
t_instruction	*create_instruction(void *mem);
int				verif_prefix_values(char byte);
void	delete_instruction(t_instruction *insts);

#endif
