#include "disassembler.h"

t_function	*create_function(t_instruction *start, t_instruction *end) {
	t_function	*fct;

	fct = NULL;
	fct = malloc(sizeof(t_function));
	if (!fct)
		return (NULL);
	fct->start = start;
	fct->end = end;
	fct->next = NULL;
	fct->previous = NULL;
	return (fct);
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
				fct = create_function(tmp, NULL);
				if (!fct)
					return (NULL);
				fct_tmp = fct;
			} else {
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
			}
		}
		tmp = tmp->next;
	}
	fct_tmp = fct;
	while (fct_tmp) {
		if (fct_tmp->start)
			printf("Function start opcode %#x | at offset : %#x\n", fct_tmp->start->opcode, fct_tmp->start->inst_offset);
		if (fct_tmp->end)
			printf("Function end opcode %#x | at offset : %#x\n\n", fct_tmp->end->opcode, fct_tmp->end->inst_offset);
		fct_tmp = fct_tmp->next;
	}
	return (fct);
}
