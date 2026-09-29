// main.h

#pragma once

#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <string>
#include <vector>
#include <cstdint>            // uint64_t
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <netdb.h>            // struct hostent
#include <algorithm>

#include "colors.h"

using namespace std;

// --- Структуры данных (ОБЯЗАТЕЛЬНО ДО функций, которые их используют) ---
struct MbInfo {
    std::string vendor;       // ASUSTeK
    std::string model;        // A68HM-K
    std::string part;         // board_asset_tag, если прошит
    std::string serial;       // board_serial / product_serial
};

struct DiskInfo {
    std::string model;
    long int size;
    std::string type;
};

struct LanInfo {
    std::string name;
    std::string ip;
    std::string mac;
};

struct MemoryModule {
    std::string manufacturer;
    std::string partNumber;
    std::string serialNumber;
    uint64_t capacityBytes = 0;

    std::string memoryType;   // DDR3 / DDR4 (DMI Type)
    std::string speed;        // 1333 MT/s  (Configured Memory Speed)
    std::string locator;      // DIMM_A1    (слот)
};


// --- Свободные функции (диалоги, утилиты) ---
bool askYesNo(const string& prompt);
char* replace(char* src, int replaceme, int newchar);

void checkHostName(int hostname);
void checkHostEntry(struct hostent* hostentry);
void checkIPbuffer(char* IPbuffer);

void okMessage(const std::string& msg);
void errMessage(const std::string& msg);

// Теперь это объявление видит тип MbInfo, потому что структура выше
std::string mbIdString(const MbInfo& mb);


// --- Класс инвентаризации ---
class TInventory
{
public:
    char buffer[80];

    TInventory();

    bool write_to_file();
    bool delete_file();
    bool write_to_ftp();
    bool write_to_lan();

    void err_message(string str);

    bool scan_id(void);
    bool print_id(void);
    bool get_other(void);

private:
    // Основные инвентарные поля (строки)
    string id_date       = "unknown",
           id_cpu        = "unknown",
           id_mem        = "unknown",
           id_mem_name   = "unknown",
           sn_mem        = "unknown",
           id_ip         = "unknown",
           id_mac        = "unknown",
           id_host       = "unknown",
           id_hdd        = "unknown",
           id_hdd_size   = "unknown",
           id_sys        = "unknown",
           id_filename   = "unknown",
           id_prn        = "unknown",     /* принтер по умолчанию */
           id_office     = "unknown",    /* кабинет */
           id_structure  = "unknown",    /* здание */
           id_inv_number = "unknown";    /* инвентарный номер */

    // Данные о материнской плате — теперь только структура
    MbInfo mb;

    // Методы сбора данных
    bool get_mb();
    bool get_cpu();
    bool get_mem();
    bool get_ip();
    bool get_mac();
    bool get_hdd();
    bool get_host();
    bool get_sys();
    bool get_prn();
    bool get_hdd_size();
    bool get_filename();
};
