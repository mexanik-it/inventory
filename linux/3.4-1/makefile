SHELL := /bin/bash

TARGET = ./inventory
CXX = g++
CXXFLAGS = -Wall -std=c++20
LDFLAGS = -static -static-libstdc++

SRCS = main.cpp added.cpp get_mb.cpp get_cpu.cpp           \
       get_mem.cpp get_ip.cpp get_host.cpp get_prn.cpp     \
       get_mac.cpp get_hdd.cpp get_other.cpp               \
       get_sys.cpp get_hdd_size.cpp get_filename.cpp       \
       write_to_file.cpp write_to_ftp.cpp write_to_lan.cpp \
       TInventory.cpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean rebuild run

# Главная цель: замеряем ВСЁ: компиляцию всех .cpp и линковку
all:
	@if [ -t 1 ]; then tput clear; fi
	@echo "===== Начало полной сборки $(TARGET) ====="
	@echo ""
	@/usr/bin/time --format="Общее время сборки: real %E, user %U, sys %S" \
		$(MAKE) -s $(TARGET)
	@echo ""
	
# Правило для самого бинарника
$(TARGET): $(OBJS)
	@echo ""
	@echo "--- Линковка $(TARGET) ---"
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)
	@echo ""
	@ls -lh $(TARGET) | awk '{print "Итоговый размер: " $$5}'

# Правило компиляции: @ скрывает саму команду g++, оставляем только наш статус
%.o: %.cpp
	@echo "[compiling] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	@echo ""
	@echo "Все временные файлы удалены."
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

rebuild: clean all
	@echo ""
	@echo "=== Полная пересборка завершена ==="
