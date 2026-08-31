#include "main.h"

/***************************************************************************************************/
/* возвращает mac-адрес сетевой карты                                                              */
/***************************************************************************************************/


#include <windows.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstring> // Для strlen

// Простая функция trim для очистки концов строки
static std::string trim(const std::string& str) {
  size_t start = 0;
  while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
    ++start;
  }
  if (start == str.size()) {
    return "";
  }
  size_t end = str.size() - 1;
  while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
    --end;
  }
  return str.substr(start, end - start + 1);
}

bool TInventory::get_mac() {
  FILE* pipe = _popen("getmac /fo csv /nh", "r");  // CSV без заголовка — самый удобный формат
  if (!pipe) {
    std::cerr << "Failed to execute getmac.\n";
    return false;
  }

  char buffer[512];
  std::string result;

  // Читаем все строки: в CSV каждая строка — один адаптер
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    // Убираем \r\n
    size_t len = std::strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') buffer[--len] = '\0';
    if (len > 0 && buffer[len - 1] == '\r') buffer[--len] = '\0';

    std::string line(buffer);
    line = trim(line);
    if (line.empty()) continue;

    result = line;
    break;  // берём первый найденный MAC (можно изменить логику, если нужно все)
  }
  _pclose(pipe);

  if (result.empty()) {
    std::cerr << "No MAC address found in getmac output.\n";
    return false;
  }

  // В CSV: "Physical Address","Connection Name"
  // MAC находится между первой парой кавычек
  size_t firstQuote = result.find('"');
  if (firstQuote == std::string::npos) {
    std::cerr << "Unexpected CSV format from getmac.\n";
    return false;
  }
  size_t secondQuote = result.find('"', firstQuote + 1);
  if (secondQuote == std::string::npos || secondQuote == firstQuote + 1) {
    std::cerr << "Invalid MAC address format in CSV.\n";
    return false;
  }

  std::string mac = result.substr(firstQuote + 1, secondQuote - firstQuote - 1);
  mac = trim(mac);

  if (mac.empty()) {
    std::cerr << "Empty MAC address after parsing.\n";
    return false;
  }

  // Приводим к верхнему регистру
  for (char& c : mac) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }

  id_mac = mac;
  return true;
}
