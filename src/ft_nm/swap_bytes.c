/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_bytes.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:08:13 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 17:08:34 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

uint64_t	swap_uint64(uint64_t n)
{
	n = ((n << 8) & 0xFF00FF00FF00FF00ULL) \
		| ((n >> 8) & 0x00FF00FF00FF00FFULL);
	n = ((n << 16) & 0xFFFF0000FFFF0000ULL) \
		| ((n >> 16) & 0x0000FFFF0000FFFFULL);
	return ((n << 32) | (n >> 32));
}

uint32_t	swap_uint32(uint32_t n)
{
	n = ((n << 8) & 0xFF00FF00) | ((n >> 8) & 0xFF00FF);
	return ((n << 16) | (n >> 16));
}

void		swap_endiane(char *data, int byte)
{
	char	temp;
	int		i;
	int		j;

	i = -1;
	j = byte;
	while (++i <= --j)
	{
		temp = *(data + i);
		*(data + i) = *(data + j);
		*(data + j) = temp;
	}
}

void		swap_32(char *data)
{
	swap_endiane(data, 4);
}

void		swap_64(char *data)
{
	swap_endiane(data, 8);
}
