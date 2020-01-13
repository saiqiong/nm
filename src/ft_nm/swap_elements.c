/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_elements.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:08:21 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 17:10:47 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

void	swap_nb_32(char *ptr, int nb)
{
	int i;

	i = -1;
	while (++i < nb)
		swap_32(ptr + i * 4);
}

void	swap_mach_header(char *ptr)
{
	swap_32(ptr + 16);
}

void	swap_loadcommand(char *ptr)
{
	swap_nb_32(ptr, 2);
}

void	swap_symtab(char *ptr)
{
	swap_nb_32(ptr + 8, 4);
}

void	swap_nlist(char *ptr, int is_64)
{
	swap_nb_32(ptr, 1);
	swap_endiane(ptr + 5, 2);
	if (is_64)
		swap_64(ptr + 7);
	else
		swap_nb_32(ptr + 7, 1);
}
