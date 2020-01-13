/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_nm_list.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:07:12 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 17:00:07 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

static int	is_extern(char c)
{
	if (c >= 'A' && c <= 'Z')
		return (1);
	return (0);
}

static int	can_show_list(t_nm *list, char *flags)
{
	if (list->type == '-')
		return (0);
	if (ft_strchr(flags, 'u') && ft_strchr(flags, 'U'))
		return (0);
	if (ft_strchr(flags, 'u'))
	{
		if (ft_strchr(flags, 'g'))
			return (list->type == 'U');
		return (list->type == 'u' || list->type == 'U');
	}
	if (ft_strchr(flags, 'U'))
	{
		if (ft_strchr(flags, 'g'))
			return (is_extern(list->type) && list->type != 'U');
		return (list->type != 'u' && list->type != 'U');
	}
	return (1);
}

int			print_nm_list(t_nm *list, char *name, int mu, char *flags)
{
	if (!list)
	{
		put_error(name, ": Is malformed file.\n");
		return (-1);
	}
	if (mu)
		ft_printf("%s:\n", name);
	while (list)
	{
		if (can_show_list(list, flags))
		{
			if (!ft_strchr(flags, 'u') && !ft_strchr(flags, 'j'))
			{
				hexdump_address(list->add, list->is_64);
				ft_printf(" %c ", list->type);
			}
			ft_printf("%s\n", list->name);
		}
		list = list->next;
	}
	return (0);
}
