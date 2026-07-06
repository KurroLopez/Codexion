# **************************************************************************** #
#                                                                              #
#    Makefile                                            :+:      :+:    :+:    #
#                                                                              #
# **************************************************************************** #

NAME		= codexion

CC		= cc
CFLAGS		= -Wall -Wextra -Werror

SRC_DIR		= coders
OBJ_DIR		= obj

SRCS		= main.c \
		  init.c \
		  utils.c \
		  heap.c \
		  dongle.c \
		  simulation.c

OBJS		= $(SRCS:.c=.o))

HEADER		= $(SRC_DIR)/codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS)  $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
