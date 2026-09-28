#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>   // <-- ОБЯЗАТЕЛЬНО: здесь лежит ioctl и TIOCGWINSZ
#include <unistd.h>      // <-- ОБЯЗАТЕЛЬНО: здесь лежит STDOUT_FILENO

/*------------------------------------------------------------------------------------------------------*/
/*                                   ДОПОЛНИТЕЛЬНЫЙ МОДУЛЬ СИСТЕМЫ                                      */
/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/* Здесь собраны всякие дополнительные функции,которые возможно смогут помочь облегчить жизнь           */
/*  программисту )))                                                                                    */
/*                                                                                                      */
/* = bool getSizeWindows( );  получение размеров окна                                                   */
/* = bool askYesNo(const string& prompt);  запрос Yes/No                                                */
/*                                                                              Dmitry Denkin (c) 2026  */
/*------------------------------------------------------------------------------------------------------*/

using namespace std;

/*------------------------------------------------------------------------------------------------------*/
/* запрос на ввод Yes/No  возврат: True/False                                                           */

bool askYesNo(const string& prompt) {
    string input;

     do {
        cout << Colors::white << prompt << Colors::cyan << "(yes/no): " << Colors::reset;
        cin >> input;
        transform(input.begin(), input.end(), input.begin(), ::tolower);

    for (char &c : input) {          /* преобразование к верхнему регистру */
            c = std::toupper(c);    /* мало-ли включён CapsLock           */
                }
        if (input == "YES" || input == "Y") return true;
        if (input == "NO" || input == "N")  return false;
	cout << "\x1b[A\r" << string(78, ' ') << "\r";
    } while (true);
}


/*------------------------------------------------------------------------------------------------------*/
/* замена символа (одного) в строке                                                                     */

char *replace(char *src, int replaceme, int newchar) {  
 //int len = strlen(src);  
 char *p = src;  
 
 for (p = src; *p; p++) {  
    if (*p == replaceme)  
       *p = newchar;  
    }  
 return src;  
}

// Returns hostname for the local computer
void checkHostName(int hostname) {
 if (hostname == -1)
  {
     perror("gethostname");
     exit(1);
    }
}

// Returns host information corresponding to host name
void checkHostEntry(struct hostent * hostentry) {
 if (hostentry == NULL)
  {
    perror("gethostbyname");
    exit(1);
    }
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer) {
 if (NULL == IPbuffer)
  {
    perror("inet_ntoa");
    exit(1);
    }
}


/*------------------------------------------------------------------------------------------------------*/
/* получение сведений об окне терминала                                                                 */
/* bool getSizeWindows() для винды                                                                      */
/* bool getSizeLinux() и для линуха                                                                     */

#ifdef _WIN32
    // --- КОД ДЛЯ WINDOWS ---
    #include <windows.h>

    bool getSizeWindows() {
    struct winsize term_ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_ws) != 0) {
        return -1;  // ошибка
    }
    std::cerr << "rows=" << term_ws.ws_row
              << ", cols=" << term_ws.ws_col << "\n";
    return 0;      // успех    }

#else
    // --- КОД ДЛЯ LINUX (CentOS, ALT) ---
    #include <sys/ioctl.h>
    #include <unistd.h>

    bool getSizeLinux() {
        struct winsize term_ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_ws) == 0) {
            std::cerr << "Linux: rows=" << term_ws.ws_row 
                      << ", cols=" << term_ws.ws_col << "\n";
            return true;
        }
        return false;
    }
#endif

#include <iostream>
#include <string>
#include <unistd.h>   // isatty, fileno
#include <thread>
#include <chrono>

// --- Цвета через ANSI-коды ---
static const char* COLOR_GRAY   = "\033[37m";   // приглушённый белый
static const char* COLOR_GREEN  = "\033[1;32m"; // ярко-зелёный
static const char* COLOR_RED    = "\033[1;31m"; // ярко-красный
static const char* COLOR_DEFAULT = "\033[0m";   // сброс

// Установить цвет
static inline void setConsoleColor(const char* color) {
    std::cout << color;
}

// Проверка: вывод идёт в консоль, а не в файл
static inline bool isConsoleOutput() {
    return isatty(fileno(stdout));
}

void okMessage(const std::string& msg) {
    if (!isConsoleOutput()) {
        std::cout << "[ Ok ] " << msg << "\n";
        return;
    }

    setConsoleColor(COLOR_GRAY);
    std::cout << "[  ";

    setConsoleColor(COLOR_GREEN);
    std::cout << "Ok";

    setConsoleColor(COLOR_GRAY);
    std::cout << "  ]  ";

    for (char c : msg) {
        std::cout << c;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    setConsoleColor(COLOR_DEFAULT);
    std::cout << "\n";
}

void errMessage(const std::string& msg) {
    setConsoleColor(COLOR_GRAY);
    std::cout << "[ ";

    setConsoleColor(COLOR_RED);
    std::cout << "Fail";

    setConsoleColor(COLOR_GRAY);
    std::cout << " ]";

    for (char c : msg) {
        std::cout << c;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    setConsoleColor(COLOR_DEFAULT);
    std::cout << "\n";
}

