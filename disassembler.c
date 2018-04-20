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
	new_instruction->opcode = 0;
	new_instruction->next = NULL;
	new_instruction->previous = NULL;
	new_instruction->instruction = mem;
	new_instruction->resize = 0;
	new_instruction->displacement = 0;
	new_instruction->immediate = 0;
	new_instruction->ModRM = NULL;
	new_instruction->SIB = NULL;
	new_instruction->rex_prefix = NULL;
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
	while (grp_prefix_index < 4 && verif_prefix_values(*(unsigned char*)mem)) {
		new_instruction->grp_prefix[grp_prefix_index] = *(unsigned char*)mem;
		mem++;
		grp_prefix_index++;
		new_instruction->inst_size++;
		new_instruction->nb_grp_prefix++;
	}

	/*
	 * Check for the rex prefix, look like this:
	 * +----+-+-+-+-+
	 * |0100|w|r|x|b|
	 * +----+-+-+-+-+
	 */
	if (!((*(unsigned char*)mem >> 4) ^ 0x4)) {
		new_instruction->rex_prefix = malloc(sizeof(t_rex_prefix));
		if (!new_instruction->rex_prefix) {
			free(new_instruction);
			return (NULL);
		}
		new_instruction->inst_size++;
		new_instruction->rex_prefix->byte = mem;
		new_instruction->rex_prefix->is_64 = *(unsigned char*)mem & 0x8;
		new_instruction->rex_prefix->sib_extension = *(unsigned char*)mem & 0x4;
		new_instruction->rex_prefix->reg_extension = *(unsigned char*)mem & 0x2;
		new_instruction->rex_prefix->dest_reg_extension = *(unsigned char*)mem & 0x1;
		mem++;
	}

	/*
	 * Opcode is directly after prefixs, but can be on 1 or 2 bytes, depending of prefix
	 */
	new_instruction->opcode = (*(unsigned char*)mem == 0x0f)? *(unsigned short*)mem : *(unsigned char*)mem;
	new_instruction->inst_size += (*(unsigned char*)mem == 0x0f)? 2 : 1;
	mem += (*(unsigned char*)mem == 0x0f)? 2 : 1;
	if ((new_instruction->opcode >= 0x40 && new_instruction->opcode <= 0x5f))
		return (new_instruction);
	if ((new_instruction->opcode >= 0xb0 && new_instruction->opcode <= 0xbf)) {
		new_instruction->immediate = *(int*)mem;
		new_instruction->inst_size += 4;
		return (new_instruction);
	}

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
	new_instruction->ModRM->byte = (unsigned char*)mem;
	new_instruction->ModRM->direct = (*(unsigned char*)mem & 0xc0) >> 6;
	new_instruction->ModRM->reg = (*(unsigned char*)mem & 0x38) >> 3;
	new_instruction->ModRM->rm = (*(unsigned char*)mem & 0x7);
	mem++;

	/*
	 * Check for the SIB. SIB is optionnal, their is an SIB when:
	 * 	Mov is direct (not 0x11)
	 * 	rm field is equal 0x100
	 */
	if (new_instruction->ModRM->direct != 0x3 && new_instruction->ModRM->rm == 0x4) {
		new_instruction->SIB = malloc(sizeof(t_sib));
		if (!new_instruction->SIB) {
			free(new_instruction->ModRM);
			free(new_instruction->rex_prefix);
			free(new_instruction);
			return (NULL);
		}
		new_instruction->inst_size++;
		new_instruction->SIB->byte = (unsigned char*)mem;
		new_instruction->SIB->scale = (*(unsigned char*)mem & 0xc0) >> 6;
		new_instruction->SIB->index = (*(unsigned char*)mem & 0x38) >> 3;
		new_instruction->SIB->base = (*(unsigned char*)mem & 0x7);
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
		if (new_instruction->ModRM->direct != 0
				|| (new_instruction->SIB->index == 0x4 && new_instruction->SIB->base == 0x5)) {
			new_instruction->displacement = (new_instruction->ModRM->direct == 1) ? *(unsigned char*)mem : *(unsigned int*)mem;
			mem += (new_instruction->ModRM->direct == 1) ? 1 : 4;
			new_instruction->inst_size += (new_instruction->ModRM->direct == 1) ? 1 : 4;
		}
	}
	return (new_instruction);
}

void	delete_instruction(t_instruction **insts) {
	if (!*insts)
		return ;
	free((*insts)->rex_prefix);
	free((*insts)->ModRM);
	free((*insts)->SIB);
	free(*insts);
}

size_t	file_size(int fd) {
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

void	print_instruction(t_instruction *insts) {
	int	i;

	printf("Grp prefix: %d\n", insts->nb_grp_prefix);
	for (i = 0; i < insts->nb_grp_prefix; i++)
		printf("  prefix %d: %#hhx\n", i, insts->grp_prefix[i]);

	if (insts->rex_prefix) {
		printf("Rex prefix:\n");
		printf("  Byte: %#hhx\n", *(insts->rex_prefix->byte));
		printf("  Is_64: %#hhx\n", insts->rex_prefix->is_64);
		printf("  Sib extension: %#hhx\n", insts->rex_prefix->sib_extension);
		printf("  Reg extension: %#hhx\n", insts->rex_prefix->reg_extension);
		printf("  Dest reg extension: %#hhx\n", insts->rex_prefix->dest_reg_extension);
	}

	printf("Opcode: %#x\n", insts->opcode);

	if (insts->ModRM) {
		printf("Mod R/M:\n");
		printf("  Byte: %#hhx\n", *(insts->ModRM->byte));
		printf("  Direct: %#hhx\n", insts->ModRM->direct);
		printf("  Reg: %#hhx\n", insts->ModRM->reg);
		printf("  R/M: %#hhx\n", insts->ModRM->rm);
	}

	if (insts->SIB) {
		printf("SIB:\n");
		printf("  Byte: %#hhx\n", *(insts->SIB->byte));
		printf("  Scale: %#hhx\n", insts->SIB->scale);
		printf("  Index: %#hhx\n", insts->SIB->index);
		printf("  Base: %#hhx\n", insts->SIB->base);
	}

	printf("Displacement: %#x\n", insts->displacement);
	printf("Immediate: %#x\n", insts->immediate);
	printf("Inst size: %lu\n", insts->inst_size);
}

void	disas_text_section(void *text, size_t size) {
	t_instruction	*insts_lst;
	t_instruction	*actual_inst;
	size_t			total_size_treated;

	insts_lst = NULL;
	total_size_treated = 0;
	while (total_size_treated < size) {
		if (!insts_lst) {
			insts_lst = create_instruction(text);
			total_size_treated += insts_lst->inst_size;
			if (!insts_lst)
				return ;
		} else {
			actual_inst = insts_lst;
			while (actual_inst->next)
				actual_inst = actual_inst->next;
			actual_inst->next = create_instruction(text + total_size_treated);
			if (!actual_inst->next)
				return ;
			total_size_treated += ((t_instruction*)actual_inst->next)->inst_size;
			((t_instruction*)actual_inst->next)->previous = actual_inst;
		}
	}
	actual_inst = insts_lst;
	while (actual_inst) {
		print_instruction(actual_inst);
		printf("\n");
//		if (actual_inst->previous) {
//			delete_instruction(&(t_instruction*)(actual_inst->previous));
//		}
		actual_inst = actual_inst->next;
	}
	return ;
}

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
	disas_text_section(text_start, text_size);
	return (0);
}
