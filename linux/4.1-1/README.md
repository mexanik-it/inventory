# inventory LINUX

***

###### 24-07-2026
[~] модифицировал вычисление памяти, теперь берутся только стандартые значения из массива \
[~] переделал получение марки принтера, теперь берется из списка изветных (официальные названия) если \
 в списке нет, выводится то, что получили в сыром виде \
[~] тоже самое и с жесткими дисками, включая марку и объём, есть списки, и если есть совпадения, то подставляется \
 заводское название
```
/**************************************************************************************************/
/* returns the hard drive type (SSD/HDD) and size, and if the disk brand is known, both the type  */
/* and size are taken from the disk name                                                          */
/*------------------------------------------------------------------------------------------------*/
/* возвращает тип жесткого диска (SSD/HDD) и размер, если марка диска известна - то и тип и       */
/* размер берутся из названия диска                                                               */
/**************************************************************************************************/
// get_hdd_size.cpp

#include "main.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>

// Возвращает размер диска в байтах (через sysfs)
// Гарантированно использует 512-байтовые логические сектора, как это делает ядро
static uint64_t get_disk_size_bytes(const std::string& dev_name) {
    std::string path = "/sys/block/" + dev_name + "/size";
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть " + path);
    }

    uint64_t sectors = 0;
    file >> sectors;
    if (file.fail()) {
        throw std::runtime_error("Ошибка чтения размера из " + path);
    }
    file.close();

    // size в sysfs — это количество 512-байтовых секторов
    const uint64_t SECTOR_SIZE = 512;
    return sectors * SECTOR_SIZE;
}

static std::string get_disk_type(const std::string& dev_name) {
    std::string path = "/sys/block/" + dev_name + "/queue/rotational";
    std::ifstream file(path);
    std::string value;

    if (file.is_open()) {
        file >> value;
        file.close();
        if (value == "0") return "SSD";
        if (value == "1") return "HDD";
    }
    return "Unknown";
}

// Округление до ближайшего кратного 10 ГБ (десятичных)
static int round_to_10_decimal_gb(uint64_t bytes) {
    constexpr double GB = 1000000000.0;  // десятичный гигабайт
    double gb = static_cast<double>(bytes) / GB;
    int rounded = static_cast<int>(std::round(gb / 10.0) * 10.0);
    return rounded;
}

bool TInventory::get_hdd_size() {
    // Список дисков для проверки
    std::vector<std::string> disks = {"sda", "sdb", "nvme0n1"};


    size_t pos = id_hdd.find("FOXLINE FLSSD256X5");
    if (pos != std::string::npos) {
        id_hdd_size = "SSD 256Gb";
        return true;
        }

    pos = id_hdd.find("TOSHIBA DT01ACA100");
    if (pos != std::string::npos) {
        id_hdd_size = "HDD 1000Gb";
        return true;
        }

    pos = id_hdd.find("ST500NM0011");
    if (pos != std::string::npos) {
        id_hdd_size = "HDD 500Gb";
        return true;
        }

    pos = id_hdd.find("KINGSTON SA400S37480G");
    if (pos != std::string::npos) {
        id_hdd_size = "SSD 480Gb";
        return true;
        }

for (const auto& disk : disks) {
        try {
            uint64_t bytes = get_disk_size_bytes(disk);
            std::string type = get_disk_type(disk);

            int size_rounded_gb = round_to_10_decimal_gb(bytes);

            // Формируем понятную строку: "SSD 500Gb" или "HDD 1000Gb"
            id_hdd_size = type + " " + std::to_string(size_rounded_gb) + "Gb";
            return true;
        } catch (const std::exception&) {
            // Если этот диск недоступен — пробуем следующий
            continue;
        }
    }

    id_hdd_size = "Unknown 0Gb";
    return false;
}
```

###### 23/07/2026
[~] переделал получение производителя жесткого диска
```
#include "main.h"
/**************************************************************************************************/
/* returns the hard drive type (SSD/HDD) and size, and if the disk brand is known, both the type  */
/* and size are taken from the disk name                                                          */
/*------------------------------------------------------------------------------------------------*/
/* getting the name of the hard drive, then you can add the manufacturer, known brands            */
/* listed, substituted according to the factory specification                                     */
/* the size is taken from the name of the drive                                                   */
/**************************************************************************************************/

/*-------------------------------------------------------------------------------------------------*/
/*                  модуль получения производителя жесткого диска и название модели                */

bool TInventory::get_hdd( ) {

 char buff[50] = "0";

 FILE *pipein = popen("lsblk -o MODEL -d", "r");
 fread( buff, 50, 50 , pipein);
 replace(buff, '\n', ' ');


 id_hdd = buff;
 id_hdd.erase( 0, 6);

 size_t pos = id_hdd.find("DT01ACA100");
    if (pos != std::string::npos) {
        id_hdd = "TOSHIBA DT01ACA100";
        return true;
        }


    // вырезаем все ненужное
    //string toRemove = "COMPUTER INC.  ";
    //size_t pos = id_mb.find(0x0A);
    //if (pos != std::string::npos) {
    //    id_mb.erase(pos);
    //}

 return true;
}
```

###### 08-07-2026
[~] переделал все, что касается цветов \
[~] дополнил вывод ошибочных данных красным цветом, правильных - ярко белым \
[+] добавился файл **colors.h**
[+] теперь дополнился определением принтера по умолчанию (работает!!!)

###### 09-06-2026
[~] переработан алгоритм определения CPU (теперь не зависит от версии linux) \  
[~] переработан метод получения общего объема оперативной памяти
    realise 3.2.-4


###### 08-06-2026
[x] все работает, модифицировал отправку файла - теперь если на первый ftp-server (86.110.167.86) происходит ошибка
    теперь будет пытаться закачать на второй сервак (denkin.ru)

