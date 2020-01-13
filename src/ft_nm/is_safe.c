/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   is_safe.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:07:00 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:31:34 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

int		is_safe(char *ptr)
{
	return (ptr - g_begin >= 0 && ptr - g_begin <= g_map_size);
}

int		is_lc_safe(char *ptr, int is_64)
{
	char *lc;

	if (is_64)
		lc = ptr + sizeof(struct mach_header);
	else
		lc = ptr + sizeof(struct mach_header_64);
	return (is_safe(lc));
}

int		is_segmant_safe(char *ptr, int is_64)
{
	if (is_64)
		return (is_safe(ptr + sizeof(struct segment_command_64)));
	return (is_safe(ptr + sizeof(struct segment_command)));
}

int		not_valid_header(unsigned int magic)
{
	return (magic != MH_CIGAM_64 && magic != MH_CIGAM
			&& magic != MH_MAGIC_64 && magic != MH_CIGAM_64
			&& magic != MH_MAGIC && magic != MH_CIGAM);
}

int		not_valid_file(unsigned int magic)
{
	return (magic != FAT_MAGIC && magic != FAT_MAGIC_64
			&& magic != FAT_CIGAM && magic != FAT_CIGAM_64
			&& magic != MH_CIGAM_64 && magic != MH_CIGAM
			&& magic != MH_MAGIC_64 && magic != MH_CIGAM_64
			&& magic != MH_MAGIC && magic != MH_CIGAM);
}
