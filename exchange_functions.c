#include "disassembler.h"

void	exchange_functions(t_relative_addr *rel_lst, t_function *fct_lst, t_instruction *inst_lst, void *text_sect) {
//	char	*mem_tmp;
	t_function	*fct_tmp;
	t_relative_addr *rel_tmp;
	int			actual_offset;
	int			inst_guard;
	int			inst_rel_guard;
//	t_function	*first_fct;
//	t_function	*sec_fct;
//	t_relative_addr	*tmp;
//	int				sec_less_first;
//	int				first_less_sec;

	(void)inst_lst;
	fct_tmp = fct_lst;
	while (fct_tmp) {
		if (fct_tmp->start && fct_tmp->end) {
			fct_tmp->fct_copy = malloc(fct_tmp->fct_size + 1);
			memcpy(fct_tmp->fct_copy, text_sect + fct_tmp->start->inst_offset, fct_tmp->fct_size);
		}
		fct_tmp = fct_tmp->next;
	}
	fct_tmp = fct_lst->next;
	actual_offset = fct_lst->start->inst_offset;
	while (fct_tmp) {
		if (fct_tmp->fct_copy) {
			fct_tmp->new_offset = actual_offset;
			memcpy(text_sect + actual_offset, fct_tmp->fct_copy, fct_tmp->fct_size);
			actual_offset += fct_tmp->fct_size;
		}
		fct_tmp = fct_tmp->next;
	}
	if (fct_lst->fct_copy) {
		fct_lst->new_offset = actual_offset;
		memcpy(text_sect + actual_offset, fct_lst->fct_copy, fct_lst->fct_size);
		actual_offset += fct_lst->fct_size;
	}
	rel_tmp = rel_lst;
	while (rel_tmp) {
		fct_tmp = fct_lst;
		inst_guard = 0;
		inst_rel_guard = 0;
		while (fct_tmp) {
			if (!inst_guard && rel_tmp->inst->inst_offset >= fct_tmp->start->inst_offset
				&& rel_tmp->inst->inst_offset < (fct_tmp->start->inst_offset + fct_tmp->fct_size)) {
				rel_tmp->inst->new_offset += (fct_tmp->new_offset - fct_tmp->start->inst_offset);
				inst_guard++;
			}
			if (!inst_rel_guard && rel_tmp->inst_related->inst_offset >= fct_tmp->start->inst_offset
				&& rel_tmp->inst_related->inst_offset < (fct_tmp->start->inst_offset + fct_tmp->fct_size)) {
				rel_tmp->inst_related->new_offset += (int)((int)(fct_tmp->new_offset) - (int)(fct_tmp->start->inst_offset));
				inst_rel_guard++;
			}
			fct_tmp = fct_tmp->next;
		}
		rel_tmp = rel_tmp->next;
	}

	rel_tmp = rel_lst;
	while (rel_tmp) {
//		printf("inst relative: %d offset: %d |inst related: %#x |fct related: %#x\n",
//			rel_tmp->inst->relative, rel_tmp->inst->inst_offset,
//			(rel_tmp->inst_related) ? rel_tmp->inst_related->inst_offset : 0,
//			(rel_tmp->fct_related) ? rel_tmp->fct_related->start->inst_offset : 0);
		*(int*)(text_sect + rel_tmp->inst->new_offset + rel_tmp->inst->relative_offset) = 
			rel_tmp->inst_related->new_offset - (rel_tmp->inst->new_offset + rel_tmp->inst->inst_size);
		rel_tmp = rel_tmp->next;
	}

	fct_tmp = fct_lst;
	while (fct_tmp) {
		free(fct_tmp->fct_copy);
		fct_tmp = fct_tmp->next;
	}
//	mem_tmp = malloc(first_fct->fct_size + 1);
//	if (!mem_tmp)
//		return ;
//	memcpy(mem_tmp, text_sect + first_fct->start->inst_offset, first_fct->fct_size);
//	memcpy(text_sect + first_fct->start->inst_offset, text_sect + sec_fct->start->inst_offset, sec_fct->fct_size);
//	memcpy(text_sect + sec_fct->start->inst_offset, mem_tmp, first_fct->fct_size);
//	free(mem_tmp);
//	sec_less_first = sec_fct->start->inst_offset - first_fct->start->inst_offset;
//	first_less_sec = first_fct->start->inst_offset - sec_fct->start->inst_offset;
//	tmp = rel_lst;
//	while (tmp) {
//		if (tmp->inst_related->inst_offset >= first_fct->start->inst_offset
//			&& tmp->inst_related->inst_offset < (first_fct->start->inst_offset + first_fct->fct_size)) {
//			*(int*)((char*)(text_sect + tmp->inst->inst_offset + tmp->inst->relative_offset)) = 
//		}
//		tmp = tmp->next;
//	}
}
