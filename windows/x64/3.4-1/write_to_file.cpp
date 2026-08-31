#include "main.h"
#include <fstream>
#include <iostream>

bool TInventory::write_to_file() {
    // Открываем в binary: никаких авто‑конвертаций \r\n и порчи байтов
    std::ofstream out(id_filename, std::ios::out | std::ios::binary);

    if (!out.is_open()) {
        std::cerr << "Не удалось открыть файл для записи данных." << std::endl;
        return false;
    }

    // Пишем XML с явными \n, без endl (чтобы не было лишних flush и платформенных различий)
    out << "<?xml version='1.0' encoding='UTF-8'?>\n"
          << "  <inventory>\n"
          << "\t<id_date>"      << id_date       << "</id_date>\n"
          << "\t<id_mb>"        << id_mb         << "</id_mb>\n"
          << "\t<id_cpu>"       << id_cpu        << "</id_cpu>\n"
          << "\t<id_mem>"       << id_mem        << "</id_mem>\n"
          << "\t<id_hdd>"       << id_hdd        << "</id_hdd>\n"
          << "\t<id_hdd_size>"  << id_hdd_size   << "</id_hdd_size>\n"
          << "\t<id_sys>"       << id_sys        << "</id_sys>\n"
          << "\t<id_prn>"       << id_prn        << "</id_prn>\n"
          << "\t<id_host>"      << id_host       << "</id_host>\n"
          << "\t<id_ip>"        << id_ip         << "</id_ip>\n"
          << "\t<id_mac>"       << id_mac        << "</id_mac>\n"
          << "\t<id_office>"    << id_office     << "</id_office>\n"
          << "\t<id_structure>" << id_structure  << "</id_structure>\n\n"
          << "  </inventory>\n";

    // close() вызывается автоматически в деструкторе, но можно оставить для ясности
    out.close();

    return true;
}
