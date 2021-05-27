#include <windows.h>

#include "Desktop.h"
#include "debug.h"

void Desktop::updateRect(const Rect rect) { this->rect = rect; }

Desktop::Desktop(HMONITOR monitor) {
    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfoW(monitor, &monitor_info)) {
        debug("Failed to get monitor.");
        return;
    }

    this->rect = {
        {monitor_info.rcWork.left, monitor_info.rcWork.top},
        {monitor_info.rcWork.right - monitor_info.rcWork.left, monitor_info.rcWork.bottom - monitor_info.rcWork.top},
    };
}
