// file: include/txhx_core.hpp
#pragma once

#include <string>
#include <filesystem>
#include "backup_stats.hpp"  // <-- ÝÒÀ ÑÒÐÎÊÀ ÊÐÈÒÈ×ÅÑÊÈ ÂÀÆÍÀ

namespace fs = std::filesystem;

class TXHX {
public:
    TXHX() = default;

    bool init(const std::string& remote_path, const std::string& local_dir);
    
    BackupStats backup();  // <-- Òåïåðü êîìïèëÿòîð çíàåò, ÷òî ýòî çà òèï
    
    bool restore();
    bool compress();

    void show_progress(const wchar_t* title);
    void close_progress();
    void set_progress(int percent, const std::string& message, double eta_seconds);
    void add_report(const std::string& msg);

    const std::string& get_remote_path() const { return m_remote_path; }
    const std::string& get_local_dir() const   { return m_local_dir; }

private:
    fs::path m_backup_root = "backup";
    std::string m_remote_path;
    std::string m_local_dir;
};
