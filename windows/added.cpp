#include <iostream>
#include <vector>
//#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm> // Для std::transform
#include <limits>    // Для std::numeric_limits
#include <cctype>    // Для ::tolower
#include <unistd.h>       // для STDIN_FILENO
#include <fcntl.h>        // если понадобится, например, для fcntl

#include <windows.h> // Обязательно подключите этот заголовок
#include <ios>       // Для std::ios::sync_with_stdio

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


bool askYesNo(const std::string& prompt, const std::string& exit_command = "quit") {
    std::string input;

    while (true) {
        std::cout << prompt << " (yes/no";
        if (!exit_command.empty()) {
            std::cout << "/" << exit_command;
        }
        std::cout << "): ";

        // Проверяем состояние потока ПЕРЕД чтением
        if (!(std::cin >> input)) {
            // Обработка Ctrl+Z / Ctrl+D или системной ошибки ввода
            std::cout << "\nОшибка ввода. Завершение.\n";
            return false; 
        }

        // Преобразуем к нижнему регистру для сравнения
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

        // Вход неверный: очищаем строку от лишних символов до конца строки
        // Это предотвращает захват "мусора" следующей операцией ввода
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

// Очистить N строк выше курсора (только Windows API, без ANSI)
void clearLines(int count) {
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

/**************************************************************************************************/
/* меню вертикальное                                                                              */
/* использование:                                                                                 */
/* --------------                                                                                 */
/* int main() {                                                                                   */
/*    std::vector<std::string> menu = {                                                           */
/*        "Start Game",                                                                           */
/*        "Settings",                                                                             */
/*        "Credits",                                                                              */
/*        "Exit"                                                                                  */
/*    };                                                                                          */
/*                                                                                                */
/*    int choice = show_menu(menu, 5, 3);                                                         */
/*                                                                                                */
/*    switch (choice) {                                                                           */
/*        case 0: std::cout << "\n Старшая сестра\n"; break;                                      */
/*        case 1: std::cout << "\n Заведующая\n"; break;                                          */
/*        case 2: std::cout << "\n Ординаторская\n"; break;                                       */
/*        case 3: std::cout << "\n Другое...\n"; return 0;                                        */
/*    }                                                                                           */
/*                                                                                                */
/*     return 0;                                                                                  */
/* }                                                                                              */
/**************************************************************************************************/
void gotoxy_v(int x, int y) {
    COORD pos = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void clear_screen() {
    system("cls");
}

int show_menu_v(const std::vector<std::string>& items, int start_x, int start_y) {
    int selected = 0;
    int count = static_cast<int>(items.size());

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hIn, &mode);
    SetConsoleMode(hIn, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

    while (true) {
        clear_screen();
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
                    SetConsoleMode(hIn, mode);
                    return selected;
            }
        }
    }
}

// Включение/выключение raw-режима терминала
// Глобальная переменная для хранения исходного режима консоли
static DWORD original_mode = 0;
static bool mode_changed = false;

void set_raw(bool enable) {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE) return;

    DWORD mode;
    if (!GetConsoleMode(hInput, &mode)) return;

    if (enable) {
        // Сохраняем оригинальный режим
        original_mode = mode;
        
        // Отключаем буферизацию ввода, эхо и обработку Ctrl+C
        mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
        
        SetConsoleMode(hInput, mode);
        mode_changed = true;
    } else {
        if (mode_changed) {
            // Восстанавливаем оригинальный режим
            SetConsoleMode(hInput, original_mode);
            mode_changed = false;
        }
    }
}
// ANSI-код: переместить курсор в (x,y) (y=строка, x=колонка)
void gotoxy_h(int x, int y) {
    std::cout << "\033[" << y << ";" << x << "H" << std::flush;
}

// ANSI-код: очистить до конца строки
void clear_eol() {
    std::cout << "\033[K" << std::flush;
}

/*************************************************************************************************/
/* горизонтальное меню                                                                           */
/* пример использования:                                                                         */
/* --------------------                                                                          */
/* int main() {                                                                                  */
/*     const std::vector<std::string> items = {"Start", "Settings", "Help", "Exit"};             */
/*     const int row = 5;                                                                        */
/*                                                                                               */
/*     int selected_index = show_menu(items, row);                                               */
/*                                                                                               */
/*     std::cout << "\nВыбран пункт: " << items[selected_index] << "\n";                         */
/*     return 0;                                                                                 */
/* }                                                                                             */
/*************************************************************************************************/
/**
 * Показывает горизонтальное меню и возвращает индекс выбранного пункта.
 * @param items Список пунктов меню.
 * @param row Строка терминала, где рисовать меню.
 * @return Индекс выбранного пункта (0..items.size()-1).
 */
int show_menu_h(const std::vector<std::string>& items, int row) {
    int selected = 0;

    set_raw(true); // включить raw-режим
    atexit([](){ set_raw(false); }); // вернуть настройки при выходе

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
            // Добавляем пробелы, чтобы выровнять ширину элементов
            int padding = item_width - static_cast<int>(items[i].size()) - 2;
            if (padding > 0) {
                std::cout << std::string(padding, ' ');
            }
        }
        std::cout << std::flush;

        int ch = getchar();
        if (ch == 27 && getchar() == '[') { // ESC [ — начало спецкода стрелок
            char dir = getchar();
            if (dir == 'D') { // Left
                selected = (selected == 0) ? static_cast<int>(items.size()) - 1 : selected - 1;
            } else if (dir == 'C') { // Right
                selected = (selected + 1 >= static_cast<int>(items.size())) ? 0 : selected + 1;
            }
        } else if (ch == '\n' || ch == '\r') { // Enter
            break;
        }
    }

    return selected;
}