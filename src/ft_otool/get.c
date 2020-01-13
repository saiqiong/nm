/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:08:48 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 15:08:51 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_otool/nm_otool.h"

int    get_lc(t_lc *lc_info, int is_64, int should_swap, char *ptr)
{
    struct mach_header      *header;
    struct mach_header_64   *header_64;

    if (!is_lc_safe(ptr, is_64))
        return (-1);
    header = (struct mach_header *)ptr;
    header_64 = (struct mach_header_64 *)ptr;
    if (should_swap)
        swap_mach_header(ptr);
    lc_info->number_cmds = is_64 ? header_64->ncmds : header->ncmds;
    lc_info->offset_cmds = is_64? sizeof(*header_64) : sizeof(*header);
    return (0);
}

uint64_t get_offsize_arch(char *array, int should_swap_arch, int is_64_arch)
{
    uint64_t        offset_64;
    uint32_t        offset;

    if (is_64_arch) {
        if (should_swap_arch)
            offset_64 = swap_uint64(((struct fat_arch_64 *)array)->offset);
        else
            offset_64 = ((struct fat_arch_64 *)array)->offset;
    }
    else {
        if (should_swap_arch)
            offset = swap_uint32(((struct fat_arch *)array)->offset);
        else
            offset = ((struct fat_arch *)array)->offset;
        offset_64 = (uint64_t )offset;
    }
    return offset_64;
}

int     get_fd(char *name)
{
    int         fd;
    struct stat buf;

    if ((fd = open(name,  O_RDONLY)) < 0){
        if (errno == EACCES)
            put_error(name, ": Permission denied.\n");
        else
            put_error(name, ": No such file or directory.\n");
        return (-1);
    }
    if (fstat(fd, &buf) < 0) {
        close(fd);
        return (-1);
    }
    if (S_ISDIR(buf.st_mode)) {
        put_error(name, ": Is a direcrory.\n");
        return (-1);
    }
    return fd;
}

