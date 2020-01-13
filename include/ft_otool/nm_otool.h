
#ifndef NM_OTOOL_H
# define NM_OTOOL_H
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
# include "../../libft/libft.h"

#define IS_64 1
#define IS_NOT_64 0
#define SHOULD_SWAP 1
#define SHOULD_NOT_SWAP 0
#define MY_CPU_X86_64 1
#define MY_CPU_I386 2
#define MULTI_ARGS 1
#define ONE_ARG 0
#define FIRST_ARG 1
#define NOT_FIRST_ARG 0

uint32_t           g_map_size;
char             *g_begin;

typedef struct	s_lc
{
    unsigned int    number_cmds;
    unsigned long   offset_cmds;

}				t_lc;

typedef struct s_fat
{
    int     should_swap;
    int     is_64;
    int     found_64;
}               t_fat;

typedef    struct s_nm
{
    int         is_64;
    unsigned char        type;
    unsigned char        *add;
    char        *name;
    struct s_nm *next;
}               t_nm;

typedef struct s_helper
{
    unsigned int            i;
    unsigned int            count;
    unsigned int            added_count;
    unsigned int            sec_size;
    unsigned int            seg_size;
}               t_helper;

typedef struct s_nb
{
    uint64_t size;
    uint32_t offset;
}           t_nb;

/*
** swap_bytes.c
*/
uint64_t	swap_uint64(uint64_t n);
uint32_t	swap_uint32(uint32_t n);
void        swap_endiane(char *data, int byte);
void        swap_32(char *data);
void        swap_64(char *data);

/*
** swap_elements.c
*/
void        swap_nb_32(char *ptr, int nb);
void        swap_mach_header(char *ptr);
void        swap_loadcommand(char *ptr);
void        swap_symtab(char *ptr);
void        swap_nlist(char *ptr, int is_64);

/*
** swap_elements_2.c
*/
void    swap_segment_command(char *ptr, int is_64);
void    swap_arch(char *ptr, int is_64);
void    swap_section(char *ptr, int is_64);

/*
** is_safe.c
*/
int         is_safe(char *ptr);
int         is_lc_safe(char *ptr, int is_64);
int         is_segmant_safe(char *ptr, int is_64);
int     not_valid_header(unsigned int magic);
int    not_valid_file(unsigned int magic);

/*
** dump.c
*/
void    hexdump_address(unsigned char *add, int is_64);
void    hexdump_data(unsigned char *add, uint64_t i, uint64_t size);
void    put_error(char *s1, char *s2);
int     dump_section(char *add, int is_64, t_nb *nb, char *ptr);

/*
** question.c
*/
int    is_fat_file(unsigned int magic);
int     is_64_arch(unsigned int magic);
int    is_64_file(unsigned int magic);
int     is_hostarch(char *array, int should_swap_arch);
/*
** handle_otool.c
*/
int             handle_otool(int is_fat, int is_64, int should_swap, char *ptr);

/*
** handle_not_fat.c
*/
int     handle_not_fat(int is_64, int should_swap, char *ptr);

/*
** get.c
*/
int         get_lc(t_lc *lc_info, int is_64, int should_swap, char *ptr);
uint64_t    get_offsize_arch(char *array, int should_swap_arch, int is_64_arch);
int         get_fd(char *name);

/*
** helper.c
*/
char        *text_in_seg(char *lc, int is_64);
int         should_swap_arch(unsigned int magic);
int         should_swap_file(unsigned int magic);
int         handle_each_arch(char *array, char *ptr, t_fat *fat);
#endif