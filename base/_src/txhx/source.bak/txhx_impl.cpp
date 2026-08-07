#include "txhx.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <vector>
#include <chrono>
#include <thread>
#include <windows.h>

namespace fs = std::filesystem;

// Объявления функций окна прогресса
extern "C" void txhx_show_progress_window(const wchar_t* title);
extern "C" void txhx_close_progress_window();
extern "C" void txhx_set_progress_percent(int percent);
extern "C" bool txhx_is_cancelled();

TXHX::TXHX() {
    fs::create_directories(m_backup_root);
}

bool TXHX::init(const std::string& remote_path, const std::string& local_dir) {
    m_remote_path = remote_path;
    m_local_dir   = local_dir;

    if (!fs::exists(remote_path)) {
        std::cerr << "[ERROR] Remote path does not exist or not accessible: " << remote_path << "\n";
        return false;
    }
    if (!fs::exists(local_dir)) {
        std::cerr << "[ERROR] Local directory does not exist: " << local_dir << "\n";
        return false;
    }

    return true;
}

std::string TXHX::make_backup_dirname() const {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::setfill('0')
        << tm.tm_year + 1900 << "-"
        << std::setw(2) << tm.tm_mon + 1 << "-"
        << std::setw(2) << tm.tm_mday << "_"
        << std::setw(2) << tm.tm_hour << "-"
        << std::setw(2) << tm.tm_min << "-"
        << std::setw(2) << tm.tm_sec;
    return oss.str();
}

bool TXHX::copy_file(const fs::path& src, const fs::path& dst) {
    try {
        fs::create_directories(dst.parent_path());
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        return true;
    } catch (...) {
        return false;
    }
}

bool TXHX::copy_directory(const fs::path& src, const fs::path& dst) {
    try {
        fs::create_directories(dst);
        for (const auto& entry : fs::recursive_directory_iterator(
                 src, fs::directory_options::skip_permission_denied)) {
            if (txhx_is_cancelled()) {
                std::cerr << "Operation cancelled by user.\n";
                return false;
            }

            fs::path rel = fs::relative(entry.path(), src);
            fs::path dst_entry = dst / rel;
            if (entry.is_directory()) {
                fs::create_directories(dst_entry);
            } else {
                fs::copy_file(entry.path(), dst_entry, fs::copy_options::overwrite_existing);
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

void TXHX::update_report(const std::string& message) {
    std::ofstream ofs("report.html", std::ios::app | std::ios::out);
    if (!ofs) return;

    ofs.seekp(0, std::ios::end);
    if (ofs.tellp() == 0) {
        ofs << "<html><head><meta charset=\"UTF-8\"><title>TXHX Report</title></head><body>\n";
        ofs << "<h1>TXHX Backup Report</h1>\n<ul>\n";
    }
    ofs << "<li>" << message << "</li>\n";
}

void TXHX::show_progress_window(const wchar_t* title) {
    txhx_show_progress_window(title);
}

void TXHX::close_progress_window() {
    txhx_close_progress_window();
}

void TXHX::set_progress_percent(int percent) {
    txhx_set_progress_percent(percent);
}

// Заглушки для restore и compress, чтобы компиляция прошла
bool TXHX::restore() {
    std::cerr << "Restore: не реализовано в этой версии\n";
    return false;
}

bool TXHX::compress() {
    std::cerr << "Compress: не реализовано в этой версии\n";
    return false;
}
