#include "txhx_report.hpp"
#include <fstream>
#include <sstream>

void init_report_file(const std::string& report_path) {
    std::ifstream f(report_path);
    if (f.good()) return; // уже есть
    f.close();

    std::ofstream ofs(report_path);
    ofs << "<html><head><meta charset=\"UTF-8\"><title>TXHX Report</title></head><body>\n";
    ofs << "<h1>TXHX Backup/Restore Report</h1>\n";
}

void append_report_line(const std::string& report_path, const std::string& line) {
    std::ofstream ofs(report_path, std::ios::app);
    if (!ofs) return;
    ofs << line << "\n";
}
