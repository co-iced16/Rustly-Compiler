CXX = C:/Qt/Tools/mingw1310_64/bin/g++.exe
CXXFLAGS = -std=c++17 -Wall -Wextra -g
LDFLAGS = -static-libgcc -static-libstdc++ -static
SRC_DIR = src
BUILD_DIR = build
TARGET = rustly-compiler.exe

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# On Windows, run directory commands via cmd.exe so recipes work from PowerShell, CMD, or Git Bash.
ifeq ($(OS),Windows_NT)
# Use //C so MSYS/Git Bash does not strip the flag (plain /C can start an interactive cmd and hang).
$(BUILD_DIR):
	cmd.exe //C "if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)"

clean:
	cmd.exe //C "if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR) & if exist $(TARGET) del /F /Q $(TARGET)"

RUN_TARGET = ./$(TARGET)
else
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

RUN_TARGET = ./$(TARGET)
endif

test: $(TARGET)
	@echo "Running test_basic.txt..."
	$(RUN_TARGET) tests/test_basic.txt
	@echo ""
	@echo "Running test_expr.txt..."
	$(RUN_TARGET) tests/test_expr.txt
	@echo ""
	@echo "Running test_func.txt..."
	$(RUN_TARGET) tests/test_func.txt
	@echo ""
	@echo "Running test_control.txt..."
	$(RUN_TARGET) tests/test_control.txt
	@echo ""
	@echo "Running test_for_loop.txt..."
	$(RUN_TARGET) tests/test_for_loop.txt
	@echo ""
	@echo "Running test_loop.txt..."
	$(RUN_TARGET) tests/test_loop.txt
	@echo ""
	@echo "Running test_else_if.txt..."
	$(RUN_TARGET) tests/test_else_if.txt
	@echo ""
	@echo "Running test_comprehensive.txt..."
	$(RUN_TARGET) tests/test_comprehensive.txt
	@echo ""
	@echo "Running test_reference.txt..."
	$(RUN_TARGET) tests/test_reference.txt
	@echo ""
	@echo "Running test_array.txt..."
	$(RUN_TARGET) tests/test_array.txt
	@echo ""
	@echo "Running test_tuple.txt..."
	$(RUN_TARGET) tests/test_tuple.txt
	@echo ""
	@echo "Running test_block_expr.txt..."
	$(RUN_TARGET) tests/test_block_expr.txt
	@echo ""
	@echo "Running test_division.txt..."
	$(RUN_TARGET) tests/test_division.txt
	@echo ""
	@echo "Running test_not_equal.txt..."
	$(RUN_TARGET) tests/test_not_equal.txt
	@echo ""
	@echo "Running test_empty_params.txt..."
	$(RUN_TARGET) tests/test_empty_params.txt
	@echo ""
	@echo "Running test_comments.txt..."
	$(RUN_TARGET) tests/test_comments.txt
	@echo ""
	@echo "Running test_identifiers.txt..."
	$(RUN_TARGET) tests/test_identifiers.txt
	@echo ""
	@echo "Running test_nested_ref.txt..."
	$(RUN_TARGET) tests/test_nested_ref.txt
	@echo ""
	@echo "Running test_if_expr.txt..."
	$(RUN_TARGET) tests/test_if_expr.txt
	@echo ""
	@echo "Running test_break_with_value.txt..."
	$(RUN_TARGET) tests/test_break_with_value.txt
	@echo ""
	@echo "Running test_empty_collections.txt..."
	$(RUN_TARGET) tests/test_empty_collections.txt

.PHONY: all clean test
