#include "BootstrapScripts/LuaScript.hpp"
#include "BootstrapScripts/NativeStrapperApi.hpp"
#include "Logger.hpp"
#include <QApplication>

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

bool LuaScript::RunScript(const std::string &scriptPath, const std::string &uri, BootstrapWindow *window) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    /* registers all Nativestrapper. functions */
    NativeStrapperApi::NativeStrapperRegisterLuaState(L, window);

    // https://github.com/rxi/json.lua/blob/master/json.lua
    std::string jsonPath = QCoreApplication::applicationDirPath().toStdString() + "/assets/NativeStrapperAPI/json.lua";
    if (luaL_loadfile(L, jsonPath.c_str()) != LUA_OK) {
        Logger::Log("Failed to load json.lua: " + std::string(lua_tostring(L, -1)), Logger::LogSeverity::SERROR, "LuaScript");
    } else {
        lua_pcall(L, 0, 1, 0);
        lua_setglobal(L, "NativeStrapperJson");
    }

    // load the file
    if (luaL_dofile(L, scriptPath.c_str()) != LUA_OK) {
        Logger::Log("Failed to load script: " + std::string(lua_tostring(L, -1)), Logger::LogSeverity::SERROR, "LuaScript");
        lua_close(L);
        return false;
    }

    // get bootstrap function
    lua_getglobal(L, "bootstrap");
    if (!lua_isfunction(L, -1)) {
        Logger::Log("No bootstrap function found in script", Logger::LogSeverity::SERROR, "LuaScript");
        lua_close(L);
        return false;
    }

    // push uri as argument and call bootstrap(uri)
    lua_pushstring(L, uri.c_str());
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        Logger::Log("Script error: " + std::string(lua_tostring(L, -1)), Logger::LogSeverity::SERROR, "LuaScript");
        lua_close(L);
        return false;
    }

    lua_close(L);
    return true;
}