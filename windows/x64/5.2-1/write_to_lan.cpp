/* write_to_lan.cpp */
#include "main.h"
#include <iostream>
#include <fstream>
#include <string>

// Функция копирования файла (должна быть в этом же файле или в отдельном .cpp, который компилируется)
bool copyFile(const std::string& source, const std::string& destination) {
    // Проверка существования источника (опционально, но полезно для логов)
    std::ifstream src(source, std::ios::binary);
    if (!src.is_open()) {
        // Если нужно логировать ошибку внутри функции копирования, можно вернуть false
        return false;
    }

    std::ofstream dst(destination, std::ios::binary);
    if (!dst.is_open()) {
        src.close();
        return false;
    }

    // Быстрое копирование всего содержимого
    dst << src.rdbuf();

    // Проверяем, не произошла ли ошибка при записи (например, диск переполнен)
    if (src.fail() || dst.fail()) {
        src.close();
        dst.close();
        return false;
    }

    src.close();
    dst.close();
    return true;
}

bool TInventory::write_to_lan() {
    const std::string networkPath = "\\\\10.67.2.33\\INVENTORY-BASE"; 
    
    // Если твой текущий путь "//..." реально работает через SMB в твоей среде — оставь его.
    // Но стандартный формат для std::ofstream в Windows: "\\\\server\\share\\file"
    
    if (id_filename.empty()) {
        errMessage("Не указан путь к файлу для копирования (id_filename пуст)");
        return false;
    }

    // Формируем полное имя файла на сетевом ресурсе. 
    // Важно: если в networkPath нет имени файла, нужно добавить его вручную.
    // Предполагаю, что INVENTORY-BASE — это папка, и мы копируем туда тот же файл.
    std::string destPath = networkPath + "\\" + id_filename;

    if (copyFile(id_filename, destPath)) {
        okMessage("Файл скопирован на сетевой ресурс: " + destPath);
        return true;
    } else {
        errMessage("Ошибка при копировании файла: " + id_filename + " -> " + destPath);
        return false;
    }
}
