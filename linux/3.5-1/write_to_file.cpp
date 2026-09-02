/**************************************************************************************************/
/* write_to_file.cpp                                                                              */ 
/* запись данных в XML файл                                                                       */
/**************************************************************************************************/
#include <iostream>
#include <fstream>
#include <string>
#include "main.h" // Подключаем объявление класса TInventory

// Явное использование пространств имен только для .cpp файла
using std::cerr;
using std::endl;
using std::ofstream;

bool TInventory::write_to_file() {
    // Проверка наличия имени файла перед открытием
    if (id_filename.empty()) {
        cerr << "Ошибка: имя файла для записи не задано." << endl;
        return false;
    }

    // Создаём объект ofstream и открываем файл для записи
    ofstream out(id_filename);

    // Проверяем, успешно ли открыт файл
    if (!out.is_open()) {
        cerr << "Не удалось открыть файл \"" << id_filename << "\" для записи данных." << endl;
        return false;
    }

    /* 
       АРХИТЕКТУРНЫЙ СОВЕТ:
       Вместо жесткой вставки тегов здесь, лучше создать отдельный класс XmlWriter.
       Сейчас данные пишутся напрямую через оператор <<. При изменении структуры Inventory
       этот метод станет сложно поддерживать.
    */

    // Записываем данные в файл
    out << "<?xml version='3.5-1' encoding='UTF-8'?>" << endl; // Заголовок
    out << "<inventory>" << endl;
    
    // Для красоты отступов используем R"(...)" (Raw string literal) — это удобнее для многострочного текста,
    // но пока оставим текущий вид для совместимости с вашими переменными.
    out << "\t<id_date>"      << id_date       << "</id_date>"          << endl
        << "\t<id_mb>"        << id_mb         << "</id_mb>"            << endl
        << "\t<id_cpu>"       << id_cpu        << "</id_cpu>"           << endl
        << "\t<id_mem>"       << id_mem        << "</id_mem>"           << endl
        << "\t<id_hdd>"       << id_hdd        << "</id_hdd>"           << endl
        << "\t<id_hdd_size>"  << id_hdd_size   << "</id_hdd_size>"      << endl
        << "\t<id_sys>"       << id_sys        << "</id_sys>"           << endl
        << "\t<id_prn>"       << id_prn        << "</id_prn>"           << endl
        << "\t<id_host>"      << id_host       << "</id_host>"          << endl
        << "\t<id_ip>"        << id_ip         << "</id_ip>"            << endl
        << "\t<id_mac>"       << id_mac        << "</id_mac>"           << endl
        << "\t<id_office>"    << id_office     << "</id_office>"        << endl
        << "\t<id_structure>" << id_structure  << "</id_structure>"     << endl
        << "\t<id_inv_number>" << id_inv_number  << "</id_inv_number>"  << endl;
        
    out << "</inventory>" << endl;

    // close() вызывать необязательно, деструктор сделает это сам при выходе из функции,
    // но явное закрытие помогает отловить ошибки записи диска до возврата bool.
    out.close();

    // Дополнительная проверка: действительно ли данные ушли на диск?
    if (out.fail()) {
        cerr << "Ошибка физической записи на диск." << endl;
        return false;
    }

    return true;
}