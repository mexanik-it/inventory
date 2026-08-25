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