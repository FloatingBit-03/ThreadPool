# ==========================================================
# PacketForge Build System
# ==========================================================


CXX := g++


CXXFLAGS := \
-std=c++20 \
-Wall \
-Wextra \
-Wpedantic \
-g \
-O0 \
-Iinclude \
-MMD \
-MP


GTEST_LIBS := \
-lgtest \
-lgtest_main \
-pthread

# ==========================================================
# Directories
# ==========================================================

SRC_DIR := src
TEST_DIR := tests
BUILD_DIR := build
BIN_DIR := bin



# ==========================================================
# Dependency Files
# ==========================================================

DEPS := $(shell find $(BUILD_DIR) -name "*.d")   

-include $(DEPS)      


# ==========================================================
# Object Files
# ==========================================================

COMMON_OBJ := \
$(BUILD_DIR)/common/error.o \
$(BUILD_DIR)/common/logger.o \
$(BUILD_DIR)/common/buffer.o \
$(BUILD_DIR)/common/endian.o


NETWORK_OBJ := \
$(BUILD_DIR)/network/socket.o \
$(BUILD_DIR)/network/connection.o \
$(BUILD_DIR)/network/transport.o


PROTOCOL_OBJ := \
$(BUILD_DIR)/protocol/packet.o \
$(BUILD_DIR)/protocol/encoder.o \
$(BUILD_DIR)/protocol/decoder.o

CLIENT_OBJ := \
$(BUILD_DIR)/client/client.o


# ==========================================================
# Test Object Files
# ==========================================================

TEST_ERROR_OBJ := \
$(BUILD_DIR)/tests/test_error.o


TEST_LOGGER_OBJ := \
$(BUILD_DIR)/tests/test_logger.o


TEST_BUFFER_OBJ := \
$(BUILD_DIR)/tests/test_buffer.o


TEST_ENDIAN_OBJ := \
$(BUILD_DIR)/tests/test_endian.o


TEST_SOCKET_OBJ := \
$(BUILD_DIR)/tests/test_socket.o


TEST_CONNECTION_OBJ := \
$(BUILD_DIR)/tests/test_connection.o


TEST_TRANSPORT_OBJ := \
$(BUILD_DIR)/tests/test_transport.o


TEST_PACKET_OBJ := \
$(BUILD_DIR)/tests/test_packet.o


TEST_ENCODER_OBJ := \
$(BUILD_DIR)/tests/test_encoder.o


TEST_DECODER_OBJ := \
$(BUILD_DIR)/tests/test_decoder.o

TEST_CLIENT_OBJ := \
$(BUILD_DIR)/tests/test_client.o


# ==========================================================
# Test Executables
# ==========================================================

TEST_ERROR_BIN := \
$(BUILD_DIR)/tests/test_error


TEST_LOGGER_BIN := \
$(BUILD_DIR)/tests/test_logger


TEST_BUFFER_BIN := \
$(BUILD_DIR)/tests/test_buffer


TEST_ENDIAN_BIN := \
$(BUILD_DIR)/tests/test_endian


TEST_SOCKET_BIN := \
$(BUILD_DIR)/tests/test_socket


TEST_CONNECTION_BIN := \
$(BUILD_DIR)/tests/test_connection


TEST_TRANSPORT_BIN := \
$(BUILD_DIR)/tests/test_transport


TEST_PACKET_BIN := \
$(BUILD_DIR)/tests/test_packet


TEST_ENCODER_BIN := \
$(BUILD_DIR)/tests/test_encoder


TEST_DECODER_BIN := \
$(BUILD_DIR)/tests/test_decoder


TEST_CLIENT_BIN := \
$(BUILD_DIR)/tests/test_client

# ==========================================================
# Phony Targets
# ==========================================================

.PHONY: \
all \
test \
clean



# ==========================================================
# Default Target
# ==========================================================

all:

	@echo "PacketForge Build System"
	@echo
	@echo "Available commands:"
	@echo "  make test"
	@echo "  make clean"



# ==========================================================
# Compile Common Sources
# ==========================================================

$(BUILD_DIR)/common/%.o: $(SRC_DIR)/common/%.cpp

	mkdir -p $(dir $@)

	$(CXX) \
	$(CXXFLAGS) \
	-c $< \
	-o $@



# ==========================================================
# Compile Network Sources
# ==========================================================

$(BUILD_DIR)/network/%.o: $(SRC_DIR)/network/%.cpp

	mkdir -p $(dir $@)

	$(CXX) \
	$(CXXFLAGS) \
	-c $< \
	-o $@


# ==========================================================
# Compile Protocol Sources
# ==========================================================

$(BUILD_DIR)/protocol/%.o: $(SRC_DIR)/protocol/%.cpp

	mkdir -p $(dir $@)

	$(CXX) \
	$(CXXFLAGS) \
	-c $< \
	-o $@


# ==========================================================
# Compile Client Sources
# ==========================================================

$(BUILD_DIR)/client/%.o: $(SRC_DIR)/client/%.cpp

	mkdir -p $(dir $@)

	$(CXX) \
	$(CXXFLAGS) \
	-c $< \
	-o $@



