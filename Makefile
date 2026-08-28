# **************************************************************************** #
#                                                                              #
#    Makefile                                            :+:      :+:    :+:    #
#                                                                              #
# **************************************************************************** #

NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

OBJ_DIR		= obj
SRC_DIR		= coders
INCLUDE		= -I coders

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

HEADER		= codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
