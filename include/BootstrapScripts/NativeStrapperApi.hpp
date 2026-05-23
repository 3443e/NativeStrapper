#pragma once
#include <map>
#include "UserInterface/BootstrapWindow.hpp"
#include "NetworkManagement.hpp"
#include "ShellUtils.hpp"
#include "Logger.hpp"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

namespace NativeStrapperApi {
    // need these to access the pointer later
    static void SetWindow(lua_State *L, BootstrapWindow *window) {
        lua_pushlightuserdata(L, (void*)window);
        lua_setfield(L, LUA_REGISTRYINDEX, "ns_window");
    }

    static BootstrapWindow* GetWindow(lua_State *L) {
        lua_getfield(L, LUA_REGISTRYINDEX, "ns_window");
        BootstrapWindow *w = (BootstrapWindow*)lua_touserdata(L, -1);
        lua_pop(L, 1);
        return w;
    }

    /* simple function, logs stuff as LuaScript (useless, you can use print btw) */
    static int NativeStrapperLog(lua_State *L) {
        const char *msg = luaL_checkstring(L, 1);
        Logger::Log(msg, Logger::LogSeverity::SINFO, "LuaScript");
        return 0;
    }

    // sets the bootstrapwindow status text to whatever
    static int NativeStrapperSetStatus(lua_State *L) {
        const char *msg = luaL_checkstring(L, 1);
        BootstrapWindow *w = GetWindow(L);
        if (w) {
            w->setStatus(QString::fromUtf8(msg));
            QApplication::processEvents();
        }
        return 0;
    }

    // runs a command and returns the thing ye
    static int NativeStrapperRun(lua_State *L) {
        const char *cmd = luaL_checkstring(L, 1);
        if (!cmd) {
            lua_pushnil(L);
            lua_pushstring(L, "command is null");
            return 2;
        }
        std::string cmdStr(cmd);
        std::string result = ShellUtils::RunCommand(cmdStr);
        lua_pushstring(L, result.c_str());
        return 1;
    }

    // NativeStrapper.request({url = "...", headers = {["key"] = "val"}})
    // returns {body = "...", status = 200}
    static int NativeStrapperRequest(lua_State *L) {
        luaL_checktype(L, 1, LUA_TTABLE);

        // read url
        lua_getfield(L, 1, "url");
        const char *url = luaL_checkstring(L, -1);
        lua_pop(L, 1);

        // read optional headers
        std::map<std::string, std::string> headers;
        lua_getfield(L, 1, "headers");
        if (lua_istable(L, -1)) {
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                if (lua_isstring(L, -2) && lua_isstring(L, -1)) {
                    headers[lua_tostring(L, -2)] = lua_tostring(L, -1);
                }
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);

        std::string body = NetworkManagement::FetchURL(std::string(url), headers);

        lua_newtable(L);
        lua_pushstring(L, body.c_str());
        lua_setfield(L, -2, "body");
        lua_pushinteger(L, body.empty() ? 0 : 200);
        lua_setfield(L, -2, "status");

        return 1;
    }

    // NativeStrapper.download("url", "out dir", "optional label used in the bootstrapper window thing")
    // returns true on success, false on failure (this is so bad)
    static int NativeStrapperDownload(lua_State *L) {
        const char *url = luaL_checkstring(L, 1);
        const char *out = luaL_checkstring(L, 2);
        const char *label = lua_isstring(L, 3) ? lua_tostring(L, 3) : "Downloading...";

        BootstrapWindow *w = GetWindow(L);

        // deepseek i love you
        std::string path = NetworkManagement::DownloadFile(
            std::string(url),
            std::string(out),
            [w, label](long long downloaded, long long total) {
                if (total <= 0) return;
                int pct = (int)((double)downloaded / (double)total * 100);
                double mb = (double)downloaded / 1024.0 / 1024.0;
                double mbTotal = (double)total / 1024.0 / 1024.0;

                Logger::Log("Downloading " + std::string(label) + " - " + std::to_string((int)mb) + " / " + std::to_string((int)mbTotal) + " MB (" + std::to_string(pct) + "%)", Logger::LogSeverity::SINFO, "Download");

                if (w) {
                    QMetaObject::invokeMethod(w, [w, pct]() {w->setProgress(pct);}, Qt::QueuedConnection);
                }
            }
        );

        lua_pushboolean(L, !path.empty());
        return 1;
    }

    static void NativeStrapperRegisterLuaState(lua_State *L, BootstrapWindow *window) {
        SetWindow(L, window);

        // NativeStrapperAPI functions
        /*----------------------------------------------------------------*/
        lua_newtable(L);

        lua_pushcfunction(L, NativeStrapperLog);
        lua_setfield(L, -2, "log");

        lua_pushcfunction(L, NativeStrapperSetStatus);
        lua_setfield(L, -2, "setStatus");

        lua_pushcfunction(L, NativeStrapperRun);
        lua_setfield(L, -2, "run");

        lua_pushcfunction(L, NativeStrapperRequest);
        lua_setfield(L, -2, "request");

        lua_pushcfunction(L, NativeStrapperDownload);
        lua_setfield(L, -2, "download");

        // NativeStrapperAPI Constants (if anyone is reading this, i couldn't figure 
        // out how to ACTUALLY make them constant, so please if you know, tell me)
        /*----------------------------------------------------------------*/

        lua_newtable(L);

        QString home = QDir::homePath();
        QString user = qgetenv("USER");
        if (user.isEmpty()) {
            user = qgetenv("LOGNAME");
        }
        if (user.isEmpty()) {
            user = qgetenv("USERNAME");
        } // windows

        lua_pushstring(L, user.toStdString().c_str());
        lua_setfield(L, -2, "USER");

        lua_pushstring(L, home.toStdString().c_str());
        lua_setfield(L, -2, "HOME");

#ifdef __linux__
        lua_pushstring(L, "Linux");
#elif _WIN32
        lua_pushstring(L, "Windows");
#elif __APPLE__
        lua_pushstring(L, "macOS");
#else
        lua_pushstring(L, "Unknown");
#endif
        lua_setfield(L, -2, "PLATFORM");

        lua_setfield(L, -2, "Constants");

        lua_setglobal(L, "NativeStrapper");
    }
}