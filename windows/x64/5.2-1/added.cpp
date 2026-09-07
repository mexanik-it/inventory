#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <cctype>
#include <fcntl.h>

//#include <windows.h>
//#include <ios>

#include "main.h"

// --- ПРОТОТИПЫ ФУНКЦИЙ (если не вынесены в main.h) ---
void clear_n_lines_from_row(int start_y, int n);

// Цвета для Windows консоли
enum class Color {
    black       = 0,
    dark_blue   = 1,
    dark_green  = 2,
    dark_cyan   = 3,
    dark_red    = 4,
    dark_magenta= 5,
    dark_yellow = 6,
    gray        = 7,
    dark_gray   = 8,
    blue        = 9,
    green       = 10,
    cyan        = 11,
    red         = 12,
    magenta     = 13,
    yellow      = 14,
    white       = 15
};

void setColor(Color textColor, Color bgColor = Color::black) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD attribute = static_cast<WORD>((static_cast<int>(bgColor) << 4) | static_cast<int>(textColor));
    SetConsoleTextAttribute(hConsole, attribute);
}

void resetColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        SetConsoleTextAttribute(hConsole, csbi.wAttributes);
    }
}

// Исправленная функция askYesNo (без дубликатов и без значения по умолчанию в .cpp)
bool askYesNo(const std::string& prompt, const std::string& exit_command) {
    std::string input;

    while (true) {
        std::cout << prompt << " (yes/no";
        if (!exit_command.empty()) {
            std::cout << "/" << exit_command;
        }
        std::cout << "): ";

        if (!(std::cin >> input)) {
            std::cout << "\nОшибка ввода. Завершение.\n";
            return false; 
        }

        std::transform(input.begin(), input.end(), input.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (input == "yes" || input == "y") {
            return true;
        }
        if (input == "no" || input == "n") {
            return false;
        }
        if (!exit_command.empty() && input == exit_command) {
            std::cout << "Ввод отменен пользователем.\n";
            throw std::runtime_error("User cancelled operation");
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Некорректный ввод. Попробуйте еще раз.\n";
    }
}

/* замена всех вхождений подстроки */
void replace_all(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

// Безопасная замена символа в std::string (возвращает новую строку)
std::string replaceChar(const std::string &src, char oldChar, char newChar) {
    std::string result = src;
    std::replace(result.begin(), result.end(), oldChar, newChar);
    return result;
}

// Удалить все вхождения подстроки
void removeAllSubstrings(std::string &str, const std::string &sub) {
    if (sub.empty()) return;
    size_t pos = 0;
    while ((pos = str.find(sub, pos)) != std::string::npos) {
        str.erase(pos, sub.size());
    }
}

// Очистить текущую строку (только Windows API)
void clear_current_line() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

    DWORD written = 0;
    COORD pos = csbi.dwCursorPosition;

    FillConsoleOutputCharacter(hOut, ' ',
        (csbi.dwSize.X - pos.X),
        pos, &written);

    pos.X = 0;
    SetConsoleCursorPosition(hOut, pos);
}

// Очистить N строк выше курсора
void clearLines_up(int count) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

    COORD cursorPos = csbi.dwCursorPosition;
    if (cursorPos.Y < count) count = cursorPos.Y;

    DWORD written;
    for (int i = 0; i < count; ++i) {
        COORD cur = {0, static_cast<SHORT>(cursorPos.Y - count + i)};
        FillConsoleOutputCharacterA(hOut, ' ', csbi.dwSize.X, cur, &written);
        FillConsoleOutputAttribute(hOut, csbi.wAttributes, csbi.dwSize.X, cur, &written);
    }

    cursorPos.Y -= count;
    cursorPos.X = 0;
    SetConsoleCursorPosition(hOut, cursorPos);
}

// Очистить N строк ниже курсора
void clearlines_down(int n) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

    COORD pos = csbi.dwCursorPosition;
    DWORD written = 0;

    for (int i = 0; i < n; ++i) {
        COORD linePos = {0, static_cast<SHORT>(pos.Y + i)};
        FillConsoleOutputCharacter(hOut, ' ', csbi.dwSize.X, linePos, &written);
    }

    SetConsoleCursorPosition(hOut, pos);
}

/**************************************************************************************************/
/* меню вертикальное                                                                              */
/**************************************************************************************************/
void gotoxy_v(int x, int y) {
    COORD pos = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void clear_screen() {
    system("cls");
}

void clear_n_lines(int n) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

    COORD pos = csbi.dwCursorPosition;
    DWORD written = 0;

    for (int i = 0; i < n; ++i) {
        COORD linePos = {0, static_cast<SHORT>(pos.Y + i)};
        FillConsoleOutputCharacter(hOut, ' ', csbi.dwSize.X, linePos, &written);
    }

    SetConsoleCursorPosition(hOut, pos);
}

// Реализация функции, которую раньше не хватало
void clear_n_lines_from_row(int start_y, int n) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

    DWORD written = 0;
    for (int i = 0; i < n; ++i) {
        COORD linePos = {0, static_cast<SHORT>(start_y + i)};
        FillConsoleOutputCharacter(hOut, ' ', csbi.dwSize.X, linePos, &written);
        // Опционально: можно раскомментировать, если остаются «призраки» цвета:
        // FillConsoleOutputAttribute(hOut, csbi.wAttributes, csbi.dwSize.X, linePos, &written);
    }
}


// Включение/выключение raw-режима терминала
static DWORD original_mode = 0;
static bool mode_changed = false;

void set_raw(bool enable) {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE) return;

    DWORD mode;
    if (!GetConsoleMode(hInput, &mode)) return;

    if (enable) {
        original_mode = mode;
        mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
        SetConsoleMode(hInput, mode);
        mode_changed = true;
    } else {
        if (mode_changed) {
            SetConsoleMode(hInput, original_mode);
            mode_changed = false;
        }
    }
}

/*************************************************************************************************/
/* горизонтальное меню (ANSI)                                                                     */
/*************************************************************************************************/
void gotoxy_h(int x, int y) {
    std::cout << "\033[" << y << ";" << x << "H" << std::flush;
}

void clear_eol() {
    std::cout << "\033[K" << std::flush;
}




#include <iostream>
#include <string>
#include <windows.h>

// --- Цвета как константы (WinAPI) ---
constexpr WORD COLOR_GRAY   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;                 // приглушённый белый
constexpr WORD COLOR_GREEN  = FOREGROUND_GREEN | FOREGROUND_INTENSITY;                             // ярко-зелёный
constexpr WORD COLOR_RED    = FOREGROUND_RED | FOREGROUND_INTENSITY;                               // ярко-красный
constexpr WORD COLOR_DEFAULT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

// Вспомогательная функция: установить цвет
static inline void setConsoleColor(WORD color) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, color);
}

// [Ok] зелёный, сообщение серое
void okMessage(const std::string& msg) {
    setConsoleColor(COLOR_GRAY);
    std::cout << "[ ";

    setConsoleColor(COLOR_GREEN);
    std::cout << "Ok";

    setConsoleColor(COLOR_GRAY);
    std::cout << " ]";
    std::cout << " " << msg;

    setConsoleColor(COLOR_DEFAULT);
    std::cout << "\n";
}

// [Fail] красный, сообщение серое
void errMessage(const std::string& msg) {
    setConsoleColor(COLOR_GRAY);
    std::cout << "[ ";

    setConsoleColor(COLOR_RED);
    std::cout << "Fail";

    setConsoleColor(COLOR_GRAY);
    std::cout << " ]";
    std::cout << " " << msg;

    setConsoleColor(COLOR_DEFAULT);
    std::cout << "\n";
}
