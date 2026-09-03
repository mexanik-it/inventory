#include "main.h"
#include <wininet.h>

bool uploadFileToFTP(const char* server, const char* username,
                  const char* password, const char* localFile,
                  const char* remoteFile) {
    HINTERNET hInternet = InternetOpenA("FTP Client",
        INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hFtpSession = InternetConnectA(hInternet, server,
        INTERNET_DEFAULT_FTP_PORT, username, password,
        INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);

    if (!hFtpSession) {
        InternetCloseHandle(hInternet);
        return false;
    }

    bool success = FtpPutFileA(hFtpSession, localFile, remoteFile,
        FTP_TRANSFER_TYPE_BINARY, 0);

    InternetCloseHandle(hFtpSession);
    InternetCloseHandle(hInternet);
    return success;
}
 
bool TInventory::write_to_ftp( ) {
  std::string buff1 = "/";
  std::string buff2 = "/tmp/";
  buff1 += id_filename; 
  buff2 += id_filename; 

    if ( uploadFileToFTP("86.110.167.86", "dimon", "1002",
                       id_filename.c_str(), buff1.c_str() )) {
        std::cout << Colors::green << "File uploaded successfully to home dir!" << Colors::reset << std::endl;
        return true;
    } 
    if(  uploadFileToFTP("ftp.denkin.ru", "a0831755", "vbnz1002",
                       id_filename.c_str(), buff2.c_str() ) ){
        std::cout << Colors::green << "File uploaded successfully to denkin dir!" << Colors::reset << std::endl;
        return true;
    }

  std::cout << Colors::red << "An error occurred while transferring the file: " << Colors::reset << std::endl;  
  return false;
}
