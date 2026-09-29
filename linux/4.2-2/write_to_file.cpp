// write_to_file.cpp

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

// Экранирует спецсимволы XML. DMI-поля приходят как есть, и амперсанд в
// названии модели ("ATX-B&O") иначе обрывает документ на середине.
static std::string xmlEscape(const std::string& s) {
    std::string r;
    r.reserve(s.size() + 8);

    for (char c : s) {
        switch (c) {
            case '&':  r += "&amp;";  break;
            case '<':  r += "&lt;";   break;
            case '>':  r += "&gt;";   break;
            case '"':  r += "&quot;"; break;
            case '\'': r += "&apos;"; break;
            default:   r += c;        break;
        }
    }
    return r;
}

bool TInventory::write_to_file() {
    // Проверка наличия имени файла перед открытием
    if (id_filename.empty()) {
        cerr << "Ошибка: имя файла для записи не задано." << endl;
        return false;
    }

    ofstream out(id_filename);

    if (!out.is_open()) {
        cerr << "Не удалось открыть файл \"" << id_filename << "\" для записи данных." << endl;
        return false;
    }

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;   // единственная существующая версия
    out << "<inventory>" << endl;

    out << "\t<id_date>"          << xmlEscape(id_date)          << "</id_date>"          << endl
        << "\t<id_mb>"           << xmlEscape(mbIdString(mb))    << "</id_mb>"            << endl
//        << "\t<mb_part>"         << xmlEscape(mb.part.empty() ? "unknown" : mb.part)  << "</mb_part>"  << endl
//        << "\t<mb_serial>"       << xmlEscape(mb.serial)         << "</mb_serial>"        << endl
        << "\t<id_cpu>"          << xmlEscape(id_cpu)            << "</id_cpu>"           << endl
        << "\t<id_mem>"          << xmlEscape(id_mem)            << "</id_mem>"           << endl
        << "\t<id_mem_name>"     << xmlEscape(id_mem_name)       << "</id_mem_name>"      << endl
        << "\t<sn_mem>"          << xmlEscape(sn_mem)            << "</sn_mem>"           << endl
        << "\t<id_hdd>"          << xmlEscape(id_hdd)            << "</id_hdd>"           << endl
        << "\t<id_hdd_size>"     << xmlEscape(id_hdd_size)       << "</id_hdd_size>"      << endl
        << "\t<id_sys>"          << xmlEscape(id_sys)            << "</id_sys>"           << endl
        << "\t<id_prn>"          << xmlEscape(id_prn)            << "</id_prn>"           << endl
        << "\t<id_host>"         << xmlEscape(id_host)           << "</id_host>"          << endl
        << "\t<id_ip>"           << xmlEscape(id_ip)             << "</id_ip>"            << endl
        << "\t<id_mac>"          << xmlEscape(id_mac)            << "</id_mac>"           << endl
        << "\t<id_office>"       << xmlEscape(id_office)         << "</id_office>"        << endl
        << "\t<id_structure>"    << xmlEscape(id_structure)      << "</id_structure>"     << endl
        << "\t<id_inv_number>"   << xmlEscape(id_inv_number)     << "</id_inv_number>"    << endl

	    << "\t\t<other_details>"  << endl
	      << "\t\t  <mb_part>"    << xmlEscape(mb.part)      << "</mb_part>"    << endl
	      << "\t\t  <mb_serial>"  << xmlEscape(mb.serial)    << "</mb_serial>"    << endl
	    << "\t\t</other_details>" << endl

	<< "</inventory>" << endl;

    // close() вызывать необязательно, деструктор сделает это сам,
    // но явное закрытие позволяет отловить ошибку записи на диск до возврата bool.
    out.close();

    if (out.fail()) {
        cerr << "Ошибка физической записи на диск." << endl;
        return false;
    }

    return true;
}

