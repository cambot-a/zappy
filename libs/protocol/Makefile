##
## EPITECH PROJECT, 2026
## Zappy
## File description:
## Protocol library Makefile (pure logic, no posix/net dependency)
##

NAME = libzappy_protocol.a

ROOT = ../..

BUILD_DIR = $(ROOT)/build/protocol

OBJ_DIR = $(BUILD_DIR)/obj

SRC_EXTENSION = .cpp

SRC = $(wildcard src/ai/*$(SRC_EXTENSION)) \
		$(wildcard src/gui/*$(SRC_EXTENSION)) \
		$(wildcard src/common/*$(SRC_EXTENSION))

OBJ = $(SRC:src/%$(SRC_EXTENSION)=$(OBJ_DIR)/%.o)
DEPS = $(OBJ:.o=.d)

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror
CXXFLAGS += -I include/
CXXFLAGS += -MMD -MP

AR = ar
ARFLAGS = rcs

all:	$(NAME)

$(NAME):	$(OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(OBJ_DIR)/%.o: src/%$(SRC_EXTENSION)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
