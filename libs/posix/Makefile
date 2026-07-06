##
## EPITECH PROJECT, 2026
## Zappy
## File description:
## POSIX library Makefile
##

NAME = libzappy_posix.a

ROOT = ../..
BUILD_DIR = $(ROOT)/build/posix
OBJ_DIR = $(BUILD_DIR)/obj
SRC_EXTENSION = .cpp

SRC = $(wildcard src/*$(SRC_EXTENSION))
OBJ = $(SRC:src/%$(SRC_EXTENSION)=$(OBJ_DIR)/%.o)
DEPS = $(OBJ:.o=.d)

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror
CXXFLAGS += -I include/
CXXFLAGS += -MMD -MP

AR = ar
ARFLAGS = rcs

TEST_BIN = unit_tests_posix
TEST_SRC = $(wildcard tests/*$(SRC_EXTENSION))
TEST_CXXFLAGS = -std=c++20 -Wall -Wextra -I include/
TEST_LDLIBS = -lcriterion

all:		$(NAME)

$(NAME):	$(OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(OBJ_DIR)/%.o:	src/%$(SRC_EXTENSION)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

tests_run:	$(NAME)
	$(CXX) $(TEST_CXXFLAGS) $(TEST_SRC) -L$(CURDIR) -lzappy_posix $(TEST_LDLIBS) -o $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)

fclean:	clean
	rm -f $(NAME)
	rm -f $(TEST_BIN)

re:	fclean all

.PHONY: all tests_run clean fclean re
