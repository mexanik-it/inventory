// file: include/backup_stats.hpp
#pragma once
#include <cstdint>

struct BackupStats {
    uint64_t total_files = 0;
    uint64_t copied_files = 0;
    uint64_t skipped_files = 0;
    uint64_t total_bytes = 0;
    uint64_t copied_bytes = 0;
};
