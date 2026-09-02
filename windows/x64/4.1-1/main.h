#pragma once
#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <string>
#include <string.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <windows.h>

#include "colors.h"


//using namespace std;

// dialog Yes/No
//bool askYesNo( const std::string& prompt );
bool askYesNo(const std::string& prompt, const std::string& exit_command = "quit");

// вертикальное меню выбор курсором + Enter
int show_menu_v(const std::vector<std::string>& items, int start_x, int start_y);
int show_menu_h(const std::vector<std::string>& items, int row);

// замена одного символа в строке
char *replace( char *src, int replaceme, int newchar );

// Returns hostname for the local computer
void checkHostName(int hostname);

// Returns host information corresponding to host name
void checkHostEntry(struct hostent * hostentry);

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer);

// очистить сверху N строк
void clearLines_up(int count);
// очистить снизу N строк
void clearlines_down(int n);
// очистить текущюю строку
void clear_current_line();

void clear_n_lines(int n);

void clear_n_lines_from_row(int start_y, int n);


                                                
/*  удаление подстроки из строки  */
std::string remove_substring(const std::string&, const std::string& );
void rem_subs( std::string id_str, const std::string& to_remove );


#include <array>

struct PrinterModel {
    std::string part1;
    std::string part2;
    std::string full_name;
};

// Структура для хранения информации об одном диске
struct DiskInfo {
    std::wstring model;
    std::wstring serialNumber;
    std::wstring type;
    unsigned long long sizeBytes;
};

	std::wstring formatDiskSize(unsigned long long bytes);  // <-- добавляем объявление
	std::vector<DiskInfo> getDisks();

class TInventory // имя класса
{
  /* private:                                                      */
  /* список свойств и методов для использования внутри класса      */
  /* public:                                                       */
  /* список методов доступных другим функциям и объектам программы */
  /* protected:                                                    */
  /*список средств, доступных при наследовании                     */

public: // спецификатор доступа public

  char buffer[80];
  std::string id_date     = "unknown",
         id_mb       = "unknown",
         id_cpu      = "unknown",
         id_mem      = "unknown",
         id_ip       = "unknown",
         id_mac      = "unknown",
         id_host     = "unknown",
         id_hdd      = "unknown",
         id_hdd_size = "unknown",
         id_sys      = "unknown",
         id_prn      = "unknown",
         id_filename = "unknown",
         
         id_office    = "unknown",   // кабинет
         id_structure = "unknown";  // здание


  TInventory( ); // конструктор класса

//private: // спецификатор доступа private

  bool get_mb     ( ); /* получение названия материнской платы         */
  bool get_cpu    ( ); /* получение названия процессора                */
  bool get_mem ( );
  bool get_ip  ( );
  bool get_mac ( );
  bool get_hdd( );
  //bool get_hdd ( );

// Новый метод: полная инвентаризация всех дисков
  //bool get_all_disks_info(std::vector<DiskInfo>& disks);
  //bool get_all_hdds();                // заполняет hdd_models (полная инвентаризация)
  //bool get_hdd_size( );

  bool get_host( );
  bool get_sys ( );
  bool get_prn ( );
  bool get_other ( );
  bool get_filename( );

  bool write_to_file( );
  bool delete_file( );
  bool write_to_ftp( );
  bool write_to_lan( );
  void err_message( std::string str); // функция (метод класса) выводящая сообщение об ошибке на экран
  bool scan_id(  void );
  bool print_id( void ); // функция (метод класса) выводящая данные на экран

private:
//  bool  get_mac_via_ioctl(const char* interface);
  void errMessage( std::string );
  void warMessage( std::string );
  void showMessage( std::string, std::string = "" );

protected:
}; // конец объявления класса TInventory
