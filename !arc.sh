#!/usr/bin/env bash
clear
set -euo pipefail

# Цвета (ANSI)
C_RESET='\033[0m'
C_GREEN='\033[92m'
C_YELLOW='\033[93m'
C_BLUE='\033[94m'
C_CYAN='\033[96m'

echo -e "${C_GREEN}===== start =====${C_RESET}"

# Получаем текущую дату/время в формате YYYY-MM-DD_HH-MM-SS
# В Linux нет wmic, используем date
dt=$(date +"%Y-%m-%d_%H-%M-%S")

# Время начала
time_start=$(date +"%H:%M")

# Архивация: lha a ../inventory-"$dt".lha *
# Если lha не установлен — можно заменить на zip/tar/7z
if command -v tar >/dev/null 2>&1; then
  tar -czf "../inventory-${dt}.tar.gz" ./*
  else
    echo "lha не найден. Пропускаем архивацию (установите lha или замените на tar/zip)." >&2
      # Пример альтернативы: tar czf "../inventory-${dt}.tar.gz" .
      fi
      
      # Время окончания
      time_end=$(date +"%H:%M")
      
      echo
      echo -e "${C_RESET}====="
      echo Compressing time:
      echo "$time_start"
      echo "$time_end"
      
      echo -e "${C_YELLOW}===============================================================================${C_RESET}"
      echo -e "${C_GREEN}========       Transferring INVENTORY files to an FTP server           ========${C_RESET}"
      echo
      
      echo -e "${C_BLUE}Starting process ..${C_RESET}"
      echo -e "${C_CYAN}${C_RESET}"
      
      archive_file="../inventory-${dt}.tar.gz"
      
      # Загрузка на первый FTP
      curl -i -T "$archive_file" -u a0831755 "ftp://denkin.ru/domains/denkin.ru/public_html/tmp/"
      
      # Загрузка на второй FTP
      curl -i -T "$archive_file" -u dimon "ftp://86.110.167.86/ftp/_inventory/"
      
      echo -e "${C_RESET}"
      echo
      
      # Аналог pause: ожидание нажатия Enter
      read -p "Press Enter to continue..."
      