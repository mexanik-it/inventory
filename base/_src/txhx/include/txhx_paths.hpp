
#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

bool validate_remote_path(const std::string& path, bool create_if_missing = false);
bool validate_local_path(const std::string& path);
std::string make_backup_dirname();
fs::path make_backup_dir_path();
