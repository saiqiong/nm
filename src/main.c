#include <stdio.h>
#include <sys/mman.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/nlist.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../include/nm_otool.h"

void    swap_endiane(char *data, unsigned long byte)
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

void   swap_loadcommand(char *ptr)
{
    swap_nb_32(ptr, 2);
}

void    swap_symtab(char *ptr)
{
     swap_nb_32(ptr + 8, 4); //the first 8 bytes, with swap loadcommand already swaped
}

void   swap_nlist(char *ptr, int is_64)
{
    swap_nb_32(ptr, 1);
    swap_endiane(ptr + 6, 2);
    if (is_64)
        swap_64(ptr + 8);
    else
        swap_nb_32(ptr + 8, 1);
}

// header should already been swapped before, no need to do here
unsigned long   get_sections_offset(char *ptr, int is_64)
{
    char    *ptr;



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


// load_command for 32 and 64 is same structure
// symtab_command for 32 and 64 is same structure
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

void    get_lc(t_lc *lc_info, int is_64, int should_swap, char *ptr)
{
    struct mach_header      *header;
    struct mach_header_64   *header_64;

    header = (struct mach_header *)ptr;
    header_64 = (struct mach_header_64 *)ptr;
    if (should_swap)
        swap_mach_header(ptr, is_64);
    lc_info->number_cmds = is_64 ? header_64->ncmds : header->ncmds;
    lc_info->offset_cmds = is_64? sizeof(*header_64) : sizeof(*header);
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

unsigned long   get_segment_setoff(char *ptr, int should_swap, int is_64)
{

    int                     i;
    struct load_command     *lc;
    unsigned long           nb;
    t_lc                    lc_info;
    

    get_lc(&lc_info, is_64, should_swap, ptr);
    i = -1;
    nb = lc_info.offset_cmds;
    lc = (void *)ptr + lc_info.offset_cmds;
    while (nb < g_map_size && ++i < lc_info.number_cmds) {
        if (lc->cmd == LC_SEGMENT || lc->cmd == LC_SEGMENT_64) {
           return nb;
        }
        lc = (void *)lc + lc->cmdsize;
        nb = nb + lc->cmdsize;
    }
}

char    get_symbol_letter(unsigned char n_type, unsigned char n_sect, uint16_t n_desc, uint32_t n_value)
{
    unsigned char    type;
    unsigned char    type_field;

    type = '?';
    type_field = n_type & N_TYPE;
    if (N_STAB & n_type)
        type = '-';
    else if (type_field == N_UNDF)
        type_field = n_value ? 'c' : 'u';
    else if (type == N_ABS)
        type_field = 'a';
    else if (type == N_SECT && n_sect == NO_SECT)
        type_field = '?';
    else if (type == NO_SECT)
        type_field = 'k';
    else if (type == N_PBUD)
		type_field = 'u';
	else if (type == N_INDR)
		type_field = 'i';
	else if (n_desc & N_WEAK_REF)
		type_field = 'W';

	//if external set uppercase
	if (type != '?' && N_EXT & n_type)
		type = type - ('a' - 'A');
    return (type);
}

void   put_symbol(char *stringtable, struct nlist *array, int should_swap)
{
    char    letter;

    if (should_swap)
        swap_nlist((char *)array, IS_NOT_64);
}

void   put_symbol_64(char *stringtable, struct nlist_64 *array, int should_swap)
{

}

//    if (sym->stroff + sizeof(*array) * i < g_map_size && sym->symoff + sizeof(*array) * i < g_map_size ) //doule check this condition to be more strict
void    put_output(struct load_command *lc, char *ptr, int should_swap, int is_64)
{
    int                     i;
    char                    *stringtable;
    struct nlist            *array;
    struct nlist_64         *array_64;
    struct symtab_command   *sym;

    sym = (struct symtab_command *)lc;
    if (should_swap)
        swap_symtab((char *)sym);
    array = (void *)ptr + sym->symoff;
    array_64 = (void *)ptr + sym->symoff;
    stringtable = (void *)ptr + sym->stroff;
    i = -1;
    while (++i < sym->nsyms) {
        if (is_64)
            put_symbol_64(stringtable, array_64 + i, should_swap);
        else
            put_symbol(stringtable, array_64 + i, should_swap);
        
        printf("%s\n", stringtable + array[i].n_un.n_strx);
    }
}

void    find_symtab(t_lc lc_info, char *ptr, int should_swap, int is_64)
{
    int                     i;
    struct load_command     *lc;
    unsigned long           size;
    

    i = -1;
    size = lc_info.offset_cmds;
    lc = (void *)ptr + lc_info.offset_cmds;
    while (size < g_map_size && ++i < lc_info.number_cmds) {
        if (should_swap)
            swap_loadcommand((char *)lc);
        if (lc->cmd == LC_SYMTAB) {
            put_output(lc, ptr, should_swap, is_64);
            break;
        }
        lc = (void *)lc + lc->cmdsize;
        size = size + lc->cmdsize;
    }
}

void    handle_not_fat(int is_64, int should_swap, char *ptr)
{
    int                     ncmds;
    struct load_command     *lc;
    t_lc                    lc_info;

    get_lc(&lc_info, is_64, should_swap, ptr);
    ncmds = lc_info.number_cmds;
    lc = (void *)ptr + lc_info.offset_cmds;
    find_symtab(lc_info, ptr, should_swap, is_64);
}

void    handle_nm(int is_fat, int is_64, int should_swap, char *ptr)
{
    // if (is_fat)
    //     handle_fat(is_64, should_swap, ptr);
    // else
        handle_not_fat(is_64, should_swap, ptr);
        is_fat++;
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