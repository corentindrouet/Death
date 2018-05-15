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
			insts_lst->inst_offset = total_size_treated;
			total_size_treated += insts_lst->inst_size;
			if (!insts_lst)
				return ;
			print_instruction(insts_lst);
			printf("\n");
			fflush(stdout);
		} else {
			actual_inst = insts_lst;
			while (actual_inst->next)
				actual_inst = actual_inst->next;
			actual_inst->next = create_instruction(text + total_size_treated);
			if (!actual_inst->next)
				return ;
			print_instruction(actual_inst);
			printf("\n");
			fflush(stdout);
			((t_instruction*)(actual_inst->next))->inst_offset = total_size_treated;
			total_size_treated += ((t_instruction*)(actual_inst->next))->inst_size;
			((t_instruction*)(actual_inst->next))->previous = actual_inst;
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

//void	sort_by_values(void **sym_table) {
//
//}
//
//void	disas_by_symbols(void *file_mem, void *text_start, size_t text_size) {
//	Elf64_Ehdr	*header;
//	Elf64_Shdr	*sec;
//	Elf64_Shdr	*sec_sym;
//	Elf64_Sym	*sym;
//	char		*file_content;
//	char		*strtab;
//	unsigned int	i;
//	void		**sym_table_ordered;
//
//	header = file_mem;
//	sec = file_mem + header->e_shoff;
//
//	file_content = file_mem + sec[header->e_shstrndx].sh_offset;
//
//	(void)text_start;
//	(void)text_size;
//	i = 0;
//	while (i < header->e_shnum) {
//		if (sec->sh_type == SHT_SYMTAB) {
//			sec_sym = sec;
//			sym = file_mem + sec->sh_offset;
//		} else if (sec->sh_type == SHT_STRTAB && !strcmp(file_content + sec->sh_name, ".strtab")) {
//			strtab = file_mem + sec->sh_offset;
//		}
//		sec++;
//		i++;
//	}
//
//	sec = file_mem + header->e_shoff
//	sym_table_ordered = malloc((sec_sym->sh_size / sizeof(Elf64_Sym)) * 8 + 8);
//	i = 0;
//	while (i < sec_sym->sh_size) {
//		if (!strcmp(file_content + sec[(sym->st_shndx < 0x8888) ? sym->st_shndx : 0].sh_name, ".text") && sym->st_value >= sec[sym->st_shndx].sh_addr && sym->st_value < (sec[sym->st_shndx].sh_addr + sec[sym->st_shndx].sh_size)) {
////			printf("name %18s |value %#lx |info %#hhx |other %#hhx |size %#lx\n", strtab + sym->st_name, sym->st_value, sym->st_info, sym->st_other, sym->st_size);
//			
//		}
//		i += sym->st_size + sizeof(Elf64_Sym);
//		sym = (void*)sym + sym->st_size + sizeof(Elf64_Sym);
//	}
//
//}

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
//	disas_by_symbols(file_mem, text_start, text_size);
	munmap(file_mem, fd_size);
	close(fd);
	return (0);
}
