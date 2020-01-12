#ifndef NM_OTOOL_H
# define NM_OTOOL_H
# include "../libft/libft.h"

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

type struct s_nb
{
    uint64_t size;
    uint32_t offset;
}           t_nb;

#endif