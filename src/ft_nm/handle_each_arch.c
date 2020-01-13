/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_each_arch.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:04:27 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:14:51 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

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

t_nm			*handle_each_arch(char *array, char *ptr, t_fat *fat)
{
	unsigned int	magic_number;
	int				is_64;
	int				should_swap;
	uint64_t		offset;

	if (!is_safe(array))
		return (NULL);
	if (!is_hostarch(array, fat->should_swap))
		return (NULL);
	if (fat->found_64 && is_hostarch(array, fat->should_swap) == MY_CPU_I386)
		return (NULL);
	offset = get_offsize_arch(array, fat->should_swap, fat->is_64);
	magic_number = *(uint32_t *)(ptr + offset);
	if (not_valid_header(magic_number))
		return (NULL);
	is_64 = is_64_file(magic_number);
	should_swap = should_swap_file(magic_number);
	if (is_hostarch(array, fat->should_swap) == MY_CPU_X86_64)
		fat->found_64 = 1;
	return (handle_not_fat(is_64, should_swap, ptr + offset));
}
