CXX = C:/Qt/Tools/mingw1310_64/bin/g++.exe
CXXFLAGS = -std=c++17 -Wall -Wextra -g
SRC_DIR = src
BUILD_DIR = build
TARGET = rustly-compiler

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

test: $(TARGET)
	@echo "Running test_basic.txt..."
	./$(TARGET) tests/test_basic.txt
	@echo ""
	@echo "Running test_expr.txt..."
	./$(TARGET) tests/test_expr.txt
	@echo ""
	@echo "Running test_func.txt..."
	./$(TARGET) tests/test_func.txt
	@echo ""
	@echo "Running test_control.txt..."
	./$(TARGET) tests/test_control.txt

.PHONY: all clean test
