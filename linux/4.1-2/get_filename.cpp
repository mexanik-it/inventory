#include "main.h"
#include <algorithm>

// Способ 1: цикл + replace()
std::string replace_with_loop(std::string str, char old_c, char new_c) {
    for (size_t pos = 0; (pos = str.find(old_c, pos)) != std::string::npos; pos++) {
        str.replace(pos, 1, 1, new_c);
    }
    return str;
}

// Способ 2: std::replace (рекомендуемый)
std::string replace_with_std(std::string str, char old_c, char new_c) {
    std::replace(str.begin(), str.end(), old_c, new_c);
    return str;
}

// Способ 3:  Цикл с find() и прямой заменой
std::string replace_char(std::string str, char old_char, char new_char) {
    size_t pos = 0;
    while ((pos = str.find(old_char, pos)) != std::string::npos) {
        str[pos] = new_char;
        pos++; // Сдвигаем позицию для поиска следующего вхождения
    }
    return str;
}
bool TInventory::get_filename( ) {

  
  id_filename = id_mac;
  //replace_with_loop( id_filename, ':', '-' );
  //replace_with_std( id_filename, ':', '-' );
  id_filename = replace_with_std( id_filename, ':', '-' );
  id_filename.append( ".xml" );

  return true;
}
