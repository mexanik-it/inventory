// file: include/txhx_paths.cpp

#include "txhx_paths.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

bool validate_local_path(const std::string& path) {
    try {
        fs::path p(path);
        if (!fs::exists(p)) {
            std::cerr << "[ERROR] Local path does not exist: " << path << "\n";
            return false;
        }
        if (!fs::is_directory(p)) {
            std::cerr << "[ERROR] Local path is not a directory: " << path << "\n";
            return false;
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ERROR] Filesystem error checking local path: " << e.what() << "\n";
        return false;
    }
}

bool validate_remote_path(const std::string& path, bool create_if_missing) {
    try {
        fs::path p(path);
        if (fs::exists(p) && fs::is_directory(p)) {
            return true;
        }
        if (create_if_missing) {
            if (fs::create_directories(p)) {
                std::cout << "[INFO] Created remote directory: " << path << "\n";
                return true;
            }
        }
        std::cerr << "[ERROR] Remote path does not exist and cannot be created: " << path << "\n";
        return false;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ERROR] Filesystem error checking remote path: " << e.what() << "\n";
        return false;
    }
}

std::string make_backup_dirname() {
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

fs::path make_backup_dir_path() {
    return fs::current_path() / "backup" / make_backup_dirname();
}
