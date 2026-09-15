#include <iostream>
#include <string>
#include <curl/curl.h>

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string& data = *static_cast<std::string*>(userdata);
    data.append(ptr, size * nmemb);
    return size * nmemb;
}

bool ftp_upload_file(const std::string& local_path,
                     const std::string& remote_path,
                     const std::string& host,
                     const std::string& user,
                     const std::string& pass) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "curl_easy_init failed\n";
        return false;
    }

    std::string url = "ftp://" + host + "/" + remote_path;

    FILE* fp = fopen(local_path.c_str(), "rb");
    if (!fp) {
        std::cerr << "Cannot open local file: " << local_path << "\n";
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, user.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pass.c_str());

    // CURLOPT_FTP_USE_PASV удалён: в новых libcurl он не нужен (по умолчанию используется пассивный режим)

    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, fp);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    std::string server_log;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &server_log);

    CURLcode res = curl_easy_perform(curl);

    fclose(fp);

    if (res != CURLE_OK) {
        std::cerr << "FTP upload failed: " << curl_easy_strerror(res) << "\n";
        if (!server_log.empty()) {
            std::cerr << "Server log:\n" << server_log << "\n";
        }
        curl_easy_cleanup(curl);
        return false;
    }

    std::cout << "Upload OK.\n";
    curl_easy_cleanup(curl);
    return true;
}

int main() {
    const std::string local = "./get_board_serial";
    const std::string remote = "/tmp/get_board_serial";
    const std::string host = "ftp.denkin.ru";
    const std::string user = "a0831755";
    const std::string pass = "vbnz1002";

    if (ftp_upload_file( local, remote, host, user, pass )) {
        std::cout << "[ Ok ] File uploaded successfully.\n";
    } else {
        std::cerr << "[ Err ] Upload failed.\n";
        return 1;
    }
    return 0;
}

