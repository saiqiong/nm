/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dump.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:08:41 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:14:51 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_otool/nm_otool.h"

void    hexdump_address(unsigned char *add, int is_64)
{
    int     i;
    int     count;
    unsigned int   test;

    i = 0;
    count = is_64 ? 8 : 4;
    while(--count >= 0) {
    test = add[count];
    ft_printf("%.2x", test);
    }
}

void    hexdump_data(unsigned char *add, uint64_t i, uint64_t size)
{
    int     j;
    int     count;
    unsigned int   test;

    j = 0;
    count = 16;
    while(j < count) {
    if (j + i == size)
        break;
    test = add[j];
    if (j == count - 1)
    ft_printf("%.2x ", test);
    else
    ft_printf("%.2x ", test);
    j++;
    }
}

void    put_error(char *s1, char *s2)
{
    write(2, s1, ft_strlen(s1));
    write(2, s2, ft_strlen(s2));
}

int    dump_section(char *add, int is_64, t_nb *nb, char *ptr)
{
    uint64_t        i;
    uint64_t        plus;
    char            *str;


    plus = 16;
    i = 0;
    while (i < nb->size){
        str = ptr + nb->offset + i;
        if (!is_safe(str))
            return (-1);
        hexdump_address((unsigned char *)add, is_64);
        ft_printf("%s", "\t");
        hexdump_data((unsigned char *)str, i, nb->size);
        ft_printf("\n");
        i = i  + plus;
        if (is_64)
            *(uint64_t *)add =  *(uint64_t *)add + 16;
        else
         *(uint32_t *)add =  *(uint32_t *)add + 16;
    }
    return (0);
}