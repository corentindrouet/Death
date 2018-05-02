#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <regex.h>

int	ft_atoi_base(const char *str, int base, int total_len);

typedef struct	s_opcode {
	unsigned char	prefix;
	unsigned char	opcode;
	unsigned char	opcode_extension_reg;
	unsigned char	opcode_extension_inst;
	char			mnemonic[16];
	unsigned char	operand[4];
}				t_opcode;

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

void	replace_character(char *str, size_t len, char c_to_replace, char c_replacing) {
	int i;

	i = 0;
	while (i < len) {
		if (*str == c_to_replace)
			*str = c_replacing;
		str++;
		i++;
	}
}

int main(int argc, char **argv) {
	int		fd;
	int		fd_dest;
	int		fd_size;
	char	*file_content;
	regex_t	html_regex;
	regmatch_t tr_table[3];
	char	*table_start;
	char	*table_stop;
	char	*tr_start;
	char	*tr_stop;
	int		i;
	int		td_index;
	t_opcode	tmp;
	char	buff[50];

	if (argc != 3)
		return (0);
	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("Can't open file: %s\n", argv[1]);
		return (0);
	}
	fd_dest = open(argv[2], O_CREAT | O_TRUNC | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd_dest < 0) {
		printf("Can't open file: %s\n", argv[2]);
		close(fd);
		return (0);
	}
	fd_size = file_size(fd);
	file_content = mmap(0, fd_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (file_content == MAP_FAILED) {
		printf("Can't mmap file\n");
		close(fd);
		close(fd_dest);
		return (0);
	}
	if (!regcomp(&html_regex, "<TD[^<>]*>[\n]?([ -\\/]?[[:alpha:]]*[ -\\/]?<[^\\/<>]*>[\n]?)*([[:digit:][:alpha:]\\/\\:]*)[\n]?([ -\\/]?[[:alpha:]]*[ -\\/]?<\\/[^<>]*>[\n]?)*<\\/[^<>]*TD>", REG_EXTENDED | REG_ICASE)) {
		printf("c ok\n");
	}
	table_start = strcasestr(file_content, "<table");
	table_stop = strcasestr(table_start, "</table>");
//	replace_character(table_start, table_stop - table_start, '"', ' ');
	tr_start = strcasestr(table_start, "<tbody");
	tr_stop = strcasestr(tr_start, "</tbody>");
//	tr_start = strcasestr(tr_stop, "<tbody");
//	tr_stop = strcasestr(tr_start, "</tbody>");
	while (tr_stop < table_stop) {
		if (!(i = regexec(&html_regex, tr_start, 3, tr_table, 0))) {
			td_index = 0;
			bzero(&tmp, sizeof(t_opcode));
			bzero(tmp.operand, 4);
			while ((tr_start + tr_table[0].rm_eo) < tr_stop) {
				switch (td_index) {
					case 1:
						tmp.prefix = ft_atoi_base(&(tr_start[tr_table[2].rm_so]), 16, tr_table[2].rm_eo - tr_table[2].rm_so);
						break ;
					case 2:
						if (!memcmp(&(tr_start[tr_table[2].rm_so]) + 2, "+r", 2)) {
							tmp.opcode = ft_atoi_base(&(tr_start[tr_table[2].rm_so]), 16, tr_table[2].rm_eo - tr_table[2].rm_so - 2);
							td_index++;
						} else {
							tmp.opcode = ft_atoi_base(&(tr_start[tr_table[2].rm_so]), 16, tr_table[2].rm_eo - tr_table[2].rm_so);
						}
						break ;
					case 3:
						tmp.opcode_extension_reg = ft_atoi_base(&(tr_start[tr_table[2].rm_so]), 16, tr_table[2].rm_eo - tr_table[2].rm_so);
						break ;
					case 4:
						tmp.opcode_extension_inst = ft_atoi_base(&(tr_start[tr_table[2].rm_so]), 16, tr_table[2].rm_eo - tr_table[2].rm_so);
						break ;
					case 10:
						memcpy(tmp.mnemonic, &(tr_start[tr_table[2].rm_so]), tr_table[2].rm_eo - tr_table[2].rm_so);
						break ;
					case 11 ... 14:
						if (tr_table[2].rm_eo - tr_table[2].rm_so > 0) {
							bzero(buff, 0);
							memcpy(buff, &(tr_start[tr_table[2].rm_so]), tr_table[2].rm_eo - tr_table[2].rm_so);
							tmp.operand[td_index - 11] = 1;
							tmp.operand[td_index - 11] += (strcasestr(buff, "imm")) ? 2 : 0;
							tmp.operand[td_index - 11] += (strcasestr(buff, "8")) ? 4 : 0;
						}
						break ;
				}
				tr_start += tr_table[0].rm_eo;
				td_index++;
				regexec(&html_regex, tr_start , 3, tr_table, 0);
			}
		} else if (i == REG_NOMATCH) {
			printf("NOP\n");
		}
		write(fd_dest, &tmp, sizeof(tmp));
		//printf("%hhx | %hhx | %hhx | %s | %hhx | %hhx | %hhx | %hhx\n", tmp.prefix, tmp.opcode, tmp.opcode_extension, tmp.mnemonic, tmp.operand[0], tmp.operand[1], tmp.operand[2], tmp.operand[3]);
		tr_start = strcasestr(tr_stop, "<tbody");
		tr_stop = strcasestr(tr_start, "</tbody>");
	}
	munmap(file_content, fd_size);
	close(fd);
	close(fd_dest);
	return (0);
}
