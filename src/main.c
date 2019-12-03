#include <stdio.h>
#include <sys/mman.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/nlist.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../include/nm_otool.h"

void    swap_endiane(char *data, int byte)
{
    char    temp;
    int     i;
    int     j;

    i = -1;
    j = byte;
    while (byte < g_map_size - 1 && ++i < --j){
        temp = data[i];
        data[i] = data[j];
        data[j] = temp;
    }
}

void    swap_32(char *data)
{
    swap_endiane(data, 4);
}

void    swap_64(char *data)
{
    swap_endiane(data, 8);
}

void    swap_nb_32(char *ptr, int nb)
{
     int     i;

    i = -1;
    while (++i < nb)
        swap_32(ptr + i);
}

void    swap_mach_header(char *ptr, int is_64)
{
    swap_nb_32(ptr, 6);
    if (is_64)
        swap_32(ptr + 7);
}

void    swap_symtab(char *ptr)
{
     swap_nb_32(ptr, 6);
}

void    print_output(int nsyms, int symoff, int stroff, char *ptr)
{
    int             i;
    char            *stringtable;
    struct nlist    *array;
    
    array = (void *)ptr + symoff;
    stringtable = (void *)ptr + stroff;
    i = -1;
    while (++i < nsyms) {
        if (stroff + sizeof(*array) * i < g_map_size && symoff + sizeof(*array) * i < g_map_size ) //doule check this condition to be more strict
            printf("%s\n", stringtable + array[i].n_un.n_strx);
        else
            continue;
    }
}

void    print_output_64(int nsyms, int symoff, int stroff, char *ptr)
{
    int             i;
    char            *stringtable;
    struct nlist_64 *array;
    
    array = (void *)ptr + symoff;
    stringtable = (void *)ptr + stroff;
    i = -1;
    while (++i < nsyms) {
        if (stroff + sizeof(*array) * i < g_map_size && symoff + sizeof(*array) * i < g_map_size ) //doule check this condition to be more strict
            printf("%s\n", stringtable + array[i].n_un.n_strx);
        else
            continue;
    }
}

void    print_output_swap(int nsyms, int symoff, int stroff, char *ptr)
{
    int             i;
    char            *stringtable;
    struct nlist_64 *array;
    

    array = (void *)ptr + symoff;
    stringtable = (void *)ptr + stroff;
    i = -1;
    while (++i < nsyms) {
        printf("%s\n", stringtable + array[i].n_un.n_strx);
    }
}

void    print_output_swap_64(int nsyms, int symoff, int stroff, char *ptr)
{
    int             i;
    char            *stringtable;
    struct nlist_64 *array;
    

    array = (void *)ptr + symoff;
    stringtable = (void *)ptr + stroff;
    i = -1;
    while (++i < nsyms) {
        printf("%s\n", stringtable + array[i].n_un.n_strx);
    }
}

void    print_symtab(int ncmds, struct load_command *lc, char *ptr, int should_swap)
{
    int                     i;
    struct symtab_command   *sym;
    int                     size;
    

    i = -1;
    size = sizeof(struct mach_header_64);
    while (size < g_map_size && ++i < ncmds) {
        if (lc->cmd == LC_SYMTAB) {
            sym = (struct symtab_command *)lc;
            print_output(sym->nsyms, sym->symoff, sym->stroff, ptr);
            break;
        }
        lc = (void *)lc + lc->cmdsize;
        size = size + lc->cmdsize;
    }
}

void    handle_64(int should_swap, char *ptr)
{
    int                     ncmds;
    struct mach_header_64   *header;
    struct load_command     *lc;

    header = (struct mach_header_64 *)ptr;
    if (should_swap)
        swap_mach_header(ptr, IS_64);
    ncmds = header->ncmds;
    lc = (void *)ptr + sizeof(*header);
    print_symtab(ncmds, lc, ptr, should_swap);
}

void    handle_32(int should_swap, char *ptr)
{
    int                     ncmds;
    struct mach_header   *header;
    struct load_command     *lc;

    header = (struct mach_header *)ptr;
    if (should_swap)
        swap_mach_header(ptr, IS_NOT_64);
    ncmds = header->ncmds;
    lc = (void *)ptr + sizeof(*header);
    print_symtab(ncmds, lc, ptr, should_swap);
}

void    handle_not_fat(int is_64, int should_swap, char *ptr)
{
    if (is_64)
        handle_64(should_swap, ptr);
    else
        handle_32(should_swap, ptr);
}

void    handle_nm(int is_fat, int is_64, int should_swap, char *ptr)
{
    if (is_fat)
        handle_fat(is_64, should_swap, ptr);
    else
        handle_not_fat(is_64, should_swap, ptr);
}

void    nm(char *ptr)
{
    unsigned int    magic_number;
    int             is_fat;
    int             is_64;
    int             should_swap;

    magic_number = *(int *)ptr;
    is_fat = (magic_number == FAT_CIGAM || magic_number == FAT_MAGIC || magic_number == FAT_CIGAM_64 || magic_number == FAT_MAGIC_64) ? 1 : 0;
    is_64 = (magic_number == FAT_CIGAM_64 || magic_number == FAT_MAGIC_64 || magic_number == MH_CIGAM_64 || magic_number == MH_MAGIC_64) ? 1 : 0;
    should_swap = (magic_number == FAT_CIGAM || magic_number == FAT_CIGAM_64 || magic_number == MH_CIGAM || magic_number == MH_CIGAM_64) ? 1 : 0;
    handle_nm(is_fat, is_64, should_swap, ptr);
}

int     main(int ac, char **av)
{
    int     fd;
    char    *ptr;
    struct stat buf;

    if (ac !=2) {
        fprintf(stderr, "Please give me an arg\n");
        return (EXIT_FAILURE);
    }
    if ((fd = open(av[1], O_RDONLY)) < 0){
        perror("open");
        return (EXIT_FAILURE);
    }
    if (fstat(fd, &buf) < 0) {
        perror("fstat");
        return (EXIT_FAILURE);
    }
    if ((ptr = mmap(0, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        perror("mmap");
        return (EXIT_FAILURE);
    }
    nm(ptr);
    if (munmap(ptr, buf.st_size) < 0) {
        perror("munmap");
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}