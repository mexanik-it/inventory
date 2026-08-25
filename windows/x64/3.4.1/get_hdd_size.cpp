#include <iostream>
#include <cstdio>
#include "main.h"

#include <windows.h>
#include <winioctl.h>
#include <iostream>
//#pragma comment(lib, "wbemuuid.lib")
//#pragma comment(lib, "ole32.lib")
//#pragma comment(lib, "oleaut32.lib")

/***************************************************************************************************/
/* возвращает общий объём жесткого диска                                                           */
/***************************************************************************************************/

#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <windows.h>
#include <winioctl.h>

bool TInventory::get_hdd_size() {
  uint64_t totalBytes = 0;
  bool foundAny = false;

  for (int i = 0; i < 32; ++i) {
    char path[64];
    std::sprintf(path, "\\\\.\\PhysicalDrive%d", i);

    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) continue;

    DISK_GEOMETRY geo{};
    DWORD bytesReturned = 0;
    BOOL ok = DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0,
                              &geo, sizeof(geo), &bytesReturned, nullptr);
    CloseHandle(h);

    if (!ok) continue;

    uint64_t sizeBytes = geo.Cylinders.QuadPart * geo.TracksPerCylinder *
                         geo.SectorsPerTrack * geo.BytesPerSector;
    totalBytes += sizeBytes;
    foundAny = true;
  }

  if (!foundAny || totalBytes == 0) {
    std::cerr << "Couldn't determine disk size via WinAPI.\n";
    return false;
  }

  const uint64_t bytesPerGb = 1000ULL * 1000 * 1000;
  uint64_t gigs = totalBytes / bytesPerGb;
  id_hdd_size = std::to_string(gigs) + "Gb";
  return true;
}
