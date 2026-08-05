#include <iostream>
#include <string>
#include <algorithm>
#include <windows.h>

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

#include <iostream>
#include <string>
#include <algorithm> // Для std::transform
#include <limits>    // Для std::numeric_limits
#include <cctype>    // Для ::tolower

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
