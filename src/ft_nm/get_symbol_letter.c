/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_symbol_letter.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:04:17 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 17:27:31 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

static char		*get_section(char *ptr, int s, int is_64, unsigned char i)
{
	struct load_command	*lc;
	t_lc				lc_info;
	t_helper			h;

	init_helper(&h, is_64);
	if (get_lc(&lc_info, is_64, SHOULD_NOT_SWAP, ptr) < 0)
		return (NULL);
	lc = (void *)ptr + lc_info.offset_cmds;
	while (h.i < lc_info.number_cmds && is_safe((char *)lc))
	{
		if (lc->cmd == LC_SEGMENT || lc->cmd == LC_SEGMENT_64)
		{
			if (!is_segmant_safe((char *)lc, is_64))
				return (NULL);
			if (s)
				swap_segment_command((char *)lc, is_64);
			h.add = get_add(is_64, lc);
			if (h.count + h.add >= i)
				return ((char *)lc + h.seg + (i - h.count - 1) * h.sec);
			h.count = h.count + h.add;
		}
		lc = (void *)lc + lc->cmdsize;
		h.i++;
	}
	return (NULL);
}

static char		symbol(char *ptr, char *a, int should_swap, int is_64)
{
	unsigned char	letter;
	unsigned int	index;
	char			*sec_name;
	char			*seg_name;
	char			*sections_add;

	index = is_64 ? ((struct nlist_64 *)a)->n_sect : \
	((struct nlist *)a)->n_sect;
	sections_add = get_section(ptr, should_swap, is_64, index);
	if (!sections_add)
		return ('s');
	if (should_swap)
		swap_section(sections_add, is_64);
	letter = 's';
	sec_name = is_64 ? ((struct section_64 *)sections_add)->sectname :\
	((struct section *)sections_add)->sectname;
	seg_name = is_64 ? ((struct section_64 *)sections_add)->segname :\
	((struct section *)sections_add)->segname;
	if (!ft_strcmp(sec_name, SECT_TEXT) && !ft_strcmp(seg_name, SEG_TEXT))
		letter = 't';
	else if (!ft_strcmp(sec_name, SECT_DATA) && !ft_strcmp(seg_name, SEG_DATA))
		letter = 'd';
	else if (!ft_strcmp(sec_name, SECT_BSS) && !ft_strcmp(seg_name, SEG_DATA))
		letter = 'b';
	return (letter);
}

static char		pre_symbol_letter(unsigned char n, int16_t n_desc, uint32_t va)
{
	unsigned char	type;
	unsigned char	type_field;

	type = '?';
	type_field = n & N_TYPE;
	if (N_STAB & n)
		type = '-';
	else if (type_field == N_UNDF)
		type = va ? 'c' : 'u';
	else if (type_field == N_ABS)
		type = 'a';
	else if (type_field == N_SECT)
		type = 's';
	else if (type_field == N_PBUD)
		type = 'u';
	else if (type_field == N_INDR)
		type = 'i';
	else if (n_desc & N_WEAK_REF)
		type = 'w';
	return (type);
}

static char		pre_64(unsigned char n_type, uint16_t n, uint64_t va)
{
	unsigned char	type;
	unsigned char	type_field;

	type = '?';
	type_field = n_type & N_TYPE;
	if (N_STAB & n_type)
		type = '-';
	else if (type_field == N_UNDF)
		type = va ? 'c' : 'u';
	else if (type_field == N_ABS)
		type = 'a';
	else if (type_field == N_SECT)
		type = 's';
	else if (type_field == N_PBUD)
		type = 'u';
	else if (type_field == N_INDR)
		type = 'i';
	else if (n & N_WEAK_REF)
		type = 'w';
	return (type);
}

char			get_symbol_letter(char *ptr, char *list, int s, int is_64)
{
	struct nlist	*l;
	struct nlist_64	*l_64;
	char			letter;

	l = (struct nlist *)list;
	l_64 = (struct nlist_64 *)list;
	if (is_64)
		letter = pre_64(l_64->n_type, l_64->n_desc, l_64->n_value);
	else
		letter = pre_symbol_letter(l->n_type, l->n_desc, l->n_value);
	if (letter == 's')
		letter = symbol(ptr, list, s, is_64);
	if (letter != '?' && (N_EXT & l->n_type))
		letter = letter - ('a' - 'A');
	return (letter);
}
