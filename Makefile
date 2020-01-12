###############################################################################
##                                                                           ##
##  This is a generalized makefiles made to be used on different kind        ##
##  of projects, such as making libraries , source files etc.                ##
##  Please note that to use this make files you need to posses the `.misc`   ##
##  directory that is included in the repo by default. This path is needed   ##
##  as some files used by make files are included there. Do change anything  ##
##  dependeing on your need.                                                 ##
##                                                                           ##
##  In the file `color` located in `.misc/make` you can find terminal escape ##
##  codes for colors arr or remove or eddid to get result as you want.       ##
##                                                                           ##
##                                                                           ##
##  In the file `path` located in `.misc/make` information about varius paths##
##  are included for to make this Makefile easier and to manage the make file##
##  More easily.                                                             ##
##                                                                           ##
###############################################################################


## Please do not remove the includes
## as they contain impoirtent information
## variables and rules

include .misc/make/color
include .misc/make/paths
include .misc/make/misc_var

## Te `.SILENT` launche evrything specified in
## silent mode so you dont have to put the `@`
.SILENT	: __START	NAME	clean fclean all re object library os_dep
.PHONY	: __START			clean fclean all re object library os_dep


## This is launched if no param given
.DEFAULT_GOAL = __START

## Project name (will be used)
PROJECT	=	ft_nm
PROJECT_OTOOL = 	ft_otool

## Project source path
P_SRC =	src/ft_nm
O_SRC = src/ft_otool

## Project include
P_INCLUDE = include\ft_nm
O_INCLUDE = include\ft_otool

## compiler related
CC		:=	clang ## default compiler is clang

CFLAGS	:=	-Werror \
			-Wall	\
			-Wextra

## some useful `flags` for memory verifications
##
## -O1 -g -fsanitize=address	\
## -fno-omit-frame-pointer		\
## -fsanitize-address-use-after-scope \

## If we don't want any compiler flags
ifdef NOCCFLAGS
	CFLAGS :=
endif

## If we want to debug then add the `SHARED=1` argument to make
ifdef DEBUG
	CFLAGS := $(CFLAGS)  -g
endif

## If we want to compile with sanitizer then add the `SHARED=1` argument to make
ifdef ASAN
	CFLAGS := $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope
endif

## binary, library etc...
MAIN	?=	$(P_SRC)/main.c
NAME	?=	$(PROJECT) 		## The name of your binary

MAIN_OTOOL	?=	$(O_SRC)/main.c
NAME_OTOOL	?=	$(PROJECT_OTOOL) 	

#The name of the library you want to make
LIB_A	?=	$(PROJECT).a

#All LIB_FT stufs
LIBFT		= libft
LIBFT_INC	= $(LIBFT)/.
LIBFT_LIB	= $(LIBFT)
LIBFT_A		= $(LIBFT_LIB)/libft.a $(LIBFT_LIB)/libftprintf.a
LIBFT_FLAGS = -I $(LIBFT_INC) -L $(LIBFT_LIB) -lft
LIBFT_MAKE_FLAGS =

## sources and objects where path names are removed.
## Add all your source files to this variable
SRC		=		$(MAIN)					\
				#Add other source files here...	\
				#$(P_SRC)/<yourfile>.c	\

SRC_OTOOL	=	$(MAIN_OTOOL)					\
				#Add other source files here...	\
				#$(P_SRC)/<yourfile>.c	\


## Objects without path names
OBJ			:=	$(addsuffix .o, $(basename $(SRC)))
OBJ_OTOOL	:=	$(addsuffix .o, $(basename $(SRC_OTOOL)))

## Objects with their path name
OBJ_P	=	$(addprefix $(P_OBJ)/,$(OBJ))	## addprefix add the
											## path name to the files...

## All header (.h) files so if they changed then all files will be recompiled
HEADERS =	$(P_INCLUDE)/nm_otool.h
O_HEADERS =	$(O_INCLUDE)/nm_otool.h

## Start making here
__START: all

## For multiple Binarys
all : $(NAME) $(NAME_OTOOL)

$(OBJ)		 : $(LIBFT_A)
$(OBJ_OTOOL) : $(LIBFT_A)

$(NAME): $(OBJ)
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIBFT_A)

$(NAME_OTOOL): $(OBJ_OTOOL)
	@$(CC) $(CFLAGS) $(OBJ_OTOOL) -o $(NAME_OTOOL) $(LIBFT_A)

## Compiles any object file that is added as dependency
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -I $(P_INCLUDE) -c -o $@ $<

%.o: %.c $(O_HEADERS)
	$(CC) $(CFLAGS) -I $(O_INCLUDE) -c -o $@ $<

#Default library related
$(LIBFT_A):
	make -C $(LIBFT)

## Clean objects and others
clean:
	make clean -C 	$(LIBFT)
	rm		-f	$(OBJ)
	rm		-f	$(OBJ_OTOOL)
	printf	"$(WARN)[!][$(PROJECT)] Removed all objects from ./$(P_SRC)\n"
	printf	"$(OK)[+][$(PROJECT)] Cleaned$(C_DEF)\n"
	printf	"$(WARN)[!][$(PROJECT_OTOOL)] Removed all objects from ./$(O_SRC)\n"
	printf	"$(OK)[+][$(PROJECT_OTOOL)] Cleaned$(C_DEF)\n"

## Cleans everything
fclean:		clean
	make fclean -C 	$(LIBFT)
	rm		-f	$(NAME)
	rm		-f	$(NAME_OTOOL)
	printf	"$(WARN)[!][$(PROJECT)] Removed all binary\n"
	printf	"$(OK)[+][$(PROJECT)] Fully cleaned.\n"
	printf	"$(WARN)[!][$(PROJECT_OTOOL)] Removed all binary\n"
	printf	"$(OK)[+][$(PROJECT_OTOOL)] Fully cleaned.\n"

re:			fclean all



## This rule is called when a difference in operating sistem has been
## detected. You can put your prerequisite to be changed if a different
## os has been detected
os_dep: #put your prerequisite for os dependent stufs
	## put your os dependent comands here
	## this will be launched if the os name is
	## different then what read from the os file.
	## ex: make re
	@printf "[$(PROJECT)] Os dependent stufs\n"

## Useful Makefile tricks
##
## ?= 			// let you put a default variable then later change it
## j<number>	// let you launche the number of job at the same time
## ifdef		// let you verify if used defined something or not
## .SILENT		// This parameter let you launch rules in silent mod
## .IGNORE		// Ignore parameter used as .SILENT
#
## --stop-on-faliur			// stop the program if error occures
## -k or --keep-going		// To keep ignoring all errors
## -i or --ignore-errors	// To Ignor error
## --no-print-directory		// This do not show the 'entered ... directory' warning
