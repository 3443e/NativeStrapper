#pragma once
#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>

inline void ApplyDarkTitleBar(QWidget *widget) {
    HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
    DwmSetWindowAttribute(hwnd, 19, &dark, sizeof(dark));
}
#endif