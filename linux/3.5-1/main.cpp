#include "main.h"
#include <unistd.h> // для geteuid()
#include <chrono>

using namespace std;

/*------------------------------------------------------------------------------------------------------*/
/* процедура для изменения размеров окна терминала                                                      */

void SetTerminalSize(int rows, int cols) {
    std::cout << "\033[8;" << rows << ";" << cols << "t";
    std::cout.flush();
}

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

/*------------------------------------------------------------------------------------------------------*/
/* сначала собираем сведения - затем записываем их в файл на /tmp и пересылаем этот файл на ftp сервер  */

int main ( ) {

  /* clean screen */
  system ( "clear" );

  /* SetConsoleOutputCP(1251); */
  setlocale ( LC_ALL, "Russian" );
  SetTerminalSize(40, 120);

  if ( geteuid() ) {
     cout << "Launch is only possible with administrator (root) rights !" << endl
          << "Запуск только от имени администратора ... " << endl;
     exit( -1 );
    }

    // начинаем замер времени выполнения программы
    auto start = std::chrono::high_resolution_clock::now();

    // --- твой код инвентаризации ---
    // auto disks = get_all_disks();
    // print_disk_table(disks);
    // ------------------------------
    
  TInventory inv;

  inv.get_other( );

  if( !inv.scan_id ( ) )
    cout << endl << "Hardware scanning error: " << endl;

  inv.print_id ( );

  inv.write_to_file ( );

/***** вывод времени выполнения программы **************************************/
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cerr << Colors::bold << Colors::yellow
              << "[INFO] Время работы утилиты: " << (int)diff.count() << " сек."
              << Colors::reset << "\n\n";
/*******************************************************************************/
  if( askYesNo ( "Transfer report to ftp-server...: " ) )
      inv.write_to_ftp ( );

  if( askYesNo ( "Copy report to local base: " ) )
    inv.write_to_lan ( );

  inv.delete_file ( );

  return 0;
}
