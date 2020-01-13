/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helper_2.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:06:48 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:28:11 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

char			*get_symbol_name(char *stringtable, char *list, int is_64)
{
	char	*name;
	int		index;

	index = is_64 ? (((struct nlist_64 *)list)->n_un).n_strx :\
	(((struct nlist *)list)->n_un).n_strx;
	name = stringtable + index;
	return (name);
}

t_nm			*add_list(t_nm *list, t_nm *add)
{
	t_nm	*cp;

	if (!add)
		return (list);
	if (!list)
		return (add);
	cp = list;
	while (cp->next)
		cp = cp->next;
	cp->next = add;
	return (list);
}

unsigned int	*get_add(int is_64, struct load_command	*lc)
{
	unsigned int	res;

	res = (is_64 ? ((struct segment_command_64 *)lc)->nsects\
			: ((struct segment_command *)lc)->nsects);
	return (res);
}
