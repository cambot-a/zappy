##
## EPITECH PROJECT, 2026
## Zappy
## File description:
## GUI Makefile
##

NAME = zappy_gui

ROOT = ../..
BIN_DIR = $(ROOT)
BUILD_DIR = $(ROOT)/build/gui
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

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
CXXFLAGS += -iquote $(ROOT)/include/
CXXFLAGS += -I$(POSIX_LIB_DIR)/include
CXXFLAGS += -I$(NET_LIB_DIR)/include
CXXFLAGS += -I$(PROTOCOL_LIB_DIR)/include
CXXFLAGS += -I$(ROOT)/libs/raygui/include
CXXFLAGS += -I$(ROOT)/libs/skybox

LDFLAGS  = -L$(NET_LIB_DIR) -lzappy_net
LDFLAGS += -L$(POSIX_LIB_DIR) -lzappy_posix
LDFLAGS += -L$(PROTOCOL_LIB_DIR) -lzappy_protocol
LDFLAGS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

DEBUG_CFLAGS = -g3 -DDEBUG -pedantic
DEBUG_LDFLAGS = 
DEBUG_CFLAGS_ASAN = -fsanitize=address
DEBUG_LDFLAGS_ASAN = -fsanitize=address
DEBUG_CFLAGS_TSAN = -fsanitize=thread 
DEBUG_LDFLAGS_TSAN = -fsanitize=thread

all:	$(BIN_DIR)/$(NAME)

$(POSIX_LIB):
	@$(MAKE) -C $(POSIX_LIB_DIR)

$(NET_LIB):
	@$(MAKE) -C $(NET_LIB_DIR)

$(PROTOCOL_LIB):
	@$(MAKE) -C $(PROTOCOL_LIB_DIR)

$(BIN_DIR)/$(NAME):	$(OBJ) $(POSIX_LIB) $(NET_LIB) $(PROTOCOL_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

debug_base: CXXFLAGS += $(DEBUG_CFLAGS)
debug_base: LDFLAGS += $(DEBUG_LDFLAGS)
debug_base: $(BIN_DIR)/$(NAME)-debug

debug_asan: DEBUG_CFLAGS += $(DEBUG_CFLAGS_ASAN)
debug_asan: DEBUG_LDFLAGS += $(DEBUG_LDFLAGS_ASAN)
debug_asan : debug_base

debug: debug_asan

$(BIN_DIR)/$(NAME)-debug:	$(DEBUG_OBJ) $(POSIX_LIB) $(NET_LIB) $(PROTOCOL_LIB)
	$(CXX) -o $(BIN_DIR)/$(NAME) $(DEBUG_OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o:	%$(SRC_EXTENSION)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DBG_DIR)/%.debug.o:	%$(SRC_EXTENSION)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(BIN_DIR)/$(NAME)

re: fclean all

debugre: fclean debug

debug_base_re: fclean debug_base

.PHONY: all debug clean fclean re debugre debug_asan debug_base debug_base_re
