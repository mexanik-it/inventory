#pragma once
#include <cstdint>
#include <string>
#include "txhx_core.hpp"

struct RestoreStats {
    size_t total_files      = 0;
    size_t restored_files   = 0;
    size_t failed_files     = 0;
};

bool perform_restore(TXHX& txhx, RestoreStats& stats);
