// file: source/txhx_backup.cpp
#include "txhx_backup.hpp"
#include "txhx_paths.hpp"
#include "txhx_report.hpp"
#include "txhx_ui.hpp"

#include <thread>
#include <atomic>
#include <algorithm>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;

static std::atomic<bool> g_cancel_requested(false);

static double estimate_eta(size_t done_files, size_t total_files, double elapsed_seconds) {
    if (done_files == 0 || total_files == 0) return 0.0;
    double avg_time = elapsed_seconds / static_cast<double>(done_files);
    size_t remaining = total_files - done_files;
    return avg_time * static_cast<double>(remaining);
}

bool perform_backup(TXHX& txhx, BackupStats& stats) {
    g_cancel_requested.store(false);

    const auto& source_path = txhx.get_remote_path();
    fs::path backup_dir = make_backup_dir_path();

    try {
        if (!fs::exists(backup_dir)) {
            fs::create_directories(backup_dir);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[CRITICAL] Cannot create backup directory: " << e.what() << "\n";
        return false;
    }

    std::vector<fs::path> source_files;
    try {
        for (const auto& entry : fs::directory_iterator(source_path)) {
            if (entry.is_regular_file()) {
                source_files.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[CRITICAL] Error reading remote directory: " << e.what() << "\n";
        return false;
    }

    stats.total_files = source_files.size();
    stats.copied_files = 0;
    stats.skipped_files = 0;
    stats.total_bytes = 0;

    auto start_time = std::chrono::steady_clock::now();
    ui_start_progress(L"Backup");

    size_t total_count = stats.total_files;
    if (total_count == 0) total_count = 1;

    for (size_t i = 0; i < source_files.size(); ++i) {
        if (g_cancel_requested.load()) {
            std::cerr << "\n[WARN] Backup cancelled by user.\n";
            break;
        }

        const auto& src = source_files[i];
        fs::path dst_local = backup_dir / src.filename();

        bool ok = true;

        // 1. Копируем
        try {
            fs::copy_file(src, dst_local, fs::copy_options::overwrite_existing);
        } catch (const fs::filesystem_error& e) {
            stats.skipped_files++;
            ok = false;
            std::cerr << "[ERROR] Cannot copy file " << src.string() << " -> " << dst_local.string() << ": " << e.what() << "\n";
        }

        // 2. Получаем размер из локальной копии — с явным выводом ошибки
        if (ok) {
            // Отладочный вывод: покажем, какой именно путь мы проверяем
            std::cout << "[DEBUG] Checking size for local file: " << dst_local.string() << "\n";

            try {
                uintmax_t size = fs::file_size(dst_local);
                stats.copied_bytes += size;
                stats.copied_files++;

                std::cout << "[DEBUG] Size = " << size << " bytes\n";
            } catch (const fs::filesystem_error& e) {
                // Теперь эта ошибка будет видна в консоли!
                std::cerr << "[ERROR] Cannot get file size for: " << dst_local.string() << ": " << e.what() << "\n";
                stats.copied_files++; // файл есть, но размер не смогли получить
            }
            // Убрали catch (...) — теперь любая ошибка файловой системы будет видна
        }

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();
        int percent = static_cast<int>((i + 1) * 100 / total_count);
        double eta = estimate_eta(i + 1, stats.total_files, elapsed);

        std::string status = ok ? "OK" : "WARN";
        ui_update_progress(percent, status + ": " + src.filename().string(), eta);
    }

    ui_finish_progress();

    return (stats.copied_files > 0) || (stats.total_files == 0);
}
