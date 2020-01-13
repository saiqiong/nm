/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_otool.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:09:05 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:14:51 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_otool/nm_otool.h"

static void    init_t_fat(t_fat *fat, int is_64, int should_swap, char *ptr)
{
    unsigned int     i;
    unsigned int     nb;
    unsigned long    size;

    i = 0;
    fat->found_64 = 0;
    fat->is_64 = is_64;
    fat->should_swap = should_swap;
    nb = should_swap ? swap_uint32(((struct fat_header *)ptr)->nfat_arch) : ((struct fat_header *)ptr)->nfat_arch;
    size = is_64 ? sizeof(struct fat_arch_64) : sizeof(struct fat_arch);
    while (i < nb) {
        if (is_hostarch(ptr + i * size + sizeof(struct fat_header), should_swap) == MY_CPU_X86_64){
            fat->found_64 = 1;
            break;
        }
        i++;
    }
}

static int     handle_fat(int is_64, int should_swap, char *ptr)
{
    unsigned int    nb;
    unsigned int    i;
    t_fat           fat;
    unsigned long   size;

    if (!is_safe(ptr + sizeof(struct fat_header)))
        return (-1);
    nb = should_swap ? swap_uint32(((struct fat_header *)ptr)->nfat_arch) : ((struct fat_header *)ptr)->nfat_arch;
    i = 0;
    init_t_fat(&fat, is_64, should_swap, ptr);
    size = is_64 ? sizeof(struct fat_arch_64) : sizeof(struct fat_arch);
    while (i < nb) {
        handle_each_arch(ptr + i * size + sizeof(struct fat_header), ptr, &fat);
        i++;
    }
    return (0);
}

int             handle_otool(int is_fat, int is_64, int should_swap, char *ptr)
{
    if (is_fat)
        return handle_fat(is_64, should_swap, ptr);
    else
        return handle_not_fat(is_64, should_swap, ptr);
}