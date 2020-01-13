/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helper.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:06:52 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:25:30 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

void	init_helper(t_helper *helper, int is_64)
{
	helper->i = 0;
	helper->count = 0;
	helper->sec = is_64 ? sizeof(struct section_64) : sizeof(struct section);
	helper->seg = is_64 ? sizeof(struct segment_command_64) :\
	sizeof(struct segment_command);
}

int		is_in_section(unsigned char type)
{
	char letter;

	letter = ft_toupper(type);
	return (letter == 'T' || letter == 'D' || letter == 'B' || letter == 'S');
}

int		is_all_zero(unsigned char *add, int count)
{
	int i;

	i = 0;
	while (i < count)
	{
		if (add[i])
			return (0);
		i++;
	}
	return (1);
}

void	print_new_line(int multi_arg, int firt_arg)
{
	if (!firt_arg && multi_arg)
		ft_printf("\n");
}

t_nm	*init_one_nm(char *ptr, char *list, int should_swap, int is_64)
{
	t_nm	*add;

	add = malloc(sizeof(t_nm));
	add->is_64 = is_64;
	add->type = get_symbol_letter(ptr, list, should_swap, is_64);
	add->add = (unsigned char *)list + 8;
	add->next = NULL;
	return (add);
}
