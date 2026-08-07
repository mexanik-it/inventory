// source/txhx_backup.cpp
#include "txhx.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <vector>
#include <chrono>
#include <thread>
#include <windows.h>

namespace fs = std::filesystem;

// Объявления функций окна прогресса (реализованы в progress_window.cpp)
extern "C" void txhx_show_progress_window(const wchar_t* title);
extern "C" void txhx_close_progress_window();
extern "C" void txhx_set_progress_percent(int percent);
extern "C" bool txhx_is_cancelled();

bool TXHX::backup() {
    std::ofstream debug("debug_backup.log", std::ios::trunc);
    debug << "=== TXHX::backup started ===\n";

    // Создаём папку бэкапа с временной меткой
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", std::localtime(&time));

    fs::path local_backup_dir = fs::current_path() / "backup" / std::string(buf);

    if (!fs::exists(m_local_dir) || !fs::is_directory(m_local_dir)) {
        debug << "[CRITICAL] Invalid local dir: " << m_local_dir << "\n";
        std::cerr << "[CRITICAL] Local directory does not exist or is not a directory.\n";
        return false;
    }

    if (!fs::exists(m_remote_path)) {
        debug << "[CRITICAL] Remote path does not exist: " << m_remote_path << "\n";
        std::cerr << "[CRITICAL] Remote path does not exist.\n";
        return false;
    }

    debug << "[INFO] Local dir: " << m_local_dir << "\n";
    debug << "[INFO] Remote dir: " << m_remote_path << "\n";

    // Собираем список файлов для копирования
    std::vector<fs::path> files_to_copy;
    uint64_t total_file_size = 0;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(
                 m_local_dir,
                 fs::directory_options::skip_permission_denied | fs::directory_options::follow_directory_symlink)) {
            if (entry.is_regular_file()) {
                uint64_t sz = entry.file_size();
                files_to_copy.push_back(entry.path());
                total_file_size += sz;
                debug << "[INFO] To copy: " << entry.path().string() << " (" << sz << " bytes)\n";
            }
        }
    } catch (const std::exception& e) {
        debug << "[CRITICAL] Cannot list files/folders: " << e.what() << "\n";
        std::cerr << "[CRITICAL] Cannot list local files/folders.\n";
        return false;
    }

    debug << "[INFO] Total files: " << files_to_copy.size()
          << ", total size: " << total_file_size << " bytes\n";

    if (files_to_copy.empty()) {
        debug << "[WARNING] No files to copy.\n";
        std::cout << "[INFO] No files to backup.\n";
        fs::create_directories(local_backup_dir);
        update_report("Backup: no files found");
        return true;
    }

    // Общая работа: копирование локально + копирование на удалённый путь
    uint64_t total_work_size = total_file_size * 2;
    int bar_width = 40;
    uint64_t copied_size = 0;
    size_t files_done = 0;

    auto start_time = std::chrono::steady_clock::now();

    // Вспомогательная функция для форматирования ETA
    auto format_eta = [](double seconds) -> std::string {
        int mins = static_cast<int>(seconds / 60.0);
        int secs = static_cast<int>(std::fmod(seconds, 60.0));
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << mins << ":"
            << std::setw(2) << std::setfill('0') << secs;
        return oss.str();
    };

    // Вывод прогресс-бара в консоль
    auto print_progress = [&](uint64_t current, size_t files_processed) {
        auto now_time = std::chrono::steady_clock::now();
        double elapsed_sec = std::chrono::duration<double>(now_time - start_time).count();

        double speed_mb = (elapsed_sec > 0.001)
                          ? (current / 1024.0 / 1024.0) / elapsed_sec
                          : 0.0;

        double eta_sec = 0.0;
        if (speed_mb > 0.0001 && current < total_work_size) {
            double remaining_mb = (total_work_size - current) / 1024.0 / 1024.0;
            eta_sec = remaining_mb / speed_mb;
        }

        int pos = static_cast<int>(bar_width * (static_cast<double>(current) / total_work_size));

        std::cout << "\r[";
        for (int k = 0; k < bar_width; ++k) {
            if (k < pos) std::cout << "=";
            else if (k == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] "
                  << std::fixed << std::setprecision(1)
                  << (100.0 * current / total_work_size) << "% ("
                  << (current / 1024.0 / 1024.0) << "/"
                  << (total_work_size / 1024.0 / 1024.0) << " MB) | "
                  << "Speed: " << std::fixed << std::setprecision(2) << speed_mb << " MB/s | "
                  << "ETA: " << format_eta(eta_sec) << " | "
                  << "Copied: " << files_processed << "/" << files_to_copy.size() << " files";
        std::cout.flush();
    };

    // Показываем окно прогресса
    show_progress_window(L"TXHX Backup");

    std::cout << "Backup progress: ";

    // --- Этап 1: локальное копирование (в папку backup/YYYY-MM-DD_HH-MM-SS) ---
    for (const auto& src : files_to_copy) {
        if (txhx_is_cancelled()) {
            std::cerr << "\nBackup cancelled by user.\n";
            debug << "[CANCELLED] Backup stopped by user at local copy stage.\n";
            close_progress_window();
            return false;
        }

        fs::path dst = local_backup_dir / fs::relative(src, m_local_dir);
        fs::create_directories(dst.parent_path());

        if (fs::exists(dst)) {
            fs::remove(dst);
            debug << "[INFO] Removed old local backup file: " << dst.filename().string() << "\n";
        }

        if (!copy_file(src, dst)) {
            debug << "[ERROR] Failed to copy local file: " << src.string() << " -> " << dst.string() << "\n";
            std::cerr << "Error copying file (local): " << src.filename().string() << "\n";
            close_progress_window();
            return false;
        }

        uint64_t file_size = fs::file_size(src);
        copied_size += file_size;
        files_done++;

        print_progress(copied_size, files_done);
        set_progress_percent(static_cast<int>((100 * copied_size) / total_work_size));

        debug << "[OK] Local: " << src.string() << " -> " << dst.string() << "\n";
    }

    // --- Этап 2: сетевое копирование (на m_remote_path) ---
    for (const auto& src : files_to_copy) {
        if (txhx_is_cancelled()) {
            std::cerr << "\nBackup cancelled by user.\n";
            debug << "[CANCELLED] Backup stopped by user at remote copy stage.\n";
            close_progress_window();
            return false;
        }

        fs::path dst = fs::path(m_remote_path) / fs::relative(src, m_local_dir);
        fs::create_directories(dst.parent_path());

        if (fs::exists(dst)) {
            fs::remove(dst);
            debug << "[INFO] Removed old remote file: " << dst.filename().string() << "\n";
        }

        if (!copy_file(src, dst)) {
            debug << "[ERROR] Failed to copy remote file: " << src.string() << " -> " << dst.string() << "\n";
            std::cerr << "Error copying file (remote): " << src.filename().string() << "\n";
            close_progress_window();
            return false;
        }

        uint64_t file_size = fs::file_size(src);
        copied_size += file_size;
        files_done++;

        print_progress(copied_size, files_done);
        set_progress_percent(static_cast<int>((100 * copied_size) / total_work_size));

        debug << "[OK] Remote: " << src.string() << " -> " << dst.string() << "\n";
    }

    close_progress_window();

    std::cout << "\nBackup completed successfully.\n";
    update_report("Backup completed: " + local_backup_dir.string());
    return true;
}
