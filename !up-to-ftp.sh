#!/usr/bin/env bash
set -euo pipefail

# Установка локали для корректного отображения кириллицы (если нужно)
export LANG=ru_RU.UTF-8

# Цвета
YELLOW='\033[33m'
GREEN='\033[92m'
CYAN='\033[96m'
ORANGE='\033[93m'
RED='\033[91m'
RESET='\033[0m'

echo -e "${YELLOW}===============================================================================${RESET}"
echo -e "${GREEN}========      Transferring INVENTORY files to an FTP server            ========${RESET}"
echo

# Функция для запроса подтверждения с автопродолжением
confirm() {
    local prompt="$1"
    echo -n -e "${prompt} Автопродолжение через 5 сек. Продолжить? [Y/n]: ${RESET}"
    
    # read с таймаутом (в секундах)
    if read -t 5 -r -n 1 response; then
        # Если пользователь что-то ввёл — проверяем
        case "$response" in
            [Nn])
                echo -e "\n${RED}Выполнение прервано${RESET}"
                exit 1
                ;;
        esac
    else
        # Таймаут истёк — считаем как «да»
        echo -e "${CYAN}Автопродолжение...${RESET}"
    fi
}

# Передача файла для Linux
confirm ""
echo -e "${CYAN}Starting process ..${ORANGE}"
if curl -T linux/inventory "ftp://a0831755:vbnz1002@ftp.denkin.ru/domains/denkin.ru/public_html/tmp/"; then
    echo -e "${GREEN}[Linux] - File transfer completed successfully...${RESET}"
else
    echo -e "${RED}[Linux] - An error occurred while transferring the file...${RESET}"
    exit 1
fi

echo

# Передача файла для Windows
confirm ""
echo -e "${CYAN}Starting process ..${ORANGE}"
if curl -T windows/inventory.exe "ftp://dimon:1002@86.110.167.96/ftp/_inventory/"; then
    echo -e "${GREEN}[Windows] - File transfer completed successfully...${RESET}"
else
    echo -e "${RED}[Windows] - An error occurred while transferring the file...${RESET}"
    exit 1
fi

echo
# Аналог pause: ожидание нажатия любой клавиши
read -p "Нажмите любую клавишу для выхода..." -n 1 -s
echo
