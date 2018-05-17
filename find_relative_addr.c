#include "disassembler.h"

t_relative_addr	*create_relative_ref(t_instruction *inst, t_instruction *inst_related, t_function *fct, void *previous) {
	t_relative_addr	*rel_ref;

	rel_ref = malloc(sizeof(t_relative_addr));
	if (!rel_ref)
		return (NULL);
	rel_ref->inst = inst;
	rel_ref->inst_related = inst_related;
	rel_ref->fct_related = fct;
	rel_ref->previous = previous;
	return (rel_ref);
}

void		delete_all_rel_ref(t_relative_addr *lst) {
	t_relative_addr	*tmp;

	tmp = lst;
	while (tmp->next) {
		tmp = tmp->next;
		free(tmp->previous);
	}
	free(tmp);
}

void		print_all_ref(t_relative_addr *lst) {
	t_relative_addr	*tmp;

	tmp = lst;
	while (tmp) {
//		printf("inst relative: %d offset: %d |inst related: %#x |fct related: %#x\n",
//			tmp->inst->relative, tmp->inst->inst_offset,
//			(tmp->inst_related) ? tmp->inst_related->inst_offset : 0,
//			(tmp->fct_related) ? tmp->fct_related->start->inst_offset : 0);
		tmp = tmp->next;
	}
}

t_function	*find_function_related(t_function *fct_lst, unsigned int offset) {
	t_function	*tmp_fct;

	tmp_fct = fct_lst;
	while (tmp_fct) {
		if (tmp_fct->start->inst_offset == offset)
			return (tmp_fct);
		tmp_fct = tmp_fct->next;
	}
	return (NULL);
}

t_instruction	*find_instruction_related(t_instruction *inst_lst, unsigned int offset) {
	t_instruction	*tmp_inst;

	tmp_inst = inst_lst;
	while (tmp_inst) {
		if (tmp_inst->inst_offset == offset)
			return (tmp_inst);
		tmp_inst = tmp_inst->next;
	}
	return (NULL);
}

t_relative_addr	*find_relative_addr(t_instruction *all_inst_lst, t_function *all_fct_lst) {
	t_instruction	*actual_inst;
	t_relative_addr	*rel_ref;
	t_relative_addr	*tmp_rel_ref;
	unsigned int	offset;

	actual_inst = all_inst_lst;
	rel_ref = NULL;
	tmp_rel_ref = rel_ref;
	while (actual_inst) {
		if (actual_inst->relative) {
			offset = actual_inst->inst_offset + actual_inst->inst_size + actual_inst->relative;
			if (!rel_ref) {
				rel_ref = create_relative_ref(actual_inst,
							find_instruction_related(all_inst_lst, offset),
							find_function_related(all_fct_lst, offset), NULL);
				tmp_rel_ref = rel_ref;
			} else {
				tmp_rel_ref->next = create_relative_ref(actual_inst,
							find_instruction_related(all_inst_lst, offset),
							find_function_related(all_fct_lst, offset), tmp_rel_ref);
				tmp_rel_ref = tmp_rel_ref->next;
			}
		}
		actual_inst = actual_inst->next;
	}
	print_all_ref(rel_ref);
//	delete_all_rel_ref(rel_ref);
	return (rel_ref);
}
