/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saxiao <saxiao@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/13 15:07:05 by saxiao            #+#    #+#             */
/*   Updated: 2020/01/13 16:52:16 by saxiao           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/ft_nm/nm_otool.h"

int		handle_each_arg(char *av, int multi_arg, int first_arg, char *flags)
{
	int			fd;
	char		*ptr;
	struct stat	buf;
	t_nu		num;

	if ((fd = get_fd(av)) < 0)
		return (EXIT_FAILURE);
	if (fstat(fd, &buf) < 0)
		return (return_error("fstat"));
	if ((ptr = mmap(0, buf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,\
	fd, 0)) == MAP_FAILED)
		return (return_error("mmap"));
	g_map_size = buf.st_size;
	g_begin = ptr;
	num.first = first_arg;
	num.multi = multi_arg;
	if (nm(ptr, av, &num, flags) < 0)
		return (EXIT_FAILURE);
	if (munmap(ptr, buf.st_size) < 0)
		return (return_error("munmap"));
	return (EXIT_SUCCESS);
}

int		one_arg(int ac, char *av, char *flags)
{
	if (!ac)
		av = "a.out";
	return (handle_each_arg(av, ONE_ARG, FIRST_ARG, flags));
}

int		multi_args(int ac, char **av, char *flags)
{
	int	i;
	int	res;

	i = 0;
	res = 0;
	while (i < ac)
	{
		if (!i)
			res = res | handle_each_arg(av[i], MULTI_ARGS, FIRST_ARG, flags);
		else
			res = res | handle_each_arg(av[i], MULTI_ARGS,\
			NOT_FIRST_ARG, flags);
		i++;
	}
	return (res);
}

int		main(int ac, char **av)
{
	int		res;
	char	flags[10];
	int		forward;

	forward = set_flags_nm(flags, av);
	if (forward < 0)
		return (EXIT_FAILURE);
	if (ac - forward <= 1)
		res = one_arg(ac - forward, *(av + forward), flags);
	else
		res = multi_args(ac - forward, av + forward, flags);
	return (res);
}
