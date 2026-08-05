#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
#else
    #error "Этот код написан под Windows"
#endif

// --- Утилиты для Windows (WinAPI) ---

bool dir_exists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool create_dir(const std::string& path) {
    if (dir_exists(path)) return true;
    return _mkdir(path.c_str()) == 0;
}

bool file_exists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool copy_file(const std::string& src, const std::string& dst, bool overwrite = true) {
    if (!file_exists(src)) return false;

    if (overwrite && file_exists(dst)) {
        if (!DeleteFileA(dst.c_str())) return false;
    } else if (!overwrite && file_exists(dst)) {
        return false;
    }

    return CopyFileA(src.c_str(), dst.c_str(), !overwrite) != 0;
}

std::vector<std::string> list_files(const std::string& dir) {
    std::vector<std::string> files;
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((dir + "\\*").c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) return files;

    do {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            files.push_back(std::string(findData.cFileName));
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    return files;
}

// --- Класс TBackup ---
class TBackup {
private:
    std::string backupDir;
    struct FileItem {
        std::string srcPath;
        std::string baseStem;
        std::string ext;
    };
    std::vector<FileItem> files;

    std::string getDateString() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_s(&tm, &time);

        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(4) << (tm.tm_year + 1900) << "."
            << std::setw(2) << (tm.tm_mon + 1) << "."
            << std::setw(2) << tm.tm_mday;
        return oss.str();
    }

public:
    explicit TBackup(const std::vector<std::string>& srcPaths) : backupDir("BackUp") {
        for (const auto& src : srcPaths) {
            FileItem item;
            item.srcPath = src;

            size_t pos = src.rfind('\\');
            if (pos == std::string::npos) pos = src.rfind('/');
            std::string baseFileName = (pos != std::string::npos) ? src.substr(pos + 1) : src;

            size_t dotPos = baseFileName.rfind('.');
            if (dotPos != std::string::npos) {
                item.baseStem = baseFileName.substr(0, dotPos);
                item.ext = baseFileName.substr(dotPos);
            } else {
                item.baseStem = baseFileName;
                item.ext = "";
            }

            files.push_back(item);
        }

        if (!create_dir(backupDir)) {
            throw std::runtime_error("Не удалось создать директорию BackUp");
        }

        std::cout << "[INFO] Директория BackUp: " << backupDir << std::endl;
        std::cout << "[INFO] Подготовлено файлов для бэкапа: " << files.size() << std::endl;
    }

    bool backup() {
        int successCount = 0;
        int failCount = 0;
        std::string dateStr = getDateString();

        for (const auto& item : files) {
            if (!file_exists(item.srcPath)) {
                std::cerr << "[ERROR] Исходный файл не найден: " << item.srcPath << std::endl;
                failCount++;
                continue;
            }

            std::string dest = backupDir + "\\" + item.baseStem + "_" + dateStr + item.ext;

            if (copy_file(item.srcPath, dest, true)) {
                std::cout << "[OK] Бэкап: " << dest << std::endl;
                successCount++;
            } else {
                std::cerr << "[ERROR] Не удалось скопировать: " << item.srcPath << " -> " << dest << std::endl;
                failCount++;
            }
        }

        std::cout << "[SUMMARY] Бэкап завершён: " << successCount << " успешно, " << failCount << " ошибок." << std::endl;
        return failCount == 0;
    }

    bool restore() {
        int successCount = 0;
        int failCount = 0;
        std::vector<std::string> backupFiles = list_files(backupDir);

        for (const auto& item : files) {
            std::string restoreFrom;
            bool found = false;

            for (const auto& name : backupFiles) {
                if (name.find(item.baseStem) == 0 &&
                    name.find('_') != std::string::npos &&
                    name.size() >= item.ext.size() &&
                    name.compare(name.size() - item.ext.size(), item.ext.size(), item.ext) == 0) {
                    restoreFrom = backupDir + "\\" + name;
                    found = true;
                    break;
                }
            }

            if (!found) {
                std::cout << "[WARN] Файл для восстановления не найден: '" << item.baseStem << item.ext << "' в папке BackUp" << std::endl;
                failCount++;
                continue;
            }

            if (copy_file(restoreFrom, item.srcPath, true)) {
                std::cout << "[OK] Восстановление: " << restoreFrom << " -> " << item.srcPath << std::endl;
                successCount++;
            } else {
                std::cerr << "[ERROR] Ошибка восстановления: " << restoreFrom << std::endl;
                failCount++;
            }
        }

        std::cout << "[SUMMARY] Восстановление завершено: " << successCount << " успешно, " << failCount << " ошибок." << std::endl;
        return failCount == 0;
    }
};

void show_help() {
    std::cout << "\nИспользование: backup.exe [опции]\n\n";
    std::cout << "Опции:\n";
    std::cout << "  /a, /backup      — выполнить только бэкап файлов\n";
    std::cout << "  /r, /restore     — выполнить только восстановление из бэкапа\n";
    std::cout << "  /?, /help        — показать эту справку\n\n";
    std::cout << "Без аргументов программа НЕ делает ничего — только выводит эту инструкцию.\n";
    std::cout << "Это сделано, чтобы случайно не перезаписать файлы при проверке запуска.\n\n";
}

int main(int argc, char* argv[]) {
    // Переключаем консоль на UTF‑8 (для корректного вывода кириллицы в cmd)
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif

    // Если есть аргумент помощи — сразу показываем справку
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "/?" || arg == "/help") {
            show_help();
            return 0;
        }
    }

    // Если аргументов нет — показываем инструкцию и выходим
    if (argc == 1) {
        show_help();
        return 0;
    }

    const std::vector<std::string> sources = {
        R"(\\10.67.1.36\oit_techbase\list-points\list-points.html)",
        R"(\\10.67.1.36\oit_techbase\list-points\list-points.xls)"
    };

    try {
        TBackup bk(sources);

        bool doRestore = false;
        bool doBackup = false;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "/r" || arg == "/restore") doRestore = true;
            if (arg == "/a" || arg == "/backup")  doBackup = true;
        }

        // Теперь без аргументов ничего не делается — только справка выше
        if (doRestore) {
            std::cout << "--- Выполняем восстановление ---" << std::endl;
            if (!bk.restore()) {
                std::cout << "[WARN] Восстановление завершено с ошибками или пропусками." << std::endl;
            }
        }

        if (doBackup) {
            std::cout << "--- Выполняем бэкап ---" << std::endl;
            if (!bk.backup()) {
                std::cerr << "[ERROR] Бэкап завершён с ошибками." << std::endl;
                return 1;
            }
        }

        std::cout << "--- Готово ---" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[CRITICAL] Непредвиденная ошибка: " << e.what() << std::endl;
        return 2;
    }
}