# ==========================================================
# Compile Unit Tests
# ==========================================================

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/unit/%.cpp

	mkdir -p $(dir $@)

	$(CXX) \
	$(CXXFLAGS) \
	-c $< \
	-o $@



# ==========================================================
# Link Error Test
# ==========================================================

$(TEST_ERROR_BIN): \
$(BUILD_DIR)/common/error.o \
$(TEST_ERROR_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@



# ==========================================================
# Link Logger Test
# ==========================================================

$(TEST_LOGGER_BIN): \
$(BUILD_DIR)/common/logger.o \
$(TEST_LOGGER_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@



# ==========================================================
# Link Buffer Test
# ==========================================================

$(TEST_BUFFER_BIN): \
$(BUILD_DIR)/common/buffer.o \
$(TEST_BUFFER_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@



# ==========================================================
# Link Endian Test
# ==========================================================

$(TEST_ENDIAN_BIN): \
$(BUILD_DIR)/common/endian.o \
$(TEST_ENDIAN_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@



# ==========================================================
# Link Socket Test
# ==========================================================

$(TEST_SOCKET_BIN): \
$(BUILD_DIR)/network/socket.o \
$(BUILD_DIR)/common/error.o \
$(TEST_SOCKET_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@



# ==========================================================
# Link Connection Test
# ==========================================================

$(TEST_CONNECTION_BIN): \
$(BUILD_DIR)/network/socket.o \
$(BUILD_DIR)/network/connection.o \
$(BUILD_DIR)/common/error.o \
$(TEST_CONNECTION_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@


# ==========================================================
# Link Transport Test
# ==========================================================

$(TEST_TRANSPORT_BIN): \
$(BUILD_DIR)/network/socket.o \
$(BUILD_DIR)/network/connection.o \
$(BUILD_DIR)/network/transport.o \
$(BUILD_DIR)/common/error.o \
$(TEST_TRANSPORT_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@


# ==========================================================
# Link Packet Test
# ==========================================================

$(TEST_PACKET_BIN): \
$(BUILD_DIR)/protocol/packet.o \
$(TEST_PACKET_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@


# ==========================================================
# Link Encoder Test
# ==========================================================

$(TEST_ENCODER_BIN): \
$(BUILD_DIR)/protocol/packet.o \
$(BUILD_DIR)/protocol/encoder.o \
$(BUILD_DIR)/common/endian.o \
$(TEST_ENCODER_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@



# ==========================================================
# Link Decoder Test
# ==========================================================

$(TEST_DECODER_BIN): \
$(BUILD_DIR)/protocol/packet.o \
$(BUILD_DIR)/protocol/encoder.o \
$(BUILD_DIR)/protocol/decoder.o \
$(BUILD_DIR)/common/endian.o \
$(TEST_DECODER_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	$^ \
	$(GTEST_LIBS) \
	-o $@



# ==========================================================
# Link Client Test
# ==========================================================

$(TEST_CLIENT_BIN): \
    $(BUILD_DIR)/client/client.o \
    $(BUILD_DIR)/network/socket.o \
    $(BUILD_DIR)/network/connection.o \
    $(BUILD_DIR)/network/transport.o \
    $(BUILD_DIR)/protocol/packet.o \
    $(BUILD_DIR)/protocol/encoder.o \
    $(BUILD_DIR)/protocol/decoder.o \
    $(BUILD_DIR)/common/endian.o \
    $(BUILD_DIR)/common/error.o \
    $(TEST_CLIENT_OBJ)

	mkdir -p $(dir $@)

	$(CXX) \
	    $^ \
	    $(GTEST_LIBS) \
	    -o $@



# ==========================================================
# Run All Unit Tests
# ==========================================================

test: \
$(TEST_ERROR_BIN) \
$(TEST_LOGGER_BIN) \
$(TEST_BUFFER_BIN) \
$(TEST_ENDIAN_BIN) \
$(TEST_SOCKET_BIN) \
$(TEST_CONNECTION_BIN) \
$(TEST_TRANSPORT_BIN) \
$(TEST_PACKET_BIN) \
$(TEST_ENCODER_BIN) \
$(TEST_DECODER_BIN) \
$(TEST_CLIENT_BIN)


	@echo
	@echo "========================================="
	@echo "Running PacketForge Unit Tests"
	@echo "========================================="


	./$(TEST_ERROR_BIN)

	./$(TEST_LOGGER_BIN)

	./$(TEST_BUFFER_BIN)

	./$(TEST_ENDIAN_BIN)

	./$(TEST_SOCKET_BIN)

	./$(TEST_CONNECTION_BIN)

	./$(TEST_TRANSPORT_BIN)

	./$(TEST_PACKET_BIN)

	./$(TEST_ENCODER_BIN)

	./$(TEST_DECODER_BIN)

	./$(TEST_CLIENT_BIN)


# ==========================================================
# Clean Build Artifacts
# ==========================================================

clean:

	rm -rf $(BUILD_DIR)/*

	rm -rf $(BIN_DIR)/*


	@echo
	@echo "Build artifacts removed."