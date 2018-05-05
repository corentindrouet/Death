#include "disassembler.h"

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

void	disas_text_section(void *text, size_t size) {
	t_instruction	*insts_lst;
	t_instruction	*actual_inst;
	t_instruction	*tmp;
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
		actual_inst = actual_inst->next;
	}
	find_functions(insts_lst);
	actual_inst = insts_lst;
	while (actual_inst) {
		tmp = actual_inst->next;
		delete_instruction(actual_inst);
		actual_inst = tmp;
	}
	return ;
}

int 	main(int argc, char **argv) {
	int		fd;
	size_t	fd_size;
	void	*file_mem;
	void	*text_start;
	size_t	text_size;
	t_opcode	*tmp;

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
	munmap(file_mem, fd_size);
	close(fd);
	return (0);
}
