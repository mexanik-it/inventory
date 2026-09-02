TARGET = inventory.exe
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20
LDFLAGS = -static -static-libstdc++ -lws2_32 -lwininet -lwinspool -lole32 -loleaut32 -lwbemuuid

SRCS = main.cpp added.cpp get_mb.cpp get_cpu.cpp get_hdd.cpp \
       get_mem.cpp get_ip.cpp get_host.cpp get_prn.cpp     \
       get_mac.cpp  get_other.cpp               \
       get_sys.cpp  get_filename.cpp   \
       write_to_file.cpp write_to_ftp.cpp write_to_lan.cpp \
       TInventory.cpp
#	get_hdd.cpp

# get_hdd_size.cpp

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
