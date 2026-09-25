#include "main.h"
#include <unistd.h> // для geteuid()

void SetConsoleSize(int cols, int lines) {
    // Приводим к SHORT, чтобы убрать предупреждения о сужении
    COORD bufferSize = {static_cast<SHORT>(cols), static_cast<SHORT>(lines * 2)};

    SMALL_RECT windowRect = {
        0,
        0,
        static_cast<SHORT>(cols - 1),
        static_cast<SHORT>(lines - 1)
    };

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleScreenBufferSize(hOut, bufferSize);
    SetConsoleWindowInfo(hOut, TRUE, &windowRect);
}

void enable_vt_mode() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

using namespace std;

/*------------------------------------------------------------------------------------------------------*/
/*                                   ОСНОВНОЙ МОДУЛЬ СИСТЕМЫ                                            */
/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/* Программа для инвентаризации оборудования на компьютере. Позволяет получить сведения о комплектующих */
/* системного блока, в будующем еще и о переферии. Так-же определяется версия операционной системы.     */
/* После этого все полученные сведения отправляются на FTP сервер для последующей обработки и записи в  */
/* общюю базу данных.                                                                                   */
/*                                                                                                      */
/*                                                                              Dmitry Denkin (c) 2026  */
/*------------------------------------------------------------------------------------------------------*/

//########################################################################################################
// Class TInventory  (c) 2025
//########################################################################################################

/*------------------------------------------------------------------------------------------------------*/
/* сначала собираем сведения - затем записываем их в файл на /tmp и пересылаем этот файл на ftp сервер  */

int main ( ) {

  /* clean screen */
  system ( "cls" );
  enable_vt_mode();

  SetConsoleOutputCP(65001);
  rebuildFont( );
  /* setlocale ( LC_ALL, "Russian" ); */
    SetConsoleSize( 100, 35);
//  if ( geteuid() ) {
//     cout << "Launch is only possible with administrator (root) rights !" << endl
//          << "Запуск только от имени администратора ... " << endl;
//     exit( -1 );
//    }

  TInventory inv;

  inv.get_other( );
  



using Clock = std::chrono::high_resolution_clock;
auto start = Clock::now();
	inv.scan_id();
	inv.print_id ( );

auto end = Clock::now();

// Важно: end - start даёт duration, потом duration_cast в milliseconds
auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << " Working time is scan_id() took : ";
    if (ms < 1000) {
        std::cout << ms << " ms";
    } else {
        long long sec = ms / 1000;
        std::cout << sec << " s";
    }
std::cout << "\n ------------------------------\n\n";




  inv.write_to_file ( );

  if( inv.write_to_ftp ( ) )
	okMessage( "Transfer report to ftp-server...: " );
  else
	errMessage( "Error transfer to ftp " );

  if( inv.write_to_lan ( ) )
	okMessage( "Copy file to lan ..." );
  else
	errMessage( "Error copy to lan ..." );


//  inv.delete_file ( );

  system( "pause" );
  return 0;
}
