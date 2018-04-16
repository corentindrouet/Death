#ifndef DISASSEMBLER_H
# define DISASSEMBLER_H
# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include <sys/mman.h>
# include <elf.h>
# include <string.h>

typedef struct	s_rex_prefix {
	char		*byte;
	char		is_64;
	char		sib_extension;
	char		reg_extension;
	char		dest_reg_extension;
}				t_rex_prefix;

typedef struct	s_mod_rm {
	char		*byte;
	char		direct;
	char		reg;
	char		rm;
}				t_mod_rm;

typedef struct	s_sib {
	char		*byte;
	char		scale;
	char		index;
	char		base;
}				t_sib;

typedef struct	s_instruction {
	char		*instruction;
	t_rex_prefix	*prefix;
	t_mod_rm	*ModRM;
	t_sib		*SIB;
	char		resize;
	char		opcode;
	void		*next;
	void		*previous;
	size_t		inst_size;
}				t_instruction;

#endif
