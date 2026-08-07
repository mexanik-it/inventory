// file: include/txhx_ui.hpp

#pragma once

#include <string>

// UI абстракция: в будущем можно заменить на GUI или оставить консольный
void ui_start_progress(const std::wstring& title);
void ui_update_progress(int percent, const std::string& status, double eta_seconds);
void ui_finish_progress();
