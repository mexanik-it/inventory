// file: source/txhx_backup.cpp
#include "txhx_backup.hpp"
#include "txhx_paths.hpp"
#include "txhx_report.hpp"
#include "txhx_ui.hpp"

#include <thread>
#include <atomic>
#include <algorithm>
#include <iostream>      // <-- ЭТОГО НЕ ХВАТАЛО для std::cerr
#include <chrono>

namespace fs = std::filesystem;

static std::atomic<bool> g_cancel_requested(false);

// Функция estimate_eta (оставляем как была)
static double estimate_eta(size_t done_files, size_t total_files, double elapsed_seconds) {
    if (done_files == 0 || total_files == 0) return 0.0;
    double avg_time = elapsed_seconds / static_cast<double>(done_files);
    size_t remaining = total_files - done_files;
    return avg_time * static_cast<double>(remaining);
}

bool perform_backup(TXHX& txhx, BackupStats& stats) {
    g_cancel_requested.store(false);

    // --- ИСПРАВЛЕНИЕ: Используем публичные геттеры вместо приватных полей ---
    const auto& remote = txhx.get_remote_path();
    const auto& local = txhx.get_local_dir();
    // ----------------------------------------------------------------------

    fs::path backup_dir = make_backup_dir_path();
    
    // Создаем директорию бэкапа, если нет
    try {
        if (!fs::exists(backup_dir)) {
            fs::create_directories(backup_dir);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[CRITICAL] Cannot create backup directory: " << e.what() << "\n";
        return false;
    }

    std::vector<fs::path> local_files;
    try {
        for (const auto& entry : fs::directory_iterator(local)) {
            if (entry.is_regular_file()) {
                local_files.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[CRITICAL] Error reading local directory: " << e.what() << "\n";
        return false;
    }

    stats.total_files = local_files.size();
    stats.copied_files = 0;
    stats.skipped_files = 0;
    stats.total_bytes = 0; // можно посчитать заранее, но пока так

    auto start_time = std::chrono::steady_clock::now();

    ui_start_progress(L"Backup");

    size_t total_count = stats.total_files;
    if (total_count == 0) total_count = 1; // Защита от деления на ноль

    for (size_t i = 0; i < local_files.size(); ++i) {
        if (g_cancel_requested.load()) {
            // --- ИСПРАВЛЕНО: теперь std::cerr работает ---
            std::cerr << "\n[WARN] Backup cancelled by user.\n";
            break;
        }

        const auto& src = local_files[i];
        fs::path dst_local = backup_dir / src.filename();
        
        // Формируем путь на удаленном ресурсе
        // Внимание: логика объединения путей может зависеть от формата remote (UNC или папка)
        fs::path dst_remote = fs::path(remote) / src.filename();

        bool ok = true;

        // 1. Копируем в локальную бэкап-папку
        try {
            fs::copy_file(src, dst_local, fs::copy_options::overwrite_existing);
            stats.copied_bytes += fs::file_size(src);
            stats.copied_files++;
        } catch (...) {
            stats.skipped_files++;
            ok = false;
        }

        // 2. Копируем на удаленный ресурс (если путь валиден и есть права)
        try {
            if (fs::exists(dst_remote)) {
                fs::remove(dst_remote);
            }
            fs::copy_file(src, dst_remote, fs::copy_options::overwrite_existing);
        } catch (...) {
            stats.skipped_files++;
            ok = false;
        }

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();

        int percent = static_cast<int>((i + 1) * 100 / total_count);
        double eta = estimate_eta(i + 1, stats.total_files, elapsed);

        std::string status = ok ? "OK" : "WARN";
        ui_update_progress(percent, status + ": " + src.filename().string(), eta);
    }

    ui_finish_progress();

    return stats.copied_files > 0 || stats.skipped_files == 0; 
}
