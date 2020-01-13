/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   question.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:09:29 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 15:09:32 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_otool/nm_otool.h"

int    is_fat_file(unsigned int magic)
{
    return (magic == FAT_CIGAM || magic == FAT_MAGIC ||  magic == FAT_CIGAM_64 || magic == FAT_MAGIC_64);
}

int     is_64_arch(unsigned int magic)
{
    return (magic == FAT_CIGAM_64 || magic == FAT_MAGIC_64);
}

int    is_64_file(unsigned int magic)
{
    if (is_fat_file(magic))
        return is_64_arch(magic);
    return (magic == MH_CIGAM_64 || magic == MH_MAGIC_64);
}

int     is_hostarch(char *array, int should_swap_arch)
{
    cpu_type_t type;

    type = should_swap_arch ? swap_uint32(*(cpu_type_t *)array) : *(cpu_type_t *)array;
    if (type == CPU_TYPE_X86_64)
        return (MY_CPU_X86_64);
    if (type == CPU_TYPE_I386)
        return (MY_CPU_I386);
    return (0);
}