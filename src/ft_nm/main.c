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

#include "../../include/ft_nm/nm_otool.h"

void    free_nm_list(t_nm *list)
{
    t_nm *temp;

    while(list) {
        temp = list;
        list = list->next;
        free(temp);
    }
}

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

// int    is_magic_safe(char *ptr)
// {

// }

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
    // lc_info->number_cmds = is_64 ? swap_uint32(header_64->ncmds) : swap_uint32(header->ncmds);
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

void    init_helper(t_helper *helper, int is_64)
{
    helper->i = 0;
    helper->count = 0;
    helper->sec_size = is_64 ? sizeof(struct section_64) : sizeof(struct section);
    helper->seg_size = is_64 ? sizeof(struct segment_command_64) : sizeof(struct segment_command);
}

int     is_segmant_safe(char *ptr, int is_64)
{
    if (is_64)
        return is_pointer_safe(ptr + sizeof(struct segment_command_64));
    return is_pointer_safe(ptr + sizeof(struct segment_command));
}

// index starts from 1
// here no need, no need to swap load_command, already swaped before
char   *get_section_by_index(char *ptr, int should_swap, int is_64, unsigned char index)
{
    struct load_command     *lc;
    t_lc                    lc_info;
    t_helper                h;

    init_helper(&h, is_64);
    if (get_lc(&lc_info, is_64, SHOULD_NOT_SWAP, ptr) < 0)
        return (NULL);
    lc = (void *)ptr + lc_info.offset_cmds;
    while ( h.i < lc_info.number_cmds) {
        if (!is_pointer_safe((char *)lc))
            break;
        if (lc->cmd == LC_SEGMENT || lc->cmd == LC_SEGMENT_64){
            if (!is_segmant_safe((char *)lc, is_64))
                return NULL;
            if (should_swap)
                swap_segment_command((char *)lc, is_64);
            h.added_count = (is_64 ? ((struct segment_command_64 *)lc)->nsects :  ((struct segment_command *)lc)->nsects);
            if (h.count + h.added_count >= index)
                return ((char *)lc + h.seg_size + (index - h.count - 1) * h.sec_size);
            h.count = h.count + h.added_count;
        }
        lc = (void *)lc + lc->cmdsize;
        h.i++;
    }
    return NULL;
}

void            swap_section(char *ptr, int is_64)
{
    swap_endiane(ptr, 2);
    swap_endiane(ptr + 16, 2);
    (void)is_64;
    // if (is_64)
    //     swap_64(ptr + 32);
    // else
    //     swap_32(ptr + 32);
    // keep on for that if needs to swap more foe future use
}
// header should already been swapped before, no need to do here


char        symbol_in_section(char *ptr, char *nlist_add, int should_swap, int is_64)
{
    unsigned char       letter;
    unsigned int        index;
    char                *sec_name;
    char                *seg_name;
    char                *sections_add;


    index = is_64 ? ((struct nlist_64 *)nlist_add)->n_sect : ((struct nlist *)nlist_add)->n_sect;
    sections_add = get_section_by_index(ptr, should_swap, is_64, index);
    if (!sections_add)
        return ('s');
    if (should_swap)
        swap_section(sections_add, is_64);
    sec_name = is_64 ? ((struct section_64 *)sections_add)->sectname : ((struct section *)sections_add)->sectname;
    seg_name = is_64 ? ((struct section_64 *)sections_add)->segname : ((struct section *)sections_add)->segname;
    if (!ft_strcmp(sec_name, SECT_TEXT) && !ft_strcmp(seg_name, SEG_TEXT))
        letter = 't';
    else if (!ft_strcmp(sec_name, SECT_DATA) && !ft_strcmp(seg_name, SEG_DATA))
        letter = 'd';
    else if (!ft_strcmp(sec_name, SECT_BSS) && !ft_strcmp(seg_name, SEG_DATA))
        letter = 'b';
    else
        letter = 's';
    return letter;
}

char    pre_symbol_letter(unsigned char n_type, int16_t n_desc, uint32_t n_value)
{
    unsigned char    type;
    unsigned char    type_field;

    type = '?';
    type_field = n_type & N_TYPE;
    if (N_STAB & n_type)
        type = '-';
    else if (type_field == N_UNDF)
        type = n_value ? 'c' : 'u';
    else if (type_field == N_ABS)
        type = 'a';
    // else if (type_field == N_SECT && n_sect == NO_SECT)
    //     type = '?';
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

char    pre_symbol_letter_64(unsigned char n_type, uint16_t n_desc, uint64_t n_value)
{
    unsigned char    type;
    unsigned char    type_field;

    type = '?';
    type_field = n_type & N_TYPE;
    if (N_STAB & n_type)
        type = '-';
    else if (type_field == N_UNDF)
        type = n_value ? 'c' : 'u';
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

char    get_symbol_letter(char *ptr, char *list, int should_swap, int is_64)
{
    struct nlist    *l;
    struct nlist_64 *l_64;
    char            letter;

    l = (struct nlist *)list;
    l_64 = (struct nlist_64 *)list;
    if (is_64)
        letter = pre_symbol_letter_64(l_64->n_type, l_64->n_desc, l_64->n_value);
    else
        letter = pre_symbol_letter(l->n_type, l->n_desc, l->n_value);
    if (letter == 's')
        letter = symbol_in_section(ptr, list, should_swap, is_64);
    //below in these bytes 64 and 32 no difference, can use either
    //if external make it uppercase
    if (letter != '?' && (N_EXT & l->n_type))
		letter = letter - ('a' - 'A');
    return letter;
}

int    is_in_section(unsigned char type)
{
    char letter;

    letter = ft_toupper(type);
    return (letter == 'T' || letter == 'D' || letter == 'B' || letter == 'S');
}

int     is_all_zero(unsigned char *add, int count)
{
    int i;

    i = 0;
    while (i < count) {
        if (add[i])
            return (0);
        i++;
    }
    return (1);
}

void  hexdump_address(unsigned char *add, int is_64)
{
    int     i;
    int     count;
    unsigned int   test;

    i = 0;
    count = is_64 ? 8 : 4;
    if (is_all_zero(add, count)) {
        if (is_64)
            ft_printf("%16s", "");
        else
            ft_printf("%8s", "");
    } else {
        while(--count >= 0) {
        test = add[count];
        ft_printf("%.2x", test);
        }
    }
}

void  put_add_letter(char *ptr, char *list, int should_swap, int is_64)
{
    char    letter;
    char    *symbol_add;

    symbol_add = NULL;
    letter = get_symbol_letter(ptr, list, should_swap, is_64);
    if (is_in_section(letter))
        symbol_add = list + 8;
    if (!symbol_add){
        if (is_64)
        ft_printf("%17s", "");
        else
        ft_printf("%9s", "");
    }
    else
        hexdump_address((unsigned char *)symbol_add, is_64);
    ft_printf("%c ", letter);
}

void    put_error(char *s1, char *s2)
{
    write(2, s1, ft_strlen(s1));
    write(2, s2, ft_strlen(s2));
}

int    is_extern(char c)
{
    if ( c >= 'A' && c <= 'Z')
        return (1);
    return (0);
}

int   can_show_list(t_nm *list, char *flags)
{
    if (list->type == '-')
        return (0);
    if (ft_strchr(flags, 'u') && ft_strchr(flags, 'U'))
        return (0);
    if (ft_strchr(flags, 'u')) {
        if (ft_strchr(flags, 'g'))
            return (list->type == 'U');
        return (list->type == 'u' || list->type == 'U');
    }
    if (ft_strchr(flags, 'U')) {
        if (ft_strchr(flags, 'g'))
            return (is_extern(list->type) && list->type != 'U');
        return (list->type != 'u' && list->type != 'U');
    }
    return (1);
}

void  print_new_line(int multi_arg, int firt_arg)
{
     if (!firt_arg && multi_arg)
        ft_printf("\n");
}

int   print_nm_list(t_nm *list, char *name, int multi_arg, char *flags)
{
    if (!list)
    {
        put_error(name, ": Is malformed file.\n");
        return (-1);
    }
    if (multi_arg)
        ft_printf("%s:\n", name);
    while (list) {
        if (can_show_list(list, flags)) {
            if (!ft_strchr(flags, 'u') && !ft_strchr(flags, 'j')) {
                hexdump_address(list->add, list->is_64);
                ft_printf(" %c ", list->type);
            }
            ft_printf("%s\n", list->name);
        }
        list = list->next;
    }
    return (0);
}

void   put_symbol_name(char *stringtable, char *list, int is_64)
{
    char    *name;
    int     index;

    index = is_64 ? (((struct nlist_64 *)list)->n_un).n_strx : (((struct nlist *)list)->n_un).n_strx;
    name = stringtable + index;
    ft_printf("%s\n", name);
}

// unsigned char    *get_addres(char *list)
// {
//     unsigned char    *symbol_add;

//     symbol_add = (unsigned char *)list + 8;
//     return symbol_add;
// }

char   *get_symbol_name(char *stringtable, char *list, int is_64)
{
    char    *name;
    int     index;

    index = is_64 ? (((struct nlist_64 *)list)->n_un).n_strx : (((struct nlist *)list)->n_un).n_strx;
    name = stringtable + index;
   return name;
}

char    *get_nlist(char *ptr, unsigned int i, int is_64, struct symtab_command   *sym)
{
    unsigned    long        size;

    size = is_64 ? sizeof(struct nlist_64) : sizeof(struct nlist);
    return (ptr + sym->symoff + i * size);
}

t_nm   *init_one_nm(char *ptr, char *list, int should_swap, int is_64)
{
    t_nm    *add;

    add = malloc(sizeof(t_nm));
    add->is_64 = is_64;
    add->type = get_symbol_letter(ptr, list, should_swap, is_64);
    add->add = (unsigned char *)list + 8;
    add->next = NULL;
    return (add);
}

t_nm    *add_list(t_nm *list, t_nm *add)
{
    t_nm *cp;

    if (!add)
        return list;
    if (!list)
        return add;
    cp = list;
    while(cp->next)
        cp = cp->next;
    cp->next = add;
    return list;
}

t_nm    *free_list_return_null(t_nm *list)
{
    free_nm_list(list);
    return NULL;
}

void    list_helper(t_nm **list, unsigned int *i, int should_swap, struct symtab_command   *sym)
{
    *list = NULL;
    if (should_swap)
        swap_symtab((char *)sym);
    *i = 0;
}
//    if (sym->stroff + sizeof(*array) * i < g_map_size && sym->symoff + sizeof(*array) * i < g_map_size ) //doule check this condition to be more strict
t_nm    *list_from_symtab(struct load_command *lc, char *ptr, int should_swap, int is_64)
{
    unsigned int           i;
    char                    *stringtable;
    struct symtab_command   *sym;
    t_nm                    *list;
    t_nm                    *add;

    sym = (struct symtab_command *)lc;
    if (!is_pointer_safe((char *)lc + sizeof(*sym)))
        return (NULL);
    list_helper(&list, &i, should_swap, sym);
    stringtable = (void *)ptr + sym->stroff;
    while (i < sym->nsyms) {
        if (!is_pointer_safe(stringtable) || !is_pointer_safe(get_nlist(ptr, i, is_64, sym)))
            return (free_list_return_null(list));
        if (should_swap)
            swap_nlist(get_nlist(ptr, i, is_64, sym), is_64);
        add = init_one_nm(ptr, get_nlist(ptr, i, is_64, sym), should_swap, is_64);
        add->name = get_symbol_name(stringtable, get_nlist(ptr, i, is_64, sym), is_64);
        if (!is_pointer_safe(add->name))
            return (free_list_return_null(list));
        list = add_list(list, add);
        i++;
    }
    return list;
}

t_nm    *get_nm_list(t_lc lc_info, char *ptr, int should_swap, int is_64)
{
    unsigned int            i;
    struct load_command     *lc;


    i = 0;
    lc = (void *)ptr + lc_info.offset_cmds;
    while (i < lc_info.number_cmds) {
        if (!is_pointer_safe((char *)lc + sizeof(*lc)))
            break;
        if (should_swap){
            swap_loadcommand((char *)lc);
        }
        if (lc->cmd == LC_SYMTAB) {
            return  list_from_symtab(lc, ptr, should_swap, is_64);
        }
        lc = (struct load_command *)((char *)lc + lc->cmdsize);
        i++;
    }
    return NULL;
}

int     compare_add(t_nm *cp, t_nm *jump)
{
    if (cp->is_64)
        return (*(uint64_t *)cp->add - *(uint64_t *)jump->add);
    else
        return (*(uint32_t *)cp->add - *(uint32_t *)jump->add);
}

void    compare_exchange(t_nm *cp, t_nm *jump)
{
    int         is_64_temp;
    unsigned char        type_temp;
    unsigned char        *add_temp;
    char        *name_temp;

    if (ft_strcmp(cp->name, jump->name) > 0 || (!ft_strcmp(cp->name, jump->name) && compare_add(cp, jump) > 0)) {
        is_64_temp = cp->is_64;
        type_temp = cp->type;
        add_temp = cp->add;
        name_temp = cp->name;
        cp->is_64 = jump->is_64;
        cp->type = jump->type;
        cp->add = jump->add;
        cp->name = jump->name;
        jump->is_64 = is_64_temp;
        jump->type = type_temp;
        jump->add = add_temp;
        jump->name = name_temp;
    }
}

void    compare_exchange_reverse(t_nm *cp, t_nm *jump)
{
    int         is_64_temp;
    unsigned char        type_temp;
    unsigned char        *add_temp;
    char        *name_temp;

    if (ft_strcmp(cp->name, jump->name) < 0 || (!ft_strcmp(cp->name, jump->name) && compare_add(cp, jump) < 0)) {
        is_64_temp = cp->is_64;
        type_temp = cp->type;
        add_temp = cp->add;
        name_temp = cp->name;
        cp->is_64 = jump->is_64;
        cp->type = jump->type;
        cp->add = jump->add;
        cp->name = jump->name;
        jump->is_64 = is_64_temp;
        jump->type = type_temp;
        jump->add = add_temp;
        jump->name = name_temp;
    }
}

void    sort_list_by_name(t_nm *list, char *flags)
{
    t_nm *cp;
    t_nm *jump;

    cp = list;
    if (list) {
    while (cp->next) {
        jump = cp->next;
        while (jump) {
            if (ft_strchr(flags, 'r'))
                compare_exchange_reverse(cp, jump);
            else
                compare_exchange(cp, jump);
            jump = jump->next;
        }
        cp = cp->next;
    }
    }
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


t_nm    *handle_not_fat(int is_64, int should_swap, char *ptr)
{
    // unsigned int                     ncmds;
    // struct load_command     *lc;
    t_lc                    lc_info;

    if (!is_pointer_safe(ptr))
        return (NULL);
    if (get_lc(&lc_info, is_64, should_swap, ptr) < 0)
        return (NULL);
    // ncmds = lc_info.number_cmds;
// ft_printf("inside handle_not_fat  should_swap=%d ncmds=%d\n", should_swap, ncmds);
    // lc = (void *)ptr + lc_info.offset_cmds;
    return get_nm_list(lc_info, ptr, should_swap, is_64);
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

t_nm    *handle_each_arch(char *array, char *ptr, t_fat *fat)
{
    unsigned int    magic_number;
    int             is_64;
    int             should_swap;
    uint64_t        offset;

    if (!is_pointer_safe(array))
        return (NULL);
    if (!is_hostarch(array, fat->should_swap))
        return NULL;
    if (fat->found_64 && is_hostarch(array, fat->should_swap) == MY_CPU_I386)
        return NULL;
    offset = get_offsize_arch(array, fat->should_swap, fat->is_64);
    // ft_printf("offset here=%u\n", offset);//double check here
    magic_number = *(uint32_t *)(ptr + offset);
    if (not_valid_header(magic_number)) {
       return NULL;
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

// no need is_64 for parameter , check later on
t_nm    *handle_fat(int is_64, int should_swap, char *ptr)
{
    unsigned int    nb;
    unsigned int    i;
    t_fat           fat;
    t_nm            *nm_list;
    unsigned long   size;

    if (!is_pointer_safe(ptr + sizeof(struct fat_header)))
        return (NULL);
    nb = should_swap ? swap_uint32(((struct fat_header *)ptr)->nfat_arch) : ((struct fat_header *)ptr)->nfat_arch;
    // ft_printf("nb=%d\n", nb);
    nm_list = NULL;
    i = 0;
    init_t_fat(&fat, is_64, should_swap, ptr);
    size = is_64 ? sizeof(struct fat_arch_64) : sizeof(struct fat_arch);
    while (i < nb) {
        nm_list = handle_each_arch(ptr + i * size + sizeof(struct fat_header), ptr, &fat);
        // nm_list = add_list(nm_list, handle_each_arch(ptr + i * size + sizeof(struct fat_header), ptr, &fat));
        if (nm_list)
            return nm_list;
        i++;
    }
    return nm_list;
}

t_nm    *handle_nm(int is_fat, int is_64, int should_swap, char *ptr)
{
    if (is_fat)
        return handle_fat(is_64, should_swap, ptr);
    else
    {

        return handle_not_fat(is_64, should_swap, ptr);
    }
}

int    sort_nm_list(t_nm *list, char *flags)
{
    if (ft_strchr(flags, 'p'))
        return (0);
    if (list)
        sort_list_by_name(list, flags);
    return (0);
}

int    nm(char *ptr, char *name, int multi_arg, int first_arg, char *flags)
{
    unsigned int    magic_number;
    int             is_fat;
    int             is_64;
    int             should_swap;
    t_nm            *nm_list;

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
    nm_list = handle_nm(is_fat, is_64, should_swap, ptr);
    sort_nm_list(nm_list, flags);

    print_new_line(multi_arg, first_arg);
    if (print_nm_list(nm_list, name, multi_arg, flags) < 0)
        return (-1);
    free_nm_list(nm_list);
    return (0);
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

int     handle_each_arg(char *av, int multi_arg, int first_arg, char *flags)
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
    if (nm(ptr, av, multi_arg, first_arg, flags) < 0)
        return (EXIT_FAILURE);
    if (munmap(ptr, buf.st_size) < 0) {
        perror("munmap");
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

int     one_arg(int ac, char *av, char *flags)
{
    if (!ac)
        av = "a.out";
    return (handle_each_arg(av, ONE_ARG, FIRST_ARG, flags));
}

int     multi_args(int ac, char **av, char *flags)
{
    int     i;
    int     res;

    i = 0;
    res = 0;
    while (i < ac) {
        if (!i)
            res = res | handle_each_arg(av[i], MULTI_ARGS, FIRST_ARG, flags);
        else
            res = res | handle_each_arg(av[i], MULTI_ARGS, NOT_FIRST_ARG, flags);
        i++;
    }
    return res;
}

int     is_flag(char c)
{
    return (c == 'g' || c == 'p' || c == 'r' || c == 'u' || c == 'U' || c == 'j');
}

int	    put_usage(char **av)
{
    char  *str1;
    char  *str2;

    str1 = "ft_nm: illegal option -- ";
    str2 = "usage: ft_nm [-gpruUj] [file ...]\n";
    write(2, str1, ft_strlen(str1));
    write(2, *av, 1);
    write(2, "\n", 1);
    write(2, str2, ft_strlen(str2));
	return (-1);
}

int		set_flags_nm(char *flags, char **av)
{
	int		ct;
	int		i;

	ct = 1;
	i = 0;
	av++;
	ft_bzero(flags, 10);
	while (*av && (*av)[0] == '-' && (*av)[1])
	{
		(*av)++;
		while (**av)
		{
			if (is_flag(**av))
			{
				if(!ft_strchr(flags, **av))
					flags[i++] = *((*av));
                (*av)++;
			}
			else
				return (put_usage(av));
		}
		(av)++;
		ct++;
	}
	return (ct);
}

int     main(int ac, char **av)
{
    int     res;
    char    flags[10];
    int     forward;


    forward = set_flags_nm(flags, av);
    if (forward < 0)
        return (EXIT_FAILURE);
    if ( ac - forward <= 1)
        res = one_arg(ac - forward, *(av + forward), flags);
    else
        res = multi_args(ac - forward, av + forward, flags);
    return res;
}
