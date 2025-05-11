NAME := ft_ping

CC := gcc
CFLAGS := -Wall -Wextra -Werror

LD := $(CC)
LDFLAGS := 

SRC := $(shell find src -name '*.c')
OBJ := $(patsubst src/%.c,obj/%.o,$(SRC))

all: $(NAME)

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	mkdir -p build
	$(LD) $(LDFLAGS) -o build/$(NAME) $(OBJ) 

clean:
	rm -rf obj

fclean: clean
	rm -rf build

re: 
	$(MAKE) fclean 
	$(MAKE) all

.PHONY: all clean fclean re
-include $(DEP)
