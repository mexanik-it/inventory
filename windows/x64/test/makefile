# --- Clean Build Script ---

CXX := g++
TARGET := inventory.exe
BUILD_DIR := build

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# ИСПРАВЛЕНО: Добавлены -lole32, -luuid, -loleaut32
LDFLAGS := -liphlpapi -lws2_32 -lwbemuuid -lole32 -luuid -loleaut32 -lcomsuppw -lgdi32  -lsupc++

.PHONY: all clean rebuild info

all: dirs $(TARGET)
	@echo "Build OK."

$(TARGET): $(OBJS)
	@echo ""
	@echo "[LD] Linking $@..."
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@echo "[CC] Compiling $<"
	$(CXX) -std=c++17 -Wall -I. -c $< -o $@

dirs: $(BUILD_DIR)
$(BUILD_DIR):
	@mkdir "$(BUILD_DIR)" 2>nul || rem ok

clean:
	@echo "[CLEAN] Removing artifacts..."
	@rmdir /s /q "$(BUILD_DIR)" 2>nul
	@del /f /q "$(TARGET)" 2>nul

rebuild: clean all