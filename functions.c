#include "disassembler.h"

t_function	*create_function(t_instruction *start, t_instruction *end) {
	t_function	*fct;

	fct = NULL;
	fct = malloc(sizeof(t_function));
	if (!fct)
		return (NULL);
	fct->start = start;
	fct->end = end;
	if (fct->start && fct->end) {
		fct->fct_size = fct->end->inst_offset - fct->start->inst_offset + fct->end->inst_size;
	} else {
		fct->fct_size = fct->start->inst_size;
	}
	fct->next = NULL;
	fct->previous = NULL;
	fct->fct_copy = NULL;
	fct->fct_or_code = 0;
	return (fct);
}

void	update_fct_insts_offset(t_function *fct) {
	t_instruction *start;
	t_instruction *end;

	start = fct->start;
	end = fct->end;
	while (start && start->inst_offset <= end->inst_offset) {
		start->fct = fct;
		start = start->next;
	}
}

void	delete_function_lst(t_function **lst) {
	t_function *tmp;
	
	tmp = *lst;
	while (tmp) {
		if (tmp->previous) {
			free(tmp->previous);
		}
		if (!tmp->next) {
			free(tmp);
		} else 
			tmp = tmp->next;
	}
}

t_function	*find_functions(t_instruction *insts_lst) {
	t_instruction	*tmp;
	t_function		*fct;
	t_function		*fct_tmp;

	tmp = insts_lst;
	fct = NULL;
	fct_tmp = NULL;
	while (tmp) {
		if (tmp->opcode == 0xc8 
				|| (tmp->opcode == 0x55 
					&& tmp->next
					&& ((t_instruction*)(tmp->next))->opcode == 0x89
					&& ((t_instruction*)(tmp->next))->ModRM
					&& ((t_instruction*)(tmp->next))->ModRM->direct == 0x3
					&& ((t_instruction*)(tmp->next))->ModRM->reg == 0x4
					&& ((t_instruction*)(tmp->next))->ModRM->rm == 0x5)) {
			if (!fct) {
				if (tmp != insts_lst) {
					fct = create_function(insts_lst, tmp->previous);
					update_fct_insts_offset(fct);
					fct->fct_or_code = 1;
					if (!fct)
						return (NULL);
					fct->next = create_function(tmp, NULL);
					fct_tmp = fct->next;
				} else {
					fct = create_function(tmp, NULL);
					fct_tmp = fct;
				}
				if (!fct)
					return (NULL);
			} else {
				if (tmp != fct_tmp->end->next) {
					fct_tmp->next = create_function(fct_tmp->end->next, tmp->previous);
					update_fct_insts_offset(fct_tmp->next);
					((t_function*)(fct_tmp->next))->fct_or_code = 1;
					fct_tmp = fct_tmp->next;
				}
				fct_tmp->next = create_function(tmp, NULL);
				if (!fct_tmp->next)
					return (NULL);
				fct_tmp = fct_tmp->next;
			}
		} else if (tmp->opcode == 0xc2
				|| tmp->opcode == 0xc3
				|| tmp->opcode == 0xca
				|| tmp->opcode == 0xcb) {
			if (fct_tmp) {
				fct_tmp->end = tmp;
				if (fct_tmp->start) {
					fct_tmp->fct_size = fct_tmp->end->inst_offset - fct_tmp->start->inst_offset + fct_tmp->end->inst_size;
					update_fct_insts_offset(fct_tmp);
				}
			}
		}
		tmp = tmp->next;
	}
	fct_tmp = fct;
	while (fct_tmp) {
		if (fct_tmp->start)
			printf("Function start opcode %#x |at offset: %#x |size: %lu |is_fct: %d\n", fct_tmp->start->opcode, fct_tmp->start->inst_offset, fct_tmp->fct_size, fct_tmp->fct_or_code);
		if (fct_tmp->end)
			printf("Function end opcode %#x |at offset: %#x |size: %lu |is_fct: %d\n\n", fct_tmp->end->opcode, fct_tmp->end->inst_offset, fct_tmp->fct_size, fct_tmp->fct_or_code);
		fct_tmp = fct_tmp->next;
	}
	return (fct);
}
