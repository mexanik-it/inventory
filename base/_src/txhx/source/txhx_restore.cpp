// file: source/txhx_restore.cpp
#include "txhx_restore.hpp"
#include "txhx_paths.hpp"
#include "txhx_ui.hpp"

#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>

namespace fs = std::filesystem;

// -----------------------------------------------------------------------------
// Вспомогательная функция: оценка времени (ETA)
// Должна быть объявлена ДО использования в цикле
// -----------------------------------------------------------------------------
static double estimate_eta(size_t done, size_t total, double elapsed_sec) {
    if (done == 0 || total == 0) {
        return 0.0;
    }
    double avg_time_per_file = elapsed_sec / static_cast<double>(done);
    size_t remaining = total - done;
    return avg_time_per_file * static_cast<double>(remaining);
}

bool perform_restore(TXHX& txhx, RestoreStats& stats) {
    // Получаем локальную директорию через геттер
    const auto& local_dir = txhx.get_local_dir();
    // remote_path не нужен для restore, поэтому не берём его — убираем предупреждение

    fs::path backup_dir = make_backup_dir_path();

    if (!fs::exists(backup_dir)) {
        std::cerr << "[CRITICAL] Backup directory not found: " << backup_dir << "\n";
        return false;
    }

    std::vector<fs::path> backup_files;
    try {
        for (const auto& entry : fs::directory_iterator(backup_dir)) {
            if (entry.is_regular_file()) {
                backup_files.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[CRITICAL] Error scanning backup directory: " << e.what() << "\n";
        return false;
    }

    stats.total_files = backup_files.size();
    stats.restored_files = 0;
    stats.failed_files = 0;

    auto start_time = std::chrono::steady_clock::now();
    ui_start_progress(L"Restore");

    size_t denom = (stats.total_files == 0) ? 1 : stats.total_files;

    for (size_t i = 0; i < backup_files.size(); ++i) {
        const auto& src = backup_files[i];
        fs::path dst = fs::path(local_dir) / src.filename();

        bool ok = true;
        try {
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
            stats.restored_files++;
        } catch (...) {
            stats.failed_files++;
            ok = false;
        }

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();

        int percent = static_cast<int>((i + 1) * 100 / denom);
        double eta = estimate_eta(i + 1, stats.total_files, elapsed);

        std::string status = ok ? "OK" : "FAIL";
        ui_update_progress(percent, status + ": " + src.filename().string(), eta);
    }

    ui_finish_progress();

    return (stats.restored_files > 0) || (stats.total_files == 0);
}
