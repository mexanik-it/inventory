# Пути к директориям MSYS2
MSYS2_PATH = C:/msys64/mingw32
INCLUDE_DIRS = $(MSYS2_PATH)/include
LIB_DIRS = $(MSYS2_PATH)/lib
BIN_DIR = $(MSYS2_PATH)/bin

# Целевой файл
TARGET = inventory-x86.exe

# Компилятор и флаги
CXX = $(BIN_DIR)/i686-w64-mingw32-g++
CXXFLAGS = -Wall -Wextra -std=c++20 \
           -I$(INCLUDE_DIRS) \
           -L$(LIB_DIRS) \
           -static

LDFLAGS = -static -static-libgcc -static-libstdc++ \
          -Wl,--static -Wl,--no-undefined \
          -lws2_32 -lwininet -lwinspool \
          -lole32 -loleaut32 -lwbemuuid \
          -lpthread -lmingw32 -lkernel32 -ladvapi32 -lshell32 -luser32

# Автоматическое определение всех .cpp файлов
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

# Добавление зависимостей
DEPS = $(SRCS:.cpp=.d)

.PHONY: all rebuild clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo Linking $(TARGET)...
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo Linking complete.

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# Включение зависимостей
-include $(DEPS)

clean:
	del /Q $(OBJS) $(TARGET) $(DEPS) 2>nul
	@echo Clean done.

rebuild: clean all
	@echo Rebuild complete.

run: $(TARGET)
	./$(TARGET)
