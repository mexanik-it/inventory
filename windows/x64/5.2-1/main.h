#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <vector>
#include <algorithm>
#include <array>

#ifdef _WIN32
#include <windows.h>
#endif

#include "colors.h"

// --- Глобальные функции (диалоги, утилиты) ---

bool askYesNo(const std::string& prompt, const std::string& exit_command = "quit");

char* replace(char* src, int replaceme, int newchar);

// Сетевые функции
void checkHostName(int hostname);
void checkHostEntry(struct hostent* hostentry);
void checkIPbuffer(char* IPbuffer);

// Функции очистки консоли
void clearLines_up(int count);
void clearlines_down(int n);
void clear_current_line();
void clear_n_lines(int n);
void clear_n_lines_from_row(int start_y, int n);

// Удаление подстроки
std::string remove_substring(const std::string&, const std::string&);
void rem_subs(std::string id_str, const std::string& to_remove);

/* замена всех вхождений подстроки */
void replace_all(std::string& str, const std::string& from, const std::string& to);

// --- Структуры ---

struct PrinterModel {
    std::string part1;
    std::string part2;
    std::string full_name;
};

struct DiskInfo {
    std::wstring model;
    std::wstring serialNumber;
    std::wstring type;
    unsigned long long sizeBytes;
};

struct LanInfo {
    std::string name;            // FriendlyName (как раньше)
    std::string interface_name;  // Description (название интерфейса)
    std::string mac;
    std::string ipv4;
    bool is_active = false;
};



// Объявления функций работы с дисками и сетью
std::wstring formatDiskSize(unsigned long long bytes);
std::vector<DiskInfo> getDisks();
std::vector<LanInfo> get_mac_addresses();


// --- Класс TInventory ---

class TInventory
{
public:
    char buffer[80];

    // Поля данных (инициализация в классе допустима в C++20)
    std::string
        id_date       = "unknown",      // дата
        id_mb         = "unknown",      // название материнской платы
        id_cpu        = "unknown",      // название процессора
        id_mem        = "unknown",      // общий объём оперативной памяти
        id_ip         = "unknown",
        id_mac        = "unknown",
        id_hdd        = "unknown",
        id_hdd_size   = "unknown",
        id_host       = "unknown",      // название компьютера
        id_sys        = "unknown",      // операционная система
        id_prn        = "unknown",      // название принтера
        id_filename   = "unknown",
        id_office     = "unknown",      // кабинет
        id_structure  = "unknown",      // здание
        id_inv_number = "unknown";      // инвентарный номер

    TInventory();

    // Методы получения данных
    bool get_mb();
    bool get_cpu();
    bool get_mem();
    bool get_lan();
    bool get_mac();
    bool get_hdd();
    bool get_host();
    bool get_sys();
    bool get_prn();
    bool get_other();
    bool get_filename();

    // Методы записи/удаления
    bool write_to_file();
    bool delete_file();
    bool write_to_ftp();
    bool write_to_lan();

    // Вывод сообщений
    void err_message(std::string str);
    bool scan_id(void);
    bool print_id(void);

private:
    void errMessage(std::string);
    void warMessage(std::string);
    void showMessage(std::string, std::string = "");
};
