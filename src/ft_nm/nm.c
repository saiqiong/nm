/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:07:08 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:56:26 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

static int		sort_nm_list(t_nm *list, char *flags)
{
	if (ft_strchr(flags, 'p'))
		return (0);
	if (list)
		sort_list_by_name(list, flags);
	return (0);
}

static int		should_swap_arch(unsigned int magic)
{
	return (magic == FAT_CIGAM || magic == FAT_CIGAM_64);
}

static int		should_swap_file(unsigned int magic)
{
	if (is_fat_file(magic))
		return (should_swap_arch(magic));
	return (magic == MH_CIGAM || magic == MH_CIGAM_64);
}

void			free_nm_list(t_nm *list)
{
	t_nm *temp;

	while (list)
	{
		temp = list;
		list = list->next;
		free(temp);
	}
}

int				nm(char *ptr, char *name, t_nu *num, char *flags)
{
	unsigned int	magic_number;
	int				is_fat;
	int				is_64;
	int				should_swap;
	t_nm			*nm_list;

	if (!is_safe(ptr + 32))
		return (put_error_with_return(name, ": is not an object file.\n"));
	magic_number = *(uint32_t *)ptr;
	if (not_valid_file(magic_number))
		return (put_error_with_return(name, ": is not an object file.\n"));
	is_fat = is_fat_file(magic_number);
	is_64 = is_64_file(magic_number);
	should_swap = should_swap_file(magic_number);
	nm_list = handle_nm(is_fat, is_64, should_swap, ptr);
	sort_nm_list(nm_list, flags);
	print_new_line(num->multi, num->first);
	if (print_nm_list(nm_list, name, num->multi, flags) < 0)
		return (-1);
	free_nm_list(nm_list);
	return (0);
}
