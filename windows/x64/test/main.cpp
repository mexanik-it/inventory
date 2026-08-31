#include <iostream>
#include "TComputer.h"
#include "Types.h"

inline std::string opt_str(const std::optional<std::string>& s) { return s ? *s : "UNDEF"; }

int main() {
    setlocale(LC_ALL, "");
    TComputer pc;

    std::cout << "[ACTION] Requesting Network info:" << std::endl;
    auto net_info = pc.get_network();
    std::cout << " -> Hostname: " << opt_str(net_info.hostname) << std::endl;
    for (const auto& a : net_info.adapters) {
        std::cout << " -> IF: " << a.name << ", MAC: " << opt_str(a.mac_address) << std::endl;
    }
    std::cout << "\n[ACTION] Full dump via get_all():" << std::endl;
    auto full_report = pc.get_all(); 
    
    try {
        auto ram_info = std::any_cast<RAMInfo>(full_report["RAM"]);
        std::cout << " [Report] RAM Vendor: " << opt_str(ram_info.vendor) << ", Total GB: " << (ram_info.total_gb ? std::to_string(*ram_info.total_gb) : "UNDEF") << std::endl;
    } catch (...) {}

    return 0;
}