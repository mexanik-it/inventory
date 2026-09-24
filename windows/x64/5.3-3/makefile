TARGET = inventory.exe
CXX = g++

# Получаем имя текущей папки (например, "5.2-2")
BUILD_VERSION := $(notdir $(CURDIR))
CXXFLAGS = -Wall -Wextra -std=c++20
CXXFLAGS += -DBUILD_VERSION_STR="\"$(BUILD_VERSION)\""

LDFLAGS = -static -static-libstdc++ -lws2_32 -lwininet -lwinspool -lole32 -loleaut32 -lwbemuuid -liphlpapi

SRCS = main.cpp added.cpp get_mb.cpp get_cpu.cpp get_hdd.cpp \
       get_mem.cpp get_lan.cpp get_host.cpp get_prn.cpp get_other.cpp \
       get_sys.cpp  get_filename.cpp get_vga.cpp  \
       write_to_file.cpp write_to_ftp.cpp write_to_lan.cpp \
       TInventory.cpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all rebuild clean run

all:
	@cls
	@echo Starting build for $(TARGET)...
	@echo [92m
	@$(MAKE) $(TARGET)

$(TARGET): $(OBJS)
	@echo [0m	
	@echo Linking $(TARGET)...
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo.
	@echo [33mLinking complete.[0m
	@pause
	@echo.

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
#	cls
	$(TARGET)

clean:
	del /Q $(OBJS) $(TARGET) 2>nul
	@echo Clean done.

rebuild: clean all
	@echo.
	@echo [33mRebuild complete.[0m

changelog:
	@chcp 65001
	@echo "Смотрите CHANGELOG.md"
	@more ..\CHANGELOG.md