/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helper.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:09:14 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:14:51 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_otool/nm_otool.h"

char    *text_in_seg(char *lc, int is_64)
{
    unsigned int    i;
    unsigned int    nb;
    unsigned int    sec_size;
    unsigned int    seg_size;
    char            *array;

    i = 0;
    nb = is_64 ? ((struct segment_command_64 *)lc)->nsects : ((struct segment_command *)lc)->nsects;
    sec_size = is_64 ? sizeof(struct section_64) : sizeof(struct section);
    seg_size = is_64 ? sizeof(struct segment_command_64) : sizeof(struct segment_command);
    while(i < nb) {
        array = lc + seg_size + i * sec_size;
        if (!is_safe(array))
            return (NULL);
        if (!ft_strcmp(array, SECT_TEXT) && !ft_strcmp(array + 16, SEG_TEXT))
            return (array);
        i++;
    }
    return (NULL);
}


int    should_swap_arch(unsigned int magic)
{
    return (magic == FAT_CIGAM || magic == FAT_CIGAM_64);
}

int    should_swap_file(unsigned int magic)
{
    if (is_fat_file(magic))
        return should_swap_arch(magic);
    return (magic == MH_CIGAM || magic == MH_CIGAM_64);
}

int     handle_each_arch(char *array, char *ptr, t_fat *fat)
{
    unsigned int    magic_number;
    int             is_64;
    int             should_swap;
    uint64_t        offset;

    if (!is_safe(array))
        return (-1);
    if (!is_hostarch(array, fat->should_swap))
        return (-1);
    if (fat->found_64 && is_hostarch(array, fat->should_swap) == MY_CPU_I386)
        return (-1);
    offset = get_offsize_arch(array, fat->should_swap, fat->is_64);
    magic_number = *(uint32_t *)(ptr + offset);
    if (not_valid_header(magic_number)) {
       return (-1);
    }
    is_64 = is_64_file(magic_number);
    should_swap = should_swap_file(magic_number);
    if (is_hostarch(array, fat->should_swap) == MY_CPU_X86_64)
        fat->found_64 = 1;
    return handle_not_fat(is_64, should_swap, ptr + offset);
}