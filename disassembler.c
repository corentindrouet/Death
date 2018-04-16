#include "disassembler.h"

t_instruction	*create_instruction(void *mem) {
	t_instruction	*new_instruction;

	new_instruction = malloc(sizeof(t_instruction));
	if (!new_instruction)
		return (NULL);
	new_instruction->next = NULL;
	new_instruction->previous = NULL;
	new_instruction->instruction = mem;
	new_instruction->resize = 0;
	if (*(char*)mem == 0x66) {
		new_instruction->resize = 0x66;
		mem++;
	}
	new_instruction->prefix = NULL;
	if (*(char*)mem & 0x01000000) {
		new_instruction->prefix = malloc(sizeof(t_rex_prefix));
		if (!new_instruction->prefix) {
			free(new_instruction);
			return (NULL);
		}
		new_instruction->prefix->byte = mem;
		new_instruction->prefix->is_64 = mem & 0x00001000;
		new_instruction->prefix->sib_extension = mem & 0x00000100;
		new_instruction->prefix->reg_extension = mem & 0x00000010;
		new_instruction->prefix->dest_reg_extension = mem & 0x00000001;
		mem++;
	}
	if (*(char*)mem &)
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

void	disas_text_section(void *text, size_t size) {
	
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
	(void)text_size;
	return (0);
}
