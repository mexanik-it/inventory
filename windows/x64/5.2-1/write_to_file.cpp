#include "main.h"
#include <fstream>
#include <iostream>

bool TInventory::write_to_file() {
    // Открываем в binary: никаких автоконвертаций \r\n и порчи байтов
    std::ofstream out(id_filename, std::ios::out | std::ios::binary);

    if (!out.is_open()) {
        errMessage("Не удалось открыть файл для записи: " + id_filename);
        return false;
    }

    // Пишем XML с явными \n, без endl
    out << "<?xml version='1.0' encoding='UTF-8'?>\n"
        << "<!-- inventory v5.2-1 -->\n"
        << "  <inventory>\n"
        << "\t<id_date>"       << id_date        << "</id_date>\n"
        << "\t<id_mb>"         << id_mb          << "</id_mb>\n"
        << "\t<id_cpu>"        << id_cpu         << "</id_cpu>\n"
        << "\t<id_mem>"        << id_mem         << "</id_mem>\n"
        << "\t<id_hdd>"        << id_hdd         << "</id_hdd>\n"
        << "\t<id_hdd_size>"   << id_hdd_size    << "</id_hdd_size>\n"
        << "\t<id_sys>"        << id_sys         << "</id_sys>\n"
        << "\t<id_prn>"        << id_prn         << "</id_prn>\n"
        << "\t<id_host>"       << id_host        << "</id_host>\n"
        << "\t<id_ip>"         << id_ip          << "</id_ip>\n"
        << "\t<id_mac>"        << id_mac         << "</id_mac>\n"
        << "\t<id_office>"     << id_office      << "</id_office>\n"
        << "\t<id_structure>"  << id_structure   << "</id_structure>\n"
        << "\t<id_inv_number>" << id_inv_number  << "</id_inv_number>\n"
        << "  </inventory>\n";

    out.close();

    okMessage("Файл записан: " + id_filename);
    return true;
}
