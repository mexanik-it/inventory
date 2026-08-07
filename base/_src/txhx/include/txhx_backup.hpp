// file: include/txhx_backup.hpp
#pragma once

#include "txhx_core.hpp"
#include "backup_stats.hpp"  // <-- Подключаем структуру, но НЕ определяем её здесь

// Здесь НЕТ struct BackupStats { ... } — только использование

bool perform_backup(TXHX& txhx, BackupStats& stats);
