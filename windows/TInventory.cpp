#include <cerrno>
#include <cstring>
#include <unistd.h>

#include "main.h"

using namespace std;

    // Комбинируем цвета и эффекты
    // std::cout << bold << blue << "Заголовок программы" << reset << std::endl;

TInventory::TInventory( ) {  // конструктор класса

    cout << endl;
//    cout << yellow  << "═════════════════╣" << green  << "  Created by Dmitry Denkin (c) 2026 " << yellow << "╠═════════════════" << endl;
    cout << Colors::yellow  << "                                                               [ver 3.4-1]" << Colors::reset << endl;
    cout << Colors::brown  << "=================|" << Colors::green  << "  Created by Dmitry Denkin (c) 2026  " << Colors::brown << "|==================" << endl;
    cout << Colors::blue    << " *******          " << Colors::cyan << " S y s t e m   I n f o r m a t i o n " << Colors::blue   << "           *******" << Colors::reset << endl << endl;
    time_t current_time = time( nullptr );
    tm* local_time = localtime( &current_time );                                                       

    strftime(buffer, sizeof(buffer), "%d.%m.%Y", local_time);
    id_date = buffer;
}

bool TInventory::delete_file( ) {
/*
    == тоже работает ==
    if (std::remove( id_filename.c_str() ) == 0) {
        std::cout << "Файл '" << id_filename << "' успешно удалён\n";
    } else {
        std::perror("Ошибка при удалении файла");
        return false;
    }
*/

    if (unlink( (char*)id_filename.c_str() ) == 0) {
        cout << "File " << id_filename << " successfully deleted..." << endl;
    } else {
        std::cerr << "Аile deletion error: " << std::strerror(errno) << "data-file..." << endl;
        return 1;
    }
 return true;
}

bool TInventory::scan_id ( void ) { // функция (метод класса) сканирует устройства (комплектуху)
  if( !get_mb() )
	cout << "MB - error TInventory\n";
  if( !get_cpu() )
	cout << "CPU - error\n";
  if( !get_mem() )
	cout << "MEM - error\n";
  if( !get_hdd() )
	cout << "HDD - error\n";
  if( !get_hdd_size() )
	cout << "HDD-SIZE - error\n";
  if( !get_ip() )
	cout << "IP - error\n";
  if( !get_mac() )
	cout << "MAK - error\n";
  if( !get_host() )
	cout << "HOST - error\n";
  if( !get_sys() )
	cout << "SYS - error\n";
  if( !get_prn() )
	cout << "PRN - error\n";
  if( !get_filename( ) )
	cout << "FILENAME - error\n";
//  if( !get_other( ) )
//	cout << "FILENAME - error\n";
  return true;
}

std::string id_print(const std::string& id_str) {
    if (id_str == "unknown") {
        return std::string(Colors::red) + id_str + std::string(Colors::reset) + "\n";
    } else {
        return std::string(Colors::white) + id_str + std::string(Colors::reset) + "\n";
    }
}

bool TInventory::print_id( void ) { // функция (метод класса) выводящая данные на экран

  clearLines(2);   // очищаем 2 строки выше

/* 
   if (isatty(STDOUT_FILENO)) {
        // Стираем текущую строку и поднимаемся на 1 — повторяем 2 раза
        std::cout << "\033[K\033[A\033[K\033[A\033[K\033[A\033[K\033[A" << std::flush;
    }
*/
    std::cout << endl
        << "==> curr-date:\t"    << id_print( id_date ) 
        << "==> mother:\t"       << id_print( id_mb )
        << "==> proc:\t"         << id_print( id_cpu )
        << "==> memory:\t"       << id_print( id_mem )
        << "==> ip-addr:\t"      << id_print( id_ip )
        << "==> ip-mac:\t"       << id_print( id_mac )
        << "==> hard disk:\t"    << id_print( id_hdd )
        << "==> hard size:\t"    << id_print( id_hdd_size )
        << "==> host:\t"         << id_print( id_host )
        << "==> system:\t"       << id_print( id_sys )
        << "==> printer:\t"      << id_print( id_prn )
        << "==> filename:\t"     << id_print( id_filename )
        << "==> office:\t"       << id_print( id_office )
        << "==> structure:\t"    << id_print( id_structure ) << "\n"
        << std::flush;

  return true;
}

void TInventory::errMessage( string str ) {
  cout << Colors::red << "Error: " << Colors::white << str << Colors::reset << endl;
  return;
}
void TInventory::warMessage( string str ) {
  cout << Colors::magenta << "Warning: " << Colors::white << str << Colors::reset << endl;
  return;
}
