#include <stdio.h>
#include <sys/mman.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/nlist.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "../../include/ft_otool/nm_otool.h"

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

void    swap_endiane(char *data, int byte)
{
    char    temp;
    int     i;
    int     j;

    i = -1;
    j = byte;
    while (++i <= --j){
        temp = *(data + i);
        *(data + i) = *(data + j);
        *(data + j) = temp;
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
        swap_32(ptr + i * 4);
}

void    swap_mach_header(char *ptr)
{
    swap_32(ptr + 16);
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
    swap_endiane(ptr + 5, 2);
    if (is_64)
        swap_64(ptr + 7);
    else
        swap_nb_32(ptr + 7, 1);
}

int     is_pointer_safe(char *ptr)
{
    return (ptr - g_begin >= 0 && ptr - g_begin <= g_map_size);
}


int    is_lc_safe(char *ptr, int is_64) {
    char    *lc;

    if (is_64)
        lc = ptr + sizeof(struct mach_header);
    else
        lc = ptr + sizeof(struct mach_header_64);
    return (is_pointer_safe(lc));
}

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

// for the moment, only swap uint32_t nsects, adds more if needs more in the future
void            swap_segment_command(char *ptr, int is_64)
{
    if (is_64)
        swap_nb_32(ptr + 42 + 2 * sizeof(vm_prot_t), 1);
    else
        swap_nb_32(ptr + 26 + 2 * sizeof(vm_prot_t), 1);
}

void    swap_fat_header(char *ptr)
{
    swap_nb_32(ptr + 4, 1);
}

void   swap_fat_arch(char *ptr, int is_64_arch)
{
    if (is_64_arch)
        swap_64(ptr + sizeof(cpu_type_t) + sizeof(cpu_subtype_t));
    else
        swap_nb_32(ptr + 8, 1);
}

//only swaped for offset and size, swap more if needed in the future
void    swap_arch(char *ptr, int is_64)
{
    if (is_64) {
        swap_64(ptr + 8);
        swap_64(ptr + 16);
    }
    else {
        swap_nb_32(ptr + 8, 2);
    }
}

int     is_segmant_safe(char *ptr, int is_64)
{
    if (is_64)
        return is_pointer_safe(ptr + sizeof(struct segment_command_64));
    return is_pointer_safe(ptr + sizeof(struct segment_command));
}

void            swap_section(char *ptr, int is_64)
{
    if (is_64){
        swap_64(ptr + 4);
        swap_64(ptr + 12);
        swap_32(ptr + 20);
    }
    else
        swap_nb_32(ptr + 4, 3);
}

void  hexdump_address(unsigned char *add, int is_64)
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

void  hexdump_data(unsigned char *add, uint64_t i, uint64_t size)
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

// !!!!!!need to remove also in ft_nm

// void  put_add_letter(char *ptr, char *list, int should_swap, int is_64)
// {
//     char    letter;
//     char    *symbol_add;

//     symbol_add = NULL;
//     letter = get_symbol_letter(ptr, list, should_swap, is_64);
//     if (is_in_section(letter))
//         symbol_add = list + 8;
//     if (!symbol_add){
//         if (is_64)
//         ft_printf("%17s", "");
//         else
//         ft_printf("%9s", "");
//     }
//     else
//         hexdump_address((unsigned char *)symbol_add, is_64);
//     ft_printf(" %c ", letter);
// }

void    put_error(char *s1, char *s2)
{
    write(2, s1, ft_strlen(s1));
    write(2, s2, ft_strlen(s2));
}

char    *get_nlist(char *ptr, unsigned int i, int is_64, struct symtab_command   *sym)
{
    unsigned    long        size;

    size = is_64 ? sizeof(struct nlist_64) : sizeof(struct nlist);
    return (ptr + sym->symoff + i * size);
}

char    *text_in_seg(char *lc, int is_64)
{
    unsigned int    i;
    unsigned int    nb;
    unsigned int    sec_size;
    unsigned int    seg_size;
    char            *array;

    i = 0;
    nb = is_64 ? ((struct segment_command_64 *)lc)->nsects : ((struct segment_command *)lc)->nsects;
    sec_size = is_64 ? sizeof(struct section_64) : sizeof(struct section);
    seg_size = is_64 ? sizeof(struct segment_command_64) : sizeof(struct segment_command);
    while(i < nb) {
        array = lc + seg_size + i * sec_size;
        if (!is_pointer_safe(array))
            return (NULL);
        if (!ft_strcmp(array, SECT_TEXT) && !ft_strcmp(array + 16, SEG_TEXT))
            return (array);
        i++;
    }
    return (NULL);
}


char    *get_text_section(t_lc lc_info, char *ptr, int should_swap, int is_64)
{
    unsigned int            i;
    struct load_command     *lc;
    char                    *res;


    i = 0;
    lc = (void *)ptr + lc_info.offset_cmds;
    while (i < lc_info.number_cmds) {
        if (!is_pointer_safe((char *)lc))
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

//check all the fileoff and offset if it's correct.
// int    dump_section_64(char *add, int is_64, uint64_t size, uint32_t offset)
// {
//     uint64_t        i;
//     uint64_t        plus;
//     char            *ptr;


//     plus = 16;
//     i = 0;
//     while (i <= size){
//         ptr = g_begin + offset + i;
//         if (!is_pointer_safe(ptr))
//             return (-1);
//         hexdump_address((unsigned char *)add, is_64);
//         ft_printf("      ");
//         hexdump_data((unsigned char *)ptr, i, size);
//         ft_printf("\n");
//         i = i  + plus;
//         *(uint64_t *)add =  *(uint64_t *)add + 16;
//     }
//     printf("pppppppppppp\n");
//     return (0);
// }

// int    dump_section(char *add, int is_64, uint32_t size, uint32_t offset)
// {
//     uint32_t        i;
//     uint32_t        plus;
//     char            *ptr;


//     plus = 8;
//     i = 0;
//     while (i <= size){
//         ptr = g_begin + offset + i;
//         if (!is_pointer_safe(ptr))
//             return (-1);
//         hexdump_address((unsigned char *)add, is_64);
//         ft_printf("      ");
//         hexdump_data((unsigned char *)ptr, i, size);
//         ft_printf("\n");
//         i = i  + 16;
//          *(uint32_t *)add =  *(uint32_t *)add + 16;
//     }
//     return (0);
// }

int    dump_section(char *add, int is_64, t_nb *nb, char *ptr)
{
    uint64_t        i;
    uint64_t        plus;
    char            *str;


    plus = 16;
    i = 0;
    while (i < nb->size){
        str = ptr + nb->offset + i;
        if (!is_pointer_safe(str))
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

int    put_text_section(char *sec, int should_swap, int is_64, char *ptr)
{
    unsigned int    sec_size;
    t_nb            nb;

    sec_size = is_64 ? sizeof(struct section_64) : sizeof(struct section);
    if (!is_pointer_safe(sec + sec_size))
        return (-1);
    if (should_swap)
        swap_section(sec, is_64);
    nb.size = is_64 ? ((struct section_64 *)sec)->size : (uint64_t)((struct section *)sec)->size ;
    nb.offset = is_64 ? ((struct section_64 *)sec)->offset : ((struct section *)sec)->offset;
    ft_putstr("Contents of (__TEXT,__text) section\n");
    dump_section(sec + 32, is_64, &nb, ptr);
    return (0);
}

int    is_fat_file(unsigned int magic)
{
    return (magic == FAT_CIGAM || magic == FAT_MAGIC ||  magic == FAT_CIGAM_64 || magic == FAT_MAGIC_64);
}

int     is_64_arch(unsigned int magic)
{
    return (magic == FAT_CIGAM_64 || magic == FAT_MAGIC_64);
}

int    is_64_file(unsigned int magic)
{
    if (is_fat_file(magic))
        return is_64_arch(magic);
    return (magic == MH_CIGAM_64 || magic == MH_MAGIC_64);
}

int    should_swap_arch(unsigned int magic)
{
    return (magic == FAT_CIGAM || magic == FAT_CIGAM_64);
}

int    should_swap_file(unsigned int magic)
{
    if (is_fat_file(magic))
        return should_swap_arch(magic);
    return (magic == MH_CIGAM || magic == MH_CIGAM_64);
}

int    not_valid_header(unsigned int magic)
{
    return (magic != MH_CIGAM_64 && magic != MH_CIGAM
	&& magic != MH_MAGIC_64 && magic != MH_CIGAM_64
	&& magic != MH_MAGIC && magic != MH_CIGAM);
}

int    not_valid_file(unsigned int magic)
{
    return (magic != FAT_MAGIC && magic != FAT_MAGIC_64
	&& magic != FAT_CIGAM && magic != FAT_CIGAM_64
	&& magic != MH_CIGAM_64 && magic != MH_CIGAM
	&& magic != MH_MAGIC_64 && magic != MH_CIGAM_64
	&& magic != MH_MAGIC && magic != MH_CIGAM);
}


int     handle_not_fat(int is_64, int should_swap, char *ptr)
{
    unsigned int            ncmds;
    t_lc                    lc_info;
    char                    *section;

    if (!is_pointer_safe(ptr))
        return (-1);
    if (get_lc(&lc_info, is_64, should_swap, ptr) < 0)
        return (-1);
    ncmds = lc_info.number_cmds;
    section = get_text_section(lc_info, ptr, should_swap, is_64);
    put_text_section(section, should_swap, is_64, ptr);
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
        // offset_64 = (uint64_t )(offset << 32) & 0xFFFFFFFF00000000;
        offset_64 = (uint64_t )offset;
    }
    return offset_64;
}

int     is_hostarch(char *array, int should_swap_arch)
{
    cpu_type_t type;

    type = should_swap_arch ? swap_uint32(*(cpu_type_t *)array) : *(cpu_type_t *)array;
    if (type == CPU_TYPE_X86_64)
        return (MY_CPU_X86_64);
    if (type == CPU_TYPE_I386)
        return (MY_CPU_I386);
    return (0);
}

int     handle_each_arch(char *array, char *ptr, t_fat *fat)
{
    unsigned int    magic_number;
    int             is_64;
    int             should_swap;
    uint64_t        offset;

    if (!is_pointer_safe(array))
        return (-1);
    if (!is_hostarch(array, fat->should_swap))
        return (-1);
    if (fat->found_64 && is_hostarch(array, fat->should_swap) == MY_CPU_I386)
        return (-1);
    offset = get_offsize_arch(array, fat->should_swap, fat->is_64);
    // ft_printf("offset here=%u\n", offset);//double check here
    magic_number = *(uint32_t *)(ptr + offset);
    if (not_valid_header(magic_number)) {
       return (-1);
    }
    is_64 = is_64_file(magic_number);
    should_swap = should_swap_file(magic_number);
    if (is_hostarch(array, fat->should_swap) == MY_CPU_X86_64)
        fat->found_64 = 1;
    return handle_not_fat(is_64, should_swap, ptr + offset);
}


void    init_t_fat(t_fat *fat, int is_64, int should_swap, char *ptr)
{
    unsigned int     i;
    unsigned int     nb;
    unsigned long    size;

    i = 0;
    fat->found_64 = 0;
    fat->is_64 = is_64;
    fat->should_swap = should_swap;
    nb = should_swap ? swap_uint32(((struct fat_header *)ptr)->nfat_arch) : ((struct fat_header *)ptr)->nfat_arch;
    size = is_64 ? sizeof(struct fat_arch_64) : sizeof(struct fat_arch);
    while (i < nb) {
        if (is_hostarch(ptr + i * size + sizeof(struct fat_header), should_swap) == MY_CPU_X86_64){
            fat->found_64 = 1;
            break;
        }
        i++;
    }
}

int     handle_fat(int is_64, int should_swap, char *ptr)
{
    unsigned int    nb;
    unsigned int    i;
    t_fat           fat;
    unsigned long   size;

    if (!is_pointer_safe(ptr + sizeof(struct fat_header)))
        return (-1);
    nb = should_swap ? swap_uint32(((struct fat_header *)ptr)->nfat_arch) : ((struct fat_header *)ptr)->nfat_arch;
    // ft_printf("nb=%d\n", nb);
    i = 0;
    init_t_fat(&fat, is_64, should_swap, ptr);
    size = is_64 ? sizeof(struct fat_arch_64) : sizeof(struct fat_arch);
    while (i < nb) {
        handle_each_arch(ptr + i * size + sizeof(struct fat_header), ptr, &fat);
        i++;
    }
    return (0);
}

int     handle_otool(int is_fat, int is_64, int should_swap, char *ptr)
{
    if (is_fat)
        return handle_fat(is_64, should_swap, ptr);
    else
        return handle_not_fat(is_64, should_swap, ptr);
}

int    otool(char *ptr, char *name)
{
    unsigned int    magic_number;
    int             is_fat;
    int             is_64;
    int             should_swap;

    if (!is_pointer_safe(ptr + 32)) {
        put_error(name, ": is not an object file.\n");
        return (-1);
    }
    magic_number = *(uint32_t *)ptr;
    if (not_valid_file(magic_number)) {
        put_error(name, ": is not an object file.\n");
        return (-1);
    }
    is_fat = is_fat_file(magic_number);
    is_64 = is_64_file(magic_number);
    should_swap = should_swap_file(magic_number);
    ft_printf("%s:\n", name);
    return (handle_otool(is_fat, is_64, should_swap, ptr));
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

int     handle_each_arg(char *av)
{
    int     fd;
    char    *ptr;
    struct stat buf;

    if ((fd = get_fd(av)) < 0)
        return (EXIT_FAILURE);
    if (fstat(fd, &buf) < 0) {
        perror("fstat");
        return (EXIT_FAILURE);
    }
    if ((ptr = mmap(0, buf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        perror("mmap");
        return (EXIT_FAILURE);
    }
    g_map_size = buf.st_size;
    g_begin = ptr;
    if (otool(ptr, av) < 0)
        return (EXIT_FAILURE);
    if (munmap(ptr, buf.st_size) < 0) {
        perror("munmap");
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

int     handle_args(int ac, char **av)
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

int     main(int ac, char **av)
{
    int     res;

    res = handle_args(ac - 1, av + 1);
    return res;
}
