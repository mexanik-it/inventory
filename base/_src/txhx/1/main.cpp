// file: source/main.cpp
#include "txhx_core.hpp"
#include "backup_stats.hpp"  // <-- ЭТА СТРОКА БЫЛА ПРОПУЩЕНА
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string remote_path;
    std::string local_dir;

    // Жёсткие пути для теста
    remote_path = "//10.67.1.36/oit_techbase/list-points";
    local_dir   = "C:/inventory/base/_src/txhx/backup";

    std::cout << "[INFO] Using paths:\n";
    std::cout << "  Remote: " << remote_path << "\n";
    std::cout << "  Local:  " << local_dir << "\n\n";

    TXHX txhx;
    if (!txhx.init(remote_path, local_dir)) {
        std::cerr << "[CRITICAL] Init failed.\n";
        return 1;
    }

    BackupStats stats = txhx.backup();  // Теперь эта строка сработает

    // Дальше остальной код...
    
    return 0;
}
