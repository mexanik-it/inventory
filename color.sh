#!/bin/bash
echo "Таблица 256 ANSI цветов (foreground):"
for i in {0..255}; do
  printf "\033[38;5;%sm%3s\033[0m " "$i" "$i"
  if (( (i + 1) % 16 == 0 )); then
    echo
  fi
done
