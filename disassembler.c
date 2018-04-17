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
	new_instruction->next = NULL;
	new_instruction->previous = NULL;
	new_instruction->instruction = mem;
	new_instruction->resize = 0;
	grp_prefix_index = 0;
	while (grp_prefix_index < 4 && verif_prefix_values(*(char*)mem)) {
		new_instruction->grp_prefix[grp_prefix_index] = *(char*)mem;
		mem++;
		grp_prefix_index++;
	}
	new_instruction->rex_prefix = NULL;
	if ((*(char*)mem >> 4) & 0x4) {
		new_instruction->rex_prefix = malloc(sizeof(t_rex_prefix));
		if (!new_instruction->rex_prefix) {
			free(new_instruction);
			return (NULL);
		}
		new_instruction->rex_prefix->byte = mem;
		new_instruction->rex_prefix->is_64 = *(char*)mem & 0x8;
		new_instruction->rex_prefix->sib_extension = *(char*)mem & 0x4;
		new_instruction->rex_prefix->reg_extension = *(char*)mem & 0x2;
		new_instruction->rex_prefix->dest_reg_extension = *(char*)mem & 0x1;
		mem++;
	}
	new_instruction->opcode = (*(char*)mem == 0x0f)? *(short*)mem : *(char*)mem;
	mem += (*(char*)mem == 0x0f)? 2 : 1;
	new_instruction->ModRM = malloc(sizeof(t_mod_rm));
	if (!new_instruction->ModRM) {
		free(new_instruction->rex_prefix);
		free(new_instruction);
		return (NULL);
	}
	new_instruction->ModRM->byte = (char*)mem;
	new_instruction->ModRM->direct = (*(char*)mem & 0xc0) >> 6;
	new_instruction->ModRM->reg = (*(char*)mem & 0x38) >> 3;
	new_instruction->ModRM->rm = (*(char*)mem & 0x7);

	new_instruction->SIB = malloc(sizeof(t_sib));
	if (!new_instruction->SIB) {
		free(new_instruction->ModRM);
		free(new_instruction->rex_prefix);
		free(new_instruction);
		return (NULL);
	}
	new_instruction->SIB->byte = (char*)mem;
	new_instruction->SIB->scale = (*(char*)mem & 0xc0) >> 6;
	new_instruction->SIB->index = (*(char*)mem & 0x38) >> 3;
	new_instruction->SIB->base = (*(char*)mem & 0x7);
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
