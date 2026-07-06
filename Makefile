##
## EPITECH PROJECT, 2026
## Zappy
## File description:
## Root Makefile
##

SERVER = zappy_server
GUI = zappy_gui
AI = zappy_ai

BINS = $(SERVER) $(GUI) $(AI)

SERVER_DIR = src/server
GUI_DIR = src/gui
AI_DIR = src/ia
POSIX_LIB_DIR = libs/posix
NET_LIB_DIR = libs/net
PROTOCOL_LIB_DIR = libs/protocol

BUILD_DIR = build

TEST_BIN = unit_tests
TEST_CXX = g++
TEST_CXXFLAGS = -std=c++20 -Wall -Wextra -iquote include/ -I$(POSIX_LIB_DIR)/include -I$(NET_LIB_DIR)/include -I$(PROTOCOL_LIB_DIR)/include
TEST_LDLIBS = -lcriterion

COV_FLAGS = --coverage
TEST_SRC = src/server/cli/CliParser.cpp \
			src/server/Server.cpp \
			src/server/client/Client.cpp \
			src/server/client/ClientRegistry.cpp \
			src/server/game/Position.cpp \
			src/server/game/Tile.cpp \
			src/server/game/Player.cpp \
			src/server/game/Egg.cpp \
			src/server/game/Team.cpp \
			src/server/game/World.cpp \
			src/server/game/ResourceSpawner.cpp \
			src/server/game/BiomeMapGenerator.cpp \
			src/server/game/EggSpawner.cpp \
			src/server/game/RefillScheduler.cpp \
			src/server/game/FoodScheduler.cpp \
			src/server/game/VisionCone.cpp \
			src/server/game/BroadcastDirection.cpp \
			src/server/scheduler/Scheduler.cpp \
			src/server/handshake/HandshakeHandler.cpp \
			src/server/ai/AiDispatcher.cpp \
			src/server/ai/LookResponseBuilder.cpp \
			src/server/gui/GuiDispatcher.cpp \
			src/server/gui/GuiLineBuilder.cpp \
			src/server/gui/GuiNotifier.cpp \
			src/server/config/FeatureFlags.cpp \
			src/server/event/EventBus.cpp \
			src/server/event/Storm.cpp \
			src/server/event/Flood.cpp \
			src/server/event/Meteor.cpp \
		    $(wildcard tests/cli/*.cpp) \
			$(wildcard tests/server/config/*.cpp) \
			$(wildcard tests/server/client/*.cpp) \
			$(wildcard tests/server/game/*.cpp) \
			$(wildcard tests/server/scheduler/*.cpp) \
			$(wildcard tests/server/handshake/*.cpp) \
			$(wildcard tests/server/ai/*.cpp) \
			$(wildcard tests/server/gui/*.cpp) \
			$(wildcard tests/server/event/*.cpp) \
			$(wildcard tests/protocol/ai/*.cpp) \
			$(wildcard tests/protocol/gui/*.cpp)

all:	$(BINS)

$(SERVER):
	@$(MAKE) -C $(SERVER_DIR)

$(GUI):
	@$(MAKE) -C $(GUI_DIR)

$(AI):
	@$(MAKE) -C $(AI_DIR)

debug:
	@$(MAKE) -C $(SERVER_DIR) debug
	@$(MAKE) -C $(GUI_DIR) debug
	@$(MAKE) -C $(AI_DIR) debug

clean:
	@$(MAKE) -C $(SERVER_DIR) clean
	@$(MAKE) -C $(GUI_DIR) clean
	@$(MAKE) -C $(AI_DIR) clean
	@$(MAKE) -C $(POSIX_LIB_DIR) clean
	@$(MAKE) -C $(NET_LIB_DIR) clean
	@$(MAKE) -C $(PROTOCOL_LIB_DIR) clean
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(BINS)
	rm -f $(TEST_BIN)

	rm -f *.gcno *.gcda *.gcov

tests_run:
	@$(MAKE) -C $(POSIX_LIB_DIR) tests_run
	@$(MAKE) -C $(NET_LIB_DIR) tests_run
	@$(MAKE) -C $(PROTOCOL_LIB_DIR)
	$(TEST_CXX) $(TEST_CXXFLAGS) $(TEST_SRC) \
		-L$(NET_LIB_DIR) -lzappy_net \
		-L$(POSIX_LIB_DIR) -lzappy_posix \
		-L$(PROTOCOL_LIB_DIR) -lzappy_protocol \
		-o $(TEST_BIN) $(TEST_LDLIBS)
	./$(TEST_BIN)
	python3 -m pytest tests/ia/ -q

tests_cov: TEST_CXXFLAGS += $(COV_FLAGS)
tests_cov: TEST_LDLIBS += $(COV_FLAGS)
tests_cov: tests_run
	gcovr --exclude tests/ --print-summary

re: fclean all

debugre: fclean debug

.PHONY: all debug clean fclean re debugre tests_run tests_cov $(SERVER) $(GUI) $(AI)
