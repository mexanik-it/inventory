// write_to_file.cpp
// -----------------

/**************************************************************************************************/
/* write_to_file.cpp                                                                              */
/* запись данных в XML файл                                                                       */
/**************************************************************************************************/

// write_to_file.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include "main.h"

using std::cerr;
using std::endl;
using std::ofstream;

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
    if (id_filename.empty()) {
        cerr << "Ошибка: имя файла для записи не задано." << endl;
        return false;
    }

    ofstream out(id_filename);
    if (!out.is_open()) {
        cerr << "Не удалось открыть файл \"" << id_filename << "\" для записи данных." << endl;
        return false;
    }

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    out << "<inventory>" << endl;

    out << "\t<id_date>"          << xmlEscape(id_date)          << "</id_date>"          << endl
        << "\t<id_mb>"           << xmlEscape(mbIdString(mb))    << "</id_mb>"            << endl
        << "\t<id_cpu>"          << xmlEscape(id_cpu)            << "</id_cpu>"           << endl
        << "\t<id_mem>"          << xmlEscape(id_mem)            << "</id_mem>"           << endl
        << "\t<id_mem_name>"     << xmlEscape(id_mem_name)       << "</id_mem_name>"      << endl
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

        << "\t<other_details>" << endl
          << "\t\t<mb_part>"   << xmlEscape(mb.part.empty()   ? "unknown" : mb.part)   << "</mb_part>"   << endl
          << "\t\t<mb_serial>" << xmlEscape(mb.serial.empty() ? "unknown" : mb.serial) << "</mb_serial>" << endl;

    // --- Перечисляем все модули памяти ---
    if (memModules.empty()) {
        out << "\t\t<mem_module>" << endl
            << "\t\t\t<vendor>unknown</vendor>" << endl
            << "\t\t\t<part>unknown</part>" << endl
            << "\t\t\t<size>unknown</size>" << endl
            << "\t\t\t<serial>unknown</serial>" << endl
            << "\t\t</mem_module>" << endl;
    } else {
        for (size_t i = 0; i < memModules.size(); ++i) {
            const auto& m = memModules[i];

            // vendor
            std::string vendor = (m.vendor.empty() || m.vendor == "Not Specified")
                                 ? "unknown" : m.vendor;
            // part
            std::string part = (m.part.empty() || m.part == "Not Specified")
                               ? "unknown" : m.part;
            // size
            std::string sizeStr;
            if (m.capacityBytes > 0) {
                double gb = static_cast<double>(m.capacityBytes) / 1'000'000'000.0;
                std::ostringstream ss;
                ss << static_cast<int>(std::round(gb)) << "Gb";
                sizeStr = ss.str();
            } else {
                sizeStr = "unknown";
            }
            // serial
            std::string serial = (m.serial.empty() || m.serial == "Not Specified")
                                 ? "unknown" : m.serial;

            out << "\t\t<mem_module>" << endl
                << "\t\t\t<vendor>"  << xmlEscape(vendor)  << "</vendor>"  << endl
                << "\t\t\t<part>"   << xmlEscape(part)    << "</part>"   << endl
                << "\t\t\t<size>"   << xmlEscape(sizeStr) << "</size>"   << endl
                << "\t\t\t<serial>" << xmlEscape(serial)  << "</serial>" << endl
                << "\t\t</mem_module>" << endl;
        }
    }

    out << "\t</other_details>" << endl
        << "</inventory>" << endl;

    out.close();
    if (out.fail()) {
        cerr << "Ошибка физической записи на диск." << endl;
        return false;
    }

    return true;
}
