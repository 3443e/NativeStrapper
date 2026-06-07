// Windows implementation of the platform default handler API.
// Implements the interface declared in platform_default_handler.h
//
// Hooks the roblox:// and roblox-player:// URI schemes so that browsers
// launch this executable instead of the official Roblox client, mirroring
// the approach used by alternative launchers such as Bloxstrap.
//
// Registry layout (HKCU — no admin rights required; shadows HKCR):
//
//   HKCU\Software\Classes\roblox\
//       (Default)    = "URL:roblox Protocol"
//       URL Protocol = ""
//     \shell\open\command\
//       (Default)    = "<exe>" "%1"
//
//   HKCU\Software\Classes\roblox-player\   (identical structure)
//
// Call setAsDefaultHandler("roblox") and setAsDefaultHandler("roblox-player")
// on first run.  The %1 argument your exe receives will be the full URI, e.g.:
//   roblox-player://1+launchmode:play+gameinfo:...+placeId:...

#include "MainURIHandler.hpp"
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>

namespace platform {

// ── internal helpers ─────────────────────────────────────────────────────────

// (conversion helpers removed; conversions are inlined where needed)

static bool RegSetSZ(HKEY hKey, const wchar_t* valueName, const std::wstring& data) {
    DWORD cb = static_cast<DWORD>((data.size() + 1) * sizeof(wchar_t));
    return RegSetValueExW(hKey, valueName, 0, REG_SZ,
                          reinterpret_cast<const BYTE*>(data.c_str()), cb) == ERROR_SUCCESS;
}

static HKEY RegCreateWrite(const std::wstring& subKey) {
    HKEY hKey = nullptr;
    LONG rc = RegCreateKeyExW(HKEY_CURRENT_USER, subKey.c_str(),
                              0, nullptr, REG_OPTION_NON_VOLATILE,
                              KEY_WRITE, nullptr, &hKey, nullptr);
    return (rc == ERROR_SUCCESS) ? hKey : nullptr;
}

// ── public API ────────────────────────────────────────────────────────────────

// Register the current executable as the handler for `scheme`.
// Intended for use with the Roblox URI schemes "roblox" and "roblox-player".
// Written under HKCU so no administrator privileges are needed; HKCU takes
// precedence over the official client's HKCR entries.
// Returns true on success.
bool SetAsDefaultHandler(const std::string& scheme) {
    wchar_t exePath[MAX_PATH] = {};
    DWORD pathLen = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    if (pathLen == 0 || pathLen == MAX_PATH) return false;

    // convert UTF-8 `scheme` to wide string
    std::wstring ws;
    if (!scheme.empty()) {
        int len = MultiByteToWideChar(CP_UTF8, 0, scheme.c_str(), -1, nullptr, 0);
        if (len > 0) {
            std::vector<wchar_t> buf(len);
            MultiByteToWideChar(CP_UTF8, 0, scheme.c_str(), -1, buf.data(), len);
            ws = std::wstring(buf.data());
        }
    }
    const std::wstring appPath(exePath);
    const std::wstring base    = L"Software\\Classes\\" + ws;

    // ── root key ──────────────────────────────────────────────────────────
    {
        HKEY hk = RegCreateWrite(base);
        if (!hk) return false;

        // Friendly name shown in "Open with" dialogs
        RegSetSZ(hk, nullptr, L"URL:" + ws + L" Protocol");

        // Presence of this value (even empty) marks the key as a URI scheme,
        // not a file-type association — required for browser hand-off.
        RegSetSZ(hk, L"URL Protocol", L"");

        RegCloseKey(hk);
    }

    // ── shell\open\command ────────────────────────────────────────────────
    {
        HKEY hk = RegCreateWrite(base + L"\\shell\\open\\command");
        if (!hk) return false;

        // %1 is replaced by Windows with the full roblox:// / roblox-player://
        // URI that the browser passes along.  Quote the exe path to handle
        // spaces in paths such as C:\Program Files\...
        RegSetSZ(hk, nullptr, L"\"" + appPath + L"\" \"%1\"");

        RegCloseKey(hk);
    }

    // If registration succeeded but Windows still reports a different
    // handler (UserChoice may override programmatic registration), open
    // the Default Apps settings so the user can set our app manually.
    try {
        std::string active = currentDefaultHandler(scheme);

        // convert appPath (wide) to UTF-8 for comparison
        std::string appPathUtf;
        if (!appPath.empty()) {
            int len = WideCharToMultiByte(CP_UTF8, 0, appPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                std::vector<char> buf(len);
                WideCharToMultiByte(CP_UTF8, 0, appPath.c_str(), -1, buf.data(), len, nullptr, nullptr);
                appPathUtf = std::string(buf.data());
            }
        }

        if (active.empty() || active != appPathUtf) {
            ShellExecuteW(nullptr, L"open", L"ms-settings:defaultapps", nullptr, nullptr, SW_SHOWNORMAL);
        }
    } catch (...) {
        // best-effort only; ignore failures
    }

    return true;
}

// Returns the path of the current handler executable for `scheme`, or "" if
// none is registered.  Checks HKCU first (custom launchers), then HKCR
// (official Roblox client), mirroring Windows URI resolution order.
std::string currentDefaultHandler(const std::string& scheme) {
    // convert UTF-8 `scheme` to wide string
    std::wstring ws;
    if (!scheme.empty()) {
        int len = MultiByteToWideChar(CP_UTF8, 0, scheme.c_str(), -1, nullptr, 0);
        if (len > 0) {
            std::vector<wchar_t> buf(len);
            MultiByteToWideChar(CP_UTF8, 0, scheme.c_str(), -1, buf.data(), len);
            ws = std::wstring(buf.data());
        }
    }

    // Search order: HKCU shadows HKCR, same as Windows itself.
    struct Candidate { HKEY root; std::wstring subKey; };
    const Candidate candidates[] = {
        { HKEY_CURRENT_USER, L"Software\\Classes\\" + ws + L"\\shell\\open\\command" },
        { HKEY_CLASSES_ROOT, ws + L"\\shell\\open\\command" },
    };

    for (const auto& c : candidates) {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(c.root, c.subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
            continue;

        DWORD type = 0, cbData = 0;
        LONG rc = RegQueryValueExW(hKey, nullptr, nullptr, &type, nullptr, &cbData);
        if (rc != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ)) {
            RegCloseKey(hKey);
            continue;
        }

        std::vector<wchar_t> buf(cbData / sizeof(wchar_t) + 1);
        rc = RegQueryValueExW(hKey, nullptr, nullptr, nullptr,
                              reinterpret_cast<LPBYTE>(buf.data()), &cbData);
        RegCloseKey(hKey);
        if (rc != ERROR_SUCCESS) continue;

        std::wstring cmd(buf.data());

        // Command is stored as: "<full\path\to\exe>" "%1"
        // Extract the path between the first pair of double-quotes.
        std::wstring resultPath;
        size_t first = cmd.find(L'"');
        if (first != std::wstring::npos) {
            size_t second = cmd.find(L'"', first + 1);
            if (second != std::wstring::npos && second > first + 1)
                resultPath = cmd.substr(first + 1, second - first - 1);
        }
        if (resultPath.empty()) resultPath = cmd;  // unquoted fallback

        if (!resultPath.empty()) {
            // convert wide resultPath to UTF-8
            int len = WideCharToMultiByte(CP_UTF8, 0, resultPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                std::vector<char> buf(len);
                WideCharToMultiByte(CP_UTF8, 0, resultPath.c_str(), -1, buf.data(), len, nullptr, nullptr);
                return std::string(buf.data());
            }
        }
    }

    return {};
}

} // namespace platform