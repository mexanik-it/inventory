CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20 -I./include
LDFLAGS = -lgdi32 -luser32 -lcomctl32

# Пути
SRC_DIR = source
OBJ_DIR = obj
BIN_DIR = bin

# Сбор всех .cpp из папки source
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = txhx_tool.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Правило компиляции: source/file.cpp -> obj/file.o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OBJ_DIR)\*.o $(TARGET) 2>nul

rebuild: clean all
