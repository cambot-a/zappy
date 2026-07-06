##
## EPITECH PROJECT, 2026
## Zappy
## File description:
## AI Makefile
##

NAME = zappy_ai

ROOT = ../..
BIN_DIR = $(ROOT)
BUILD_DIR = $(ROOT)/build/ia

SRC		=	main.py
MODULES	=	connection.py \
			handshake.py \
			line_buffer.py \
			sender.py \
			dispatcher.py \
			event_loop.py \
			inventory.py \
			look.py \
			broadcast.py \
			elevation.py \
			player_state.py \
			torus.py \
			navigation.py \
			broadcast_direction.py \
			stones.py \
			broadcast_protocol.py \
			fsm_helpers.py \
			survival.py \
			foraging.py \
			collecting.py \
			have_broadcast.py \
			coordination.py \
			leading_elevation.py \
			scouting.py \
			forking.py \
			fsm.py

all:	$(BIN_DIR)/$(NAME)

$(BIN_DIR)/$(NAME):	$(SRC) $(MODULES)
	@mkdir -p $(BIN_DIR) $(BUILD_DIR)
	cp $(SRC) $(BIN_DIR)/$(NAME)
	cp $(MODULES) $(BIN_DIR)/
	chmod +x $(BIN_DIR)/$(NAME)

debug:	all

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(BIN_DIR)/$(NAME)

re: fclean all

debugre: fclean debug

.PHONY: all debug clean fclean re debugre
