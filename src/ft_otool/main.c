#include "../../include/ft_otool/nm_otool.h"

static int    otool(char *ptr, char *name)
{
    unsigned int    magic_number;
    int             is_fat;
    int             is_64;
    int             should_swap;

    if (!is_safe(ptr + 32)) {
        put_error(name, ": is not an object file\n");
        return (-1);
    }
    magic_number = *(uint32_t *)ptr;
    if (not_valid_file(magic_number)) {
        put_error(name, ": is not an object file\n");
        return (-1);
    }
    is_fat = is_fat_file(magic_number);
    is_64 = is_64_file(magic_number);
    should_swap = should_swap_file(magic_number);
    ft_printf("%s:\n", name);
    return (handle_otool(is_fat, is_64, should_swap, ptr));
}

static int     handle_each_arg(char *av)
{
    int     fd;
    char    *ptr;
    struct stat buf;

    if ((fd = get_fd(av)) < 0)
        return (EXIT_FAILURE);
    if (fstat(fd, &buf) < 0) {
        write(2, "fstat", 5);
        return (EXIT_FAILURE);
    }
    if ((ptr = mmap(0, buf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        write(2, "mmap", 5);
        return (EXIT_FAILURE);
    }
    g_map_size = buf.st_size;
    g_begin = ptr;
    if (otool(ptr, av) < 0)
        return (EXIT_FAILURE);
    if (munmap(ptr, buf.st_size) < 0) {
        write(2, "munmap", 6);
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

static int     handle_args(int ac, char **av)
{
    int     i;
    int     res;
    char    *str;

    i = 0;
    res = 0;
    if (!ac) {
        str = "ft_otool: At least one file must be specified.\n";
        write(2, str, ft_strlen(str));
    }
    while (i < ac) {
        res = res | handle_each_arg(av[i]);
        i++;
    }
    return res;
}

int             main(int ac, char **av)
{
    int     res;

    res = handle_args(ac - 1, av + 1);
    return res;
}
