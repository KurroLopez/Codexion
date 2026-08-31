# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/08/31 18:53:02 by fralopez          #+#    #+#              #
#    Updated: 2026/08/31 18:56:27 by fralopez         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# --------------#
#	Variables	#
# --------------#

NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

OBJ_DIR		= obj
SRC_DIR		= src
INCLUDE		= -I src

SRCS		= main.c \
			utils/args.c \
			utils/init.c \
			utils/utils.c \
			heap/heap.c \
			heap/heap_utils.c \
			dongle/dongle.c \
			dongle/dongle_acquire.c \
			simulation/simulation_state.c \
			simulation/simulation_cycle.c \
			simulation/simulation_monitor.c \
			simulation/simulation_run.c

OBJS		 = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

# --------------#
#	  Colors	#
# --------------#

BLUE		=		\033[96m
MAGENTA		=		\033[35m
GREEN		=		\033[32m
CYAN		=		\033[36m
YELLOW		=		\033[93m
BOLD		=		\033[1m
BGREEN		=		\033[92m
BRED		=		\033[91m
BMAGENTA	=		\033[95m
UNDERLINE	=		\033[4m
ITALIC		=		\033[3m
RESET		=		\033[0m

# --------------#
#	  TEXT		#
# --------------#

BOX_LINE		=	$(MAGENTA)━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$(RESET)
BOX_ROD_TOP_L	=	$(MAGENTA)┏$(RESET)
BOX_ROD_BOTT_L	=	$(MAGENTA)┗$(RESET)
BOX_ROD_TOP_R	=	$(MAGENTA)┓$(RESET)
BOX_ROD_BOTT_R	=	$(MAGENTA)┛$(RESET)
BOX_ROD_MIDDLE	=	$(MAGENTA)┃$(RESET)

LINE			=	⮑

USAGE			=	$(BMAGENTA)$(BOLD) Usage:\t\t\t\t\t\t\t\t\t\t   $(RESET)
ARGUMENTS_LINE1	=	$(CYAN)$(BOLD) ./$(NAME)$(ITALIC) <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug>$(RESET)
ARGUMENTS_LINE2	=	$(CYAN)$(BOLD)$(ITALIC)     <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>$(RESET)

SUCCESS_COMPIL	=	$(BGREEN)Success! $(RESET)$(BGREEN)Compiling $(NAME)...
COMPILED		=	$(GREEN)$(BOLD)[Compiled] ✅ $(RESET)

CLEAN_OBJ		=	$(YELLOW)$(BOLD)$(OBJ_DIR)$(RESET)$(YELLOW) folder have been deleted 🗑️$(RESET)
CLEAN_EXEC		=	$(YELLOW)$(BOLD)$(NAME)$(RESET)$(YELLOW) executable have been cleaned 🗑️$(RESET)

HEADER		= codexion.h

# --------------#
#	  RULES		#
# --------------#

all: $(NAME)

$(NAME): $(OBJS)
	@echo "\n$(LINE) $(SUCCESS_COMPIL)$(RESET)"
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(BOX_ROD_TOP_L)$(BOX_LINE)$(BOX_ROD_TOP_R)"
	@echo "$(BOX_ROD_MIDDLE)\t\t\t\t\t\t\t\t\t\t\t   $(BOX_ROD_MIDDLE)\n$(BOX_ROD_MIDDLE)$(USAGE)$(BOX_ROD_MIDDLE)\n$(BOX_ROD_MIDDLE)$(ARGUMENTS_LINE1)\t   $(BOX_ROD_MIDDLE)\n$(BOX_ROD_MIDDLE)\t$(ARGUMENTS_LINE2)$(BOX_ROD_MIDDLE)"
	@echo "$(BOX_ROD_BOTT_L)$(BOX_LINE)$(BOX_ROD_BOTT_R)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "$(LINE) $(COMPILED)$(BLUE)\"$^\"$(RESET)"
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	@echo "$(LINE) $(CLEAN_OBJ)"

fclean: clean
	rm -f $(NAME)
	@echo "$(LINE) $(CLEAN_EXEC)"

re: fclean all

.PHONY: all clean fclean re
