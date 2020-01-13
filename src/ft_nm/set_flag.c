/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   set_flag.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:07:53 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 17:02:57 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

static int	is_flag(char c)
{
	return (c == 'g' || c == 'p' || c == 'r' ||\
	c == 'u' || c == 'U' || c == 'j');
}

static int	put_usage(char **av)
{
	char	*str1;
	char	*str2;

	str1 = "ft_nm: illegal option -- ";
	str2 = "usage: ft_nm [-gpruUj] [file ...]\n";
	write(2, str1, ft_strlen(str1));
	write(2, *av, 1);
	write(2, "\n", 1);
	write(2, str2, ft_strlen(str2));
	return (-1);
}

int			set_flags_nm(char *flags, char **av)
{
	int		ct;
	int		i;

	ct = 1;
	i = 0;
	av++;
	ft_bzero(flags, 10);
	while (*av && (*av)[0] == '-' && (*av)[1])
	{
		(*av)++;
		while (**av)
		{
			if (is_flag(**av))
			{
				if (!ft_strchr(flags, **av))
					flags[i++] = *((*av));
				(*av)++;
			}
			else
				return (put_usage(av));
		}
		(av)++;
		ct++;
	}
	return (ct);
}
