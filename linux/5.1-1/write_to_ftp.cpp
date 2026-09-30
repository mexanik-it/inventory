#include <iostream>
#include <string>
#include "main.h"

#include <iostream>
#include <string>
#include <curl/curl.h>
/*
bool UploadFileToFTP(const char* localFilePath,
                     const char* ftpServer,      // только IP или имя, без ftp://
                     const char* username,
                     const char* password,
                     const char* remoteFileName) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string url = "ftp://" + std::string(ftpServer) + "/" + remoteFileName;

    FILE* localFile = fopen(localFilePath, "rb");
    if (!localFile) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, localFile);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L); // создать путь, если нет
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);                  // нужен upload

    // Для отладки можно включить verbose:
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    fclose(localFile);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && httpCode >= 200 && httpCode < 300);
}
*/

bool TInventory::write_to_ftp() {

  // Формируем команду curl
  std::string ftp_server = "ftp.denkin.ru";
  std::string remote_path = "/tmp/" + id_filename;
  std::string username = "a0831755";
  std::string password = "vbnz1002";
  std::string local_file = id_filename;

  std::string command = "/usr/bin/curl -T " + local_file +
    " ftp://" + username + ":" + password + "@" + ftp_server + remote_path;
  int result = system(command.c_str());

  if (result == 0) {
    okMessage( "The report file has been successfully uploaded to the server ftp.denkin.ru" );
    return true;
  }


  ftp_server = "86.110.167.86";
  remote_path = "/";
  username = "dimon";
  password = "1002";


  command = "/usr/bin/curl -T " + local_file +
    " ftp://" + username + ":" + password + "@" + ftp_server + remote_path;
  result = system(command.c_str());

  if (result == 0) {
    okMessage( "The report file has been successfully uploaded to the server 86.110.167.86" );
    return true;
  }

  errMessage( "An error occurred while transferring the file: " );
  return false;
}