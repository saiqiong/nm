/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dump.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:03:50 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 17:21:00 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

void	hexdump_address(unsigned char *add, int is_64)
{
	int				i;
	int				count;
	unsigned int	test;

	i = 0;
	count = is_64 ? 8 : 4;
	while (--count >= 0)
	{
		test = add[count];
		ft_printf("%.2x", test);
	}
}

void	put_error(char *s1, char *s2)
{
	write(2, s1, ft_strlen(s1));
	write(2, s2, ft_strlen(s2));
}

int		return_error(char *s1)
{
	write(2, s1, ft_strlen(s1));
	return (EXIT_FAILURE);
}

int		put_error_with_return(char *s1, char *s2)
{
	write(2, s1, ft_strlen(s1));
	write(2, s2, ft_strlen(s2));
	return (-1);
}
