#pragma once
#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/sysinfo.h>
#include <algorithm>

#include "colors.h"


using namespace std;

// dialog Yes/No
bool askYesNo( const string& prompt );

// замена одного символа в строке
char *replace( char *src, int replaceme, int newchar );

// Returns hostname for the local computer
void checkHostName(int hostname);

// Returns host information corresponding to host name
void checkHostEntry(struct hostent * hostentry);

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer);

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
  string id_date       = "unknown",
         id_mb         = "unknown",
         id_cpu        = "unknown",
         id_mem        = "unknown",
         id_ip         = "unknown",
         id_mac        = "unknown",
         id_host       = "unknown",
         id_hdd        = "unknown",
         id_hdd_size   = "unknown",
         id_sys        = "unknown",
         id_filename   = "unknown",
         id_prn        = "unknown",     /*  принтер по умолчанию */
         id_office     = "unknown",     /*  кабинет              */
         id_structure  = "unknown",     /*  здание               */
         id_inv_number = "unknown";     /*  инвентарный номер    */

  TInventory( ); // конструктор класса

//private: // спецификатор доступа private

  bool get_mb  ( );
  bool get_cpu ( );
  bool get_mem ( );
  bool get_ip  ( );
  bool get_mac ( );
  bool get_hdd ( );
  bool get_host( );
  bool get_sys ( );
  bool get_prn ( );
  bool get_other ( );
  bool get_hdd_size( );
  bool get_filename( );

  bool write_to_file( );
  bool delete_file( );
  bool write_to_ftp( );
  bool write_to_lan( );
  void err_message( string str); // функция (метод класса) выводящая сообщение об ошибке на экран
  bool scan_id(  void );
  bool print_id( void ); // функция (метод класса) выводящая данные на экран

private:
//  bool  get_mac_via_ioctl(const char* interface);

}; // конец объявления класса TInventory
