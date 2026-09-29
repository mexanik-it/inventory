#include "main.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>

/***************************************************************************************************/
/* присваивает переменной id_cpu  название процессора и тактовую частоту                           */
/***************************************************************************************************/

/***************************************************************************************************/
/* Что улучшено                                                                                    */
/* Команда стала точнее: grep 'model name' /proc/cpuinfo | head -n1 берёт только первую строку с   */
/* названием модели (у всех ядер оно одинаковое).                                                  */
/* Обработка пробелов после двоеточия сделана циклом, а не двумя ручными tmp++.                    */
/* Проверка на пустую строку и отсутствие двоеточия — корректная.                                  */
/* Безопасная работа с буфером: 1024 байт обычно достаточно для одной строки model name; если      */
/* нужно больше, можно увеличить.                                                                  */
/* Удаление \n явно, без зависимости от нестандартной функции replace.                             */
/***************************************************************************************************/

bool TInventory::get_cpu() {
    FILE* hand = popen("grep 'model name' /proc/cpuinfo | head -n1", "r");
    if (!hand) {
        perror("Can't open shell process");
        return false;
    }

    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), hand)) {
        pclose(hand);
        return false; // Нет данных
    }
    pclose(hand);

    // Ищем двоеточие после "model name"
    char* tmp = strchr(buffer, ':');
    if (!tmp) {
        return false;
    }
    tmp++; // Пропускаем ':'
    while (*tmp == ' ' || *tmp == '\t') {
        tmp++; // Пропускаем пробелы после ':'
    }

    // Удаляем перевод строки в конце
    size_t len = strlen(tmp);
    if (len > 0 && tmp[len - 1] == '\n') {
        tmp[len - 1] = '\0';
        len--;
    }

    id_cpu = tmp;



    // Убираем лишнее (пример: " Dual Core Processor")
    std::string toRemove = " Dual Core Processor";
    size_t pos = id_cpu.find(toRemove);
    if (pos != std::string::npos)
        id_cpu.erase(pos, toRemove.length());
    toRemove = "Core(TM) ";
    pos = id_cpu.find(toRemove);
    if (pos != std::string::npos)
        id_cpu.erase(pos, toRemove.length());
    toRemove = "CPU @ ";
    pos = id_cpu.find(toRemove);
    if (pos != std::string::npos)
        id_cpu.erase(pos, toRemove.length());

    return true;
}