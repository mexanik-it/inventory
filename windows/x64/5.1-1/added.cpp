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

int show_menu_v(const std::vector<std::string>& items, int start_x, int start_y) {
    int selected = 0;
    int count = static_cast<int>(items.size());

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hIn, &mode);
    SetConsoleMode(hIn, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

    while (true) {
        // Очищаем ровно count строк, начиная с start_y
        //clear_n_lines_from_row(start_y, count); 

        for (int i = 0; i < count; ++i) {
            gotoxy_v(start_x, start_y + i);
            if (i == selected) {
                std::cout << "> " << items[i];
            } else {
                std::cout << "  " << items[i];
            }
        }

        INPUT_RECORD ir;
        DWORD read;
        ReadConsoleInputA(hIn, &ir, 1, &read);

        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            switch (ir.Event.KeyEvent.wVirtualKeyCode) {
                case VK_UP:
                    selected = (selected - 1 + count) % count;
                    break;
                case VK_DOWN:
                    selected = (selected + 1) % count;
                    break;
                case VK_RETURN:
                    SetConsoleMode(hIn, mode); // Восстанавливаем режим консоли
                    return selected;         // Возвращаем выбор
            }
        }
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

int show_menu_h(const std::vector<std::string>& items, int row) {
    int selected = 0;

    set_raw(true);
    atexit([](){ set_raw(false); });

    int max_len = 0;
    for (const auto& s : items) {
        max_len = std::max(max_len, static_cast<int>(s.size()));
    }
    int item_width = max_len + 2;

    while (true) {
        gotoxy_h(0, row);
        clear_eol();

        for (size_t i = 0; i < items.size(); ++i) {
            if (static_cast<int>(i) == selected) {
                std::cout << "[" << items[i] << "]";
            } else {
                std::cout << " " << items[i] << " ";
            }
            int padding = item_width - static_cast<int>(items[i].size()) - 2;
            if (padding > 0) {
                std::cout << std::string(padding, ' ');
            }
        }
        std::cout << std::flush;

        int ch = getchar();
        if (ch == 27 && getchar() == '[') {
            char dir = getchar();
            if (dir == 'D') {
                selected = (selected == 0) ? static_cast<int>(items.size()) - 1 : selected - 1;
            } else if (dir == 'C') {
                selected = (selected + 1 >= static_cast<int>(items.size())) ? 0 : selected + 1;
            }
        } else if (ch == '\n' || ch == '\r') {
            break;
        }
    }

    return selected;
}
