##
## EPITECH PROJECT, 2026
## Zappy
## File description:
## Server Makefile
##

NAME = zappy_server

ROOT = ../..
BIN_DIR = $(ROOT)
BUILD_DIR = $(ROOT)/build/server
OBJ_DIR = $(BUILD_DIR)/obj
DBG_DIR = $(BUILD_DIR)/debug
SRC_EXTENSION = .cpp

POSIX_LIB_DIR = $(ROOT)/libs/posix
POSIX_LIB = $(POSIX_LIB_DIR)/libzappy_posix.a

NET_LIB_DIR = $(ROOT)/libs/net
NET_LIB = $(NET_LIB_DIR)/libzappy_net.a

PROTOCOL_LIB_DIR = $(ROOT)/libs/protocol
PROTOCOL_LIB = $(PROTOCOL_LIB_DIR)/libzappy_protocol.a

SRC = $(wildcard *$(SRC_EXTENSION)) \
		$(wildcard **/*$(SRC_EXTENSION)) \
		$(wildcard **/**/*$(SRC_EXTENSION))

OBJ = $(SRC:%$(SRC_EXTENSION)=$(OBJ_DIR)/%.o)
DEBUG_OBJ = $(SRC:%$(SRC_EXTENSION)=$(DBG_DIR)/%.debug.o)
DEPS = $(OBJ:.o=.d) $(DEBUG_OBJ:.o=.d)

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
CXXFLAGS += -iquote $(ROOT)/include/
CXXFLAGS += -I$(POSIX_LIB_DIR)/include
CXXFLAGS += -I$(NET_LIB_DIR)/include
CXXFLAGS += -I$(PROTOCOL_LIB_DIR)/include
CXXFLAGS += -MMD -MP

LDFLAGS = -L$(NET_LIB_DIR) -lzappy_net -L$(POSIX_LIB_DIR) -lzappy_posix
LDFLAGS += -L$(PROTOCOL_LIB_DIR) -lzappy_protocol

DEBUG_CFLAGS = -g3 -fsanitize=address -DDEBUG
DEBUG_LDFLAGS = -fsanitize=address

all:	$(BIN_DIR)/$(NAME)

$(POSIX_LIB): FORCE
	@$(MAKE) -C $(POSIX_LIB_DIR)

$(NET_LIB): FORCE
	@$(MAKE) -C $(NET_LIB_DIR)

$(PROTOCOL_LIB): FORCE
	@$(MAKE) -C $(PROTOCOL_LIB_DIR)

FORCE:

$(BIN_DIR)/$(NAME):	$(OBJ) $(POSIX_LIB) $(NET_LIB) $(PROTOCOL_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

debug: CXXFLAGS += $(DEBUG_CFLAGS)
debug: $(BIN_DIR)/$(NAME)-debug

$(BIN_DIR)/$(NAME)-debug:	$(DEBUG_OBJ) $(POSIX_LIB) $(NET_LIB) $(PROTOCOL_LIB)
	$(CXX) -o $(BIN_DIR)/$(NAME) $(DEBUG_OBJ) $(LDFLAGS) $(DEBUG_LDFLAGS)

$(OBJ_DIR)/%.o:	%$(SRC_EXTENSION)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DBG_DIR)/%.debug.o:	%$(SRC_EXTENSION)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(BIN_DIR)/$(NAME)

re: fclean all

debugre: fclean debug

.PHONY: all debug clean fclean re debugre FORCE
