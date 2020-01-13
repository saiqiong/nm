/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_list_by_name.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:08:00 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 17:06:31 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

static int	compare_add(t_nm *cp, t_nm *jump)
{
	if (cp->is_64)
		return (*(uint64_t *)cp->add - *(uint64_t *)jump->add);
	else
		return (*(uint32_t *)cp->add - *(uint32_t *)jump->add);
}

static void	compare_exchange(t_nm *cp, t_nm *jump)
{
	int				is_64_temp;
	unsigned char	type_temp;
	unsigned char	*add_temp;
	char			*name_temp;

	if (ft_strcmp(cp->name, jump->name) > 0 || (!ft_strcmp(cp->name,\
	jump->name) && compare_add(cp, jump) > 0))
	{
		is_64_temp = cp->is_64;
		type_temp = cp->type;
		add_temp = cp->add;
		name_temp = cp->name;
		cp->is_64 = jump->is_64;
		cp->type = jump->type;
		cp->add = jump->add;
		cp->name = jump->name;
		jump->is_64 = is_64_temp;
		jump->type = type_temp;
		jump->add = add_temp;
		jump->name = name_temp;
	}
}

static void	compare_exchange_reverse(t_nm *cp, t_nm *jump)
{
	int				is_64_temp;
	unsigned char	type_temp;
	unsigned char	*add_temp;
	char			*name_temp;

	if (ft_strcmp(cp->name, jump->name) < 0 || (!ft_strcmp(cp->name,\
	jump->name) && compare_add(cp, jump) < 0))
	{
		is_64_temp = cp->is_64;
		type_temp = cp->type;
		add_temp = cp->add;
		name_temp = cp->name;
		cp->is_64 = jump->is_64;
		cp->type = jump->type;
		cp->add = jump->add;
		cp->name = jump->name;
		jump->is_64 = is_64_temp;
		jump->type = type_temp;
		jump->add = add_temp;
		jump->name = name_temp;
	}
}

void		sort_list_by_name(t_nm *list, char *flags)
{
	t_nm *cp;
	t_nm *jump;

	cp = list;
	if (list)
	{
		while (cp->next)
		{
			jump = cp->next;
			while (jump)
			{
				if (ft_strchr(flags, 'r'))
					compare_exchange_reverse(cp, jump);
				else
					compare_exchange(cp, jump);
				jump = jump->next;
			}
			cp = cp->next;
		}
	}
}
