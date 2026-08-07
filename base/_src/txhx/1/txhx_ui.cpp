// file: include/txhx_ui.cpp

#include "txhx_ui.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <thread>

static bool g_ui_active = false;
static std::chrono::steady_clock::time_point g_start_time;

void ui_start_progress(const std::wstring& title) {
    g_ui_active = true;
    g_start_time = std::chrono::steady_clock::now();

    // Заголовок: переводим wstring в string (простая ASCII-версия для консоли)
    std::string t;
    for (wchar_t c : title) {
        if (c < 128) t.push_back(static_cast<char>(c));
        else t.push_back('?');
    }
    std::cout << "\n=== " << t << " ===\n";
}

void ui_update_progress(int percent, const std::string& status, double eta_seconds) {
    if (!g_ui_active) return;

    // Простой прогресс-бар в одной строке (перезапись той же строки)
    const int bar_width = 40;
    int filled = percent * bar_width / 100;

    std::cout << "\r[";
    for (int i = 0; i < filled; ++i) std::cout << "#";
    for (int i = filled; i < bar_width; ++i) std::cout << ".";
    std::cout << "] "
              << std::setw(3) << percent << "% "
              << status;

    if (eta_seconds > 0) {
        int eta_min = static_cast<int>(eta_seconds) / 60;
        int eta_sec = static_cast<int>(eta_seconds) % 60;
        std::cout << " | ETA: " << eta_min << "m " << eta_sec << "s";
    }

    std::cout.flush();
}

void ui_finish_progress() {
    if (!g_ui_active) return;
    std::cout << "\nDone.\n";
    g_ui_active = false;
}
