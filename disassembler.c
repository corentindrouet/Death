#include "disassembler.h"

int				verif_prefix_values(char byte) {
	static char	grp_prefix_values[14] = {0xf0, 0xf2, 0xf3, 0x2e, 0x36, 0x3e, 0x26, 0x64, 0x65, 0x2e, 0x3e, 0x66, 0x67, 0x00};
	int			i;

	i = 0;
	while (grp_prefix_values[i] != 0x00) {
		if (grp_prefix_values[i] == byte)
			return (1);
		i++;
	}
	return (0);
}

t_instruction	*create_instruction(void *mem) {
	t_instruction	*new_instruction;
	int				grp_prefix_index;

	new_instruction = malloc(sizeof(t_instruction));
	if (!new_instruction)
		return (NULL);
	new_instruction->inst_size = 0;
	new_instruction->next = NULL;
	new_instruction->previous = NULL;
	new_instruction->instruction = mem;
	new_instruction->resize = 0;
	grp_prefix_index = 0;

	/*
	 * Check for the prefix grps, it can be 4 prefix grps:
	 * grp1:
	 * 	0xf0 LOCK prefix
	 * 	0xf2 REPNE/REPNZ prefix
	 * 	0xf3 REP/REPE/REPZ prefix
	 * grp2:
	 * 	0x2e CS segment override
	 * 	0x36 SS segment override
	 * 	0x3e DS segment override
	 * 	0x26 ES segment override
	 * 	0x64 FS segment override
	 * 	0x65 GS segment override
	 * 	0x2e Branch not taken
	 * 	0x3e Branch taken
	 * grp3:
	 * 	0x66 Operand size override
	 * grp4:
	 * 	0x67 Address size override
	 */
	while (grp_prefix_index < 4 && verif_prefix_values(*(char*)mem)) {
		new_instruction->grp_prefix[grp_prefix_index] = *(char*)mem;
		mem++;
		grp_prefix_index++;
		new_instruction->inst_size++;
	}

	/*
	 * Check for the rex prefix, look like this:
	 * +----+-+-+-+-+
	 * |0100|w|r|x|b|
	 * +----+-+-+-+-+
	 */
	new_instruction->rex_prefix = NULL;
	if ((*(char*)mem >> 4) & 0x4) {
		new_instruction->rex_prefix = malloc(sizeof(t_rex_prefix));
		if (!new_instruction->rex_prefix) {
			free(new_instruction);
			return (NULL);
		}
		new_instruction->inst_size++;
		new_instruction->rex_prefix->byte = mem;
		new_instruction->rex_prefix->is_64 = *(char*)mem & 0x8;
		new_instruction->rex_prefix->sib_extension = *(char*)mem & 0x4;
		new_instruction->rex_prefix->reg_extension = *(char*)mem & 0x2;
		new_instruction->rex_prefix->dest_reg_extension = *(char*)mem & 0x1;
		mem++;
	}

	/*
	 * Opcode is directly after prefixs, but can be on 1 or 2 bytes, depending of prefix
	 */
	new_instruction->opcode = (*(char*)mem == 0x0f)? *(short*)mem : *(char*)mem;
	new_instruction->inst_size += (*(char*)mem == 0x0f)? 2 : 1;
	mem += (*(char*)mem == 0x0f)? 2 : 1;

	/*
	 * Now take the ModRM field. It loog like that:
	 * +--+--+--+--+--+--+--+--+
	 * |Mod  |  Reg   |  r/m   |
	 * +--+--+--+--+--+--+--+--+
	 * Mod is for direct or indirect addressing
	 * Reg is for the register use in the instruction
	 * r/m can be register or memory.
	 */
	new_instruction->ModRM = malloc(sizeof(t_mod_rm));
	if (!new_instruction->ModRM) {
		free(new_instruction->rex_prefix);
		free(new_instruction);
		return (NULL);
	}
	new_instruction->inst_size++;
	new_instruction->ModRM->byte = (char*)mem;
	new_instruction->ModRM->direct = (*(char*)mem & 0xc0) >> 6;
	new_instruction->ModRM->reg = (*(char*)mem & 0x38) >> 3;
	new_instruction->ModRM->rm = (*(char*)mem & 0x7);

	/*
	 * Check for the SIB. SIB is optionnal, their is an SIB when:
	 * 	Mov is direct (not 0x11)
	 * 	rm field is equal 0x100
	 */
	new_instruction->SIB = NULL;
	if (new_instruction->ModRM->direct != 0x3 && new_instruction->ModRM->rm == 0x4) {
		new_instruction->SIB = malloc(sizeof(t_sib));
		if (!new_instruction->SIB) {
			free(new_instruction->ModRM);
			free(new_instruction->rex_prefix);
			free(new_instruction);
			return (NULL);
		}
		new_instruction->inst_size++;
		new_instruction->SIB->byte = (char*)mem;
		new_instruction->SIB->scale = (*(char*)mem & 0xc0) >> 6;
		new_instruction->SIB->index = (*(char*)mem & 0x38) >> 3;
		new_instruction->SIB->base = (*(char*)mem & 0x7);
		mem++;

		/*
		 * Displacement is for instruction like: mov rdi, [rax + 0x1234]
		 * 0x1234 is here the displacement, but it can be only on 1/4 bytes, depending of
		 * of the Mod value:
		 *  Mod == 00 -> not displacement
		 *  Mod == 01 -> displacement of 1 byte
		 *  Mod == 10 -> displacement of 4 bytes
		 */
		new_instruction->displacement = 0;
		if (new_instruction->ModRM->direct != 0) {
			new_instruction->displacement = (new_instruction->ModRM->direct == 1) ? *(char*)mem : *(int*)mem;
			mem += (new_instruction->ModRM->direct == 1) ? 1 : 4;
			new_instruction->inst_size += (new_instruction->ModRM->direct == 1) ? 1 : 4;
		}
	}
	return (new_instruction);
}

size_t	file_size(int fd)
{
	off_t	off;

	if (fd < 0)
		return (0);
	lseek(fd, 0, SEEK_SET);
	off = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	if (off == -1)
		return (0);
	return ((size_t)off);
}

size_t	find_text_section(void *file_mem, void **text_start) {
	Elf64_Ehdr	*header;
	Elf64_Shdr	*sec;
	int			i;
	char		*file_content;

	header = file_mem;
	sec = file_mem + header->e_shoff;
	file_content = file_mem + sec[header->e_shstrndx].sh_offset;
	i = 0;
	while (i < header->e_shnum) {
		if (sec->sh_type == SHT_PROGBITS && !strcmp(file_content + sec->sh_name, ".text")) {
			*text_start = file_mem + sec->sh_offset;
			return (sec->sh_size);
		}
		sec++;
		i++;
	}
	return (-1);
}

/*void	disas_text_section(void *text, size_t size) {
	
}*/

int 	main(int argc, char **argv) {
	int		fd;
	size_t	fd_size;
	void	*file_mem;
	void	*text_start;
	size_t	text_size;

	if (argc != 2) {
		printf("Not enough arguments!\n");
		return (0);
	}
	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("Can't open file!\n");
		return (0);
	}
	fd_size = file_size(fd);
	file_mem = mmap(0, fd_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_mem == MAP_FAILED) {
		close(fd);
		printf("Mmap failed!\n");
		return(0);
	}
	text_size = find_text_section(file_mem, &text_start);
	(void)text_size;
	return (0);
}
