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

.PHONY: all clean test
