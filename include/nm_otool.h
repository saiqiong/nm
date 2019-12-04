#ifndef NM_OTOOL_H
# define NM_OTOOL_H
# include "../libft/libft.h"

#define IS_64 1
#define IS_NOT_64 0

unsigned long    g_map_size;

typedef struct	s_lc
{
    int             number_cmds;
    unsigned long   offset_cmds;

}				t_lc;

#endif