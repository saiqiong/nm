/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_not_fat.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:06:44 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:18:20 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

static t_nm		*free_list_return_null(t_nm *list)
{
	free_nm_list(list);
	return (NULL);
}

static void		h(t_nm **l, unsigned int *i, int s, struct symtab_command *m)
{
	*l = NULL;
	if (s)
		swap_symtab((char *)m);
	*i = 0;
}

static t_nm		*list_symtab(struct load_command *lc, char *ptr, int s, int x)
{
	unsigned int			i;
	char					*stringtable;
	struct symtab_command	*sym;
	t_nm					*list;
	t_nm					*add;

	sym = (struct symtab_command *)lc;
	if (!is_safe((char *)lc + sizeof(*sym)))
		return (NULL);
	h(&list, &i, s, sym);
	stringtable = (void *)ptr + sym->stroff;
	while (i < sym->nsyms)
	{
		if (!is_safe(stringtable) || !is_safe(get_nlist(ptr, i, x, sym)))
			return (free_list_return_null(list));
		if (s)
			swap_nlist(get_nlist(ptr, i, x, sym), x);
		add = init_one_nm(ptr, get_nlist(ptr, i, x, sym), s, x);
		add->name = get_symbol_name(stringtable, get_nlist(ptr, i, x, sym), x);
		if (!is_safe(add->name))
			return (free_list_return_null(list));
		list = add_list(list, add);
		i++;
	}
	return (list);
}

static t_nm		*get_nm_list(t_lc lc_info, char *ptr, int s, int is_64)
{
	unsigned int			i;
	struct load_command		*lc;

	i = 0;
	lc = (void *)ptr + lc_info.offset_cmds;
	while (i < lc_info.number_cmds)
	{
		if (!is_safe((char *)lc + sizeof(*lc)))
			break ;
		if (s)
			swap_loadcommand((char *)lc);
		if (lc->cmd == LC_SYMTAB)
			return (list_symtab(lc, ptr, s, is_64));
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
		i++;
	}
	return (NULL);
}

t_nm			*handle_not_fat(int is_64, int should_swap, char *ptr)
{
	t_lc	lc_info;

	if (!is_safe(ptr))
		return (NULL);
	if (get_lc(&lc_info, is_64, should_swap, ptr) < 0)
		return (NULL);
	return (get_nm_list(lc_info, ptr, should_swap, is_64));
}
