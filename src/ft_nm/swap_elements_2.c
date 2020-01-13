/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_elements_2.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:08:31 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 17:12:12 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

void	swap_segment_command(char *ptr, int is_64)
{
	if (is_64)
		swap_nb_32(ptr + 42 + 2 * sizeof(vm_prot_t), 1);
	else
		swap_nb_32(ptr + 26 + 2 * sizeof(vm_prot_t), 1);
}

void	swap_arch(char *ptr, int is_64)
{
	if (is_64)
	{
		swap_64(ptr + 8);
		swap_64(ptr + 16);
	}
	else
		swap_nb_32(ptr + 8, 2);
}

void	swap_section(char *ptr, int is_64)
{
	if (is_64)
	{
		swap_64(ptr + 4);
		swap_64(ptr + 12);
		swap_32(ptr + 20);
	}
	else
		swap_nb_32(ptr + 4, 3);
}
