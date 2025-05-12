NAME := ft_ping

CC := gcc
CFLAGS := -Wall -Wextra -Werror -iquoteinclude -g

LD := $(CC)
LDFLAGS := 

SRC := $(shell find src -name '*.c')
OBJ := $(patsubst src/%.c,obj/%.o,$(SRC))

all: $(NAME)

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	$(LD) $(LDFLAGS) -o $(NAME) $(OBJ) 

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)

re: 
	$(MAKE) fclean 
	$(MAKE) all

.PHONY: all clean fclean re
