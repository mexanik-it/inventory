// file: source/txhx_core.cpp
#include "txhx_core.hpp"
#include "txhx_backup.hpp"
#include "txhx_restore.hpp"   // <-- Обязательно: чтобы RestoreStats был известен
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

bool TXHX::init(const std::string& remote_path, const std::string& local_dir) {
    m_remote_path = remote_path;
    m_local_dir = local_dir;

    std::cout << "[INFO] Checking local directory: " << m_local_dir << "\n";

    try {
        if (!fs::exists(m_local_dir)) {
            std::cout << "[INFO] Directory not found, creating: " << m_local_dir << "\n";
            if (fs::create_directories(m_local_dir)) {
                std::cout << "[OK] All parent directories created.\n";
            }
        } else if (!fs::is_directory(m_local_dir)) {
            std::cerr << "[CRITICAL] Path exists but is not a directory: " << m_local_dir << "\n";
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[CRITICAL] Filesystem error: " << e.what() << "\n";
        return false;
    }

    std::cout << "[OK] Init completed successfully.\n";
    return true;
}

BackupStats TXHX::backup() {
    BackupStats stats;
    bool ok = perform_backup(*this, stats);

    if (!ok) {
        std::cerr << "[ERROR] Backup failed.\n";
    } else {
        std::cout << "[INFO] Backup complete.\n";
        std::cout << "  Total files:      " << stats.total_files << "\n";
        std::cout << "  Copied:           " << stats.copied_files << "\n";
        std::cout << "  Skipped:          " << stats.skipped_files << "\n";
        std::cout << "  Total bytes:      " << stats.total_bytes << " B\n";
    }
    return stats;                 // <-- Возврат значения
}                                 // <-- ЭТОТ } КРИТИЧЕСКИ ВАЖЕН

bool TXHX::restore() {
    RestoreStats stats;
    bool ok = perform_restore(*this, stats);

    if (!ok) {
        std::cerr << "[ERROR] Restore failed.\n";
        return false;
    }

    std::cout << "[INFO] Restore complete.\n";
    std::cout << "  Total files:      " << stats.total_files << "\n";
    std::cout << "  Restored:         " << stats.restored_files << "\n";
    std::cout << "  Failed:           " << stats.failed_files << "\n";

    return true;
}

bool TXHX::compress() {
    std::cerr << "[WARNING] Compress functionality not implemented yet.\n";
    // TODO: Реализовать логику сжатия
    return false;
}
