#include "txhx.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Чтобы русский текст в консоли отображался корректно
    SetConsoleOutputCP(CP_UTF8);
#endif

    TXHX txhx;

    std::string local_dir = ".";
    std::string remote_path = "";

    // Если передаём аргументы: txhx_tool.exe <remote> <local>
    if (argc >= 3) {
        remote_path = argv[1];
        local_dir   = argv[2];
    } else if (argc == 2) {
        // Если только один аргумент — считаем его локальной папкой
        local_dir = argv[1];
    }

    // Если удалённого пути нет — не пытаемся инициализировать с плохим дефолтом
    if (remote_path.empty()) {
        std::cerr << "Ошибка: не указан удалённый путь для бэкапа.\n";
        std::cerr << "Запуск: txhx_tool.exe \"<remote_path>\" \"<local_dir>\"\n";
        std::cerr << "Пример: txhx_tool.exe \"//10.67.1.36/oit_techbase\" \"C:/Users/User/data\"\n";
        return 1;
    }

    if (!txhx.init(remote_path, local_dir)) {
        std::cerr << "Ошибка инициализации TXHX (проверьте пути и права доступа).\n";
        return 1;
    }

    std::cout << "TXHX запущен. Выберите действие:\n";
    std::cout << "1 - Backup\n";
    std::cout << "2 - Restore\n";
    std::cout << "3 - Compress\n";
    std::cout << "> ";

    int choice;
    if (!(std::cin >> choice)) {
        std::cerr << "Неверный ввод.\n";
        return 1;
    }

    bool ok = false;
    switch (choice) {
        case 1:
            ok = txhx.backup();
            break;
        case 2:
            ok = txhx.restore();
            break;
        case 3:
            ok = txhx.compress();
            break;
        default:
            std::cerr << "Неизвестный выбор.\n";
            return 1;
    }

    return ok ? 0 : 1;
}
