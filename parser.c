#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <regex.h>

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
	if (!regcomp(&html_regex, "<TD[^<>]*>(<[^<>]*>)*([a-zA-Z]*)(<[^<>]*>)*<\\/[^<>]*TD>", REG_EXTENDED | REG_ICASE)) {
		printf("c ok\n");
	}
	table_start = strcasestr(file_content, "<table");
	table_stop = strcasestr(table_start, "</table>");
	tr_start = strcasestr(table_start, "<tr");
	tr_stop = strcasestr(tr_start, "</tr>");
	tr_start = strcasestr(tr_stop, "<tr");
	tr_stop = strcasestr(tr_start, "</tr>");
	while (tr_stop < table_stop) {
		if (!(i = regexec(&html_regex, tr_start, 3, tr_table, 0))) {
			while ((tr_start + tr_table[0].rm_eo) < tr_stop) {
				write(1, "[=] ", 4);
				write(1, &(tr_start[tr_table[2].rm_so]), tr_table[2].rm_eo - tr_table[2].rm_so);
				write(1, "\n", 1);
				tr_start += tr_table[0].rm_eo;
				regexec(&html_regex, tr_start , 3, tr_table, 0);
			}
		} else if (i == REG_NOMATCH) {
			printf("NOP\n");
		}
		tr_start = strcasestr(tr_stop, "<tr");
		tr_stop = strcasestr(tr_start, "</tr>");
	}
	munmap(file_content, fd_size);
	close(fd);
	close(fd_dest);
	return (0);
}
