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
  /* setlocale ( LC_ALL, "Russian" ); */
//    SetConsoleSize(100, 30);
//  if ( geteuid() ) {
//     cout << "Launch is only possible with administrator (root) rights !" << endl
//          << "Запуск только от имени администратора ... " << endl;
//     exit( -1 );
//    }

  TInventory inv;

  if( !inv.scan_id ( ) )
    cout << endl << "Hardware scanning error: " << endl;


  inv.get_other( );
  
  inv.print_id ( );

  inv.write_to_file ( );

  if( askYesNo ( "Transfer report to ftp-server...: " ) )
      inv.write_to_ftp ( );

  if( askYesNo ( "Copy report to local base: " ) )
      inv.write_to_lan ( );

//  inv.delete_file ( );

  system( "pause" );
  return 0;
}
