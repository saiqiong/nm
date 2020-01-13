/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_not_fat.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:08:56 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:14:51 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_otool/nm_otool.h"

static char    *get_text_section(t_lc lc_info, char *ptr, int should_swap, int is_64)
{
    unsigned int            i;
    struct load_command     *lc;
    char                    *res;


    i = 0;
    lc = (void *)ptr + lc_info.offset_cmds;
    while (i < lc_info.number_cmds) {
        if (!is_safe((char *)lc))
            break;
        if (should_swap){
            swap_loadcommand((char *)lc);
        }
        if (lc->cmd == LC_SEGMENT || lc->cmd == LC_SEGMENT_64) {
            if (!is_segmant_safe((char *)lc, is_64))
                return NULL;
            if (should_swap)
                swap_segment_command((char *)lc, is_64);
           if ((res = text_in_seg((char *)lc, is_64)))
            return res;
        }
        lc = (struct load_command *)((char *)lc + lc->cmdsize);
        i++;
    }
    return NULL;
}

static int    put_text_section(char *sec, int should_swap, int is_64, char *ptr)
{
    unsigned int    sec_size;
    t_nb            nb;

    sec_size = is_64 ? sizeof(struct section_64) : sizeof(struct section);
    if (!is_safe(sec + sec_size))
        return (-1);
    if (should_swap)
        swap_section(sec, is_64);
    nb.size = is_64 ? ((struct section_64 *)sec)->size : (uint64_t)((struct section *)sec)->size ;
    nb.offset = is_64 ? ((struct section_64 *)sec)->offset : ((struct section *)sec)->offset;
    ft_putstr("Contents of (__TEXT,__text) section\n");
    dump_section(sec + 32, is_64, &nb, ptr);
    return (0);
}

int         handle_not_fat(int is_64, int should_swap, char *ptr)
{
    unsigned int            ncmds;
    t_lc                    lc_info;
    char                    *section;

    if (!is_safe(ptr))
        return (-1);
    if (get_lc(&lc_info, is_64, should_swap, ptr) < 0)
        return (-1);
    ncmds = lc_info.number_cmds;
    section = get_text_section(lc_info, ptr, should_swap, is_64);
    put_text_section(section, should_swap, is_64, ptr);
    return (0);
}