#include "BootstrapScripts/ScriptManager.hpp"
#include "BootstrapScripts/NativeStrapperApi.hpp"
#include "Logger.hpp"
#include "URIHandler/MainURIHandler.hpp"
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QString>
#include <QRegularExpression>
// woo
extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

/* just to initialize it or whatever */
namespace ScriptManager {
    std::vector<BootstrapScript> LoadedScripts;
}

// function that gets the installed scripts directory
std::string ScriptManager::GetScriptsDir() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/installed-scripts";
    QDir().mkpath(dir);
    return dir.toStdString();
}

// returns the path of a script by it's name, "sanitizes" the path too
std::string ScriptManager::GetScriptPath(const std::string &title) {
    QString safeTitle = QString::fromStdString(title).toLower().replace(" ", "-"); /* remove spaces and caps */
    safeTitle.remove(QRegularExpression("[^a-z0-9-_]")); // removes all of the other stuff
    return ScriptManager::GetScriptsDir() + "/" + safeTitle.toStdString() + ".lua";
}

// reads a string array from a lua table at the top of the stack (thank you chatgpt)
static std::vector<std::string> ReadStringArray(lua_State *L, int tableIndex) {
    std::vector<std::string> result;
    lua_pushnil(L); // first key
    while (lua_next(L, tableIndex) != 0) {
        if (lua_isstring(L, -1)) {
            result.push_back(lua_tostring(L, -1));
        }   
        lua_pop(L, 1); // pop value, keep key for next iteration
    }
    return result;
}

// LOADS and gets metadata global fields, give it a bootstrapscript struct pointer returns false if something went wrong
bool ScriptManager::ReadMetadata(const std::string &luaFilePath, BootstrapScript &out) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L); // for standard lua libraries

    // scary idk
    NativeStrapperApi::NativeStrapperRegisterLuaState(L, nullptr); /* no window */

    // load and run the file so metadata table gets defined for the thing
    if (luaL_dofile(L, luaFilePath.c_str()) != LUA_OK) {
        Logger::Log("Failed to load bootstrap script: " + std::string(lua_tostring(L, -1)), Logger::LogSeverity::SERROR, "ScriptManager");
        lua_close(L);
        return false;
    }

    // get the metadata global table /* this should still work even after registering the nativestrapper api (yeah it does) */
    lua_getglobal(L, "metadata");
    if (!lua_istable(L, -1)) {
        Logger::Log("No metadata table found in: " + luaFilePath, Logger::LogSeverity::SERROR, "ScriptManager");
        lua_close(L);
        return false;
    }

    int metaIndex = lua_gettop(L);

    // read title
    lua_getfield(L, metaIndex, "title");
    if (lua_isstring(L, -1)) {
        out.title = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    // read run command
    lua_getfield(L, metaIndex, "run");
    if (lua_isstring(L, -1)) {
        out.run = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    // read uris array
    lua_getfield(L, metaIndex, "uris");
    if (lua_istable(L, -1)) {
         out.uris = ReadStringArray(L, lua_gettop(L));
    }
    lua_pop(L, 1);

    // read platform array
    lua_getfield(L, metaIndex, "platform");
    if (lua_istable(L, -1)) {
        out.platform = ReadStringArray(L, lua_gettop(L));
    }
    lua_pop(L, 1);

    lua_getfield(L, metaIndex, "appdirectories");
    if (lua_istable(L, -1)) {
        int arrIndex = lua_gettop(L);
        lua_pushnil(L);
        while (lua_next(L, arrIndex) != 0) {
            if (lua_istable(L, -1)) {
                ScriptManager::AppDataDirectory dir;
                int dirIndex = lua_gettop(L);

                lua_getfield(L, dirIndex, "path");
                if (lua_isstring(L, -1)) {
                    dir.path = lua_tostring(L, -1);
                }
                lua_pop(L, 1);

                lua_getfield(L, dirIndex, "label");
                if (lua_isstring(L, -1)) {
                    dir.label = lua_tostring(L, -1);
                }
                lua_pop(L, 1);

                if (!dir.path.empty()) {
                    out.appdirectories.push_back(dir);
                }
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, metaIndex, "logfolders");
    if (lua_istable(L, -1)) {
        out.logfolders = ReadStringArray(L, lua_gettop(L));
    }
    lua_pop(L, 1);

    lua_getfield(L, metaIndex, "capabilities");
    if (lua_istable(L, -1)) {
        out.capabilities = ReadStringArray(L, lua_gettop(L));
    }
    lua_pop(L, 1);

    lua_close(L);

    if (out.title.empty()) {
        Logger::Log("Script has no title: " + luaFilePath, Logger::LogSeverity::SERROR, "ScriptManager");
        return false;
    }

    // why are you using this if you're not gonna provide a URI for it duh
    if (out.uris.empty()) {
        Logger::Log("Script has no URIs: " + luaFilePath, Logger::LogSeverity::SERROR, "ScriptManager");
        return false;
    }

    return true;
}

// "installs" a bootstrap script, copies it to installed-scripts folder, give it a file path
bool ScriptManager::InstallScript(const std::string &luaFilePath) {
    // read metadata first to validate the script
    BootstrapScript info;
    if (!ReadMetadata(luaFilePath, info)) return false;

    // copy lua file to scripts dir
    QString src = QString::fromStdString(luaFilePath);
    QString dest = QString::fromStdString(GetScriptPath(info.title)); /* GetScriptPath() sanitizes the file path btw */

    if (QFile::exists(dest)) {  /* check if it exists */
        Logger::Log("A script with the same title is already installed.", Logger::LogSeverity::SERROR, "ScriptManager");
        return false;
    }

    if (!QFile::copy(src, dest)) {
        Logger::Log("Failed to copy script to: " + dest.toStdString(), Logger::LogSeverity::SERROR, "ScriptManager");
        return false;
    }

    // yep
    URIHandler::InstallURIs(info.title, info.uris);
    
    // push it to loadedscripts
    LoadedScripts.push_back(info);

    Logger::Log("Installed script: " + info.title, Logger::LogSeverity::SSUCCESS, "ScriptManager");
    return true;
}

// "uninstalls an installed script" just deletes the file from installed-scripts lol, give it the title of the bootstrap script 
bool ScriptManager::UninstallScript(const std::string &title) {
    // find the script struct
    auto it = std::find_if(LoadedScripts.begin(), LoadedScripts.end(), [&](const BootstrapScript &s) { return s.title == title; });

    if (it == LoadedScripts.end()) {
        Logger::Log("Script not found: " + title, Logger::LogSeverity::SERROR, "ScriptManager");
        return false;
    }

    // uninstall URIs before erasing
    URIHandler::UninstallURIs(title, it->uris);

    // delete the lua file
    QString path = QString::fromStdString(GetScriptPath(title));
    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    // now erase from list
    LoadedScripts.erase(it);

    Logger::Log("Uninstalled script: " + title, Logger::LogSeverity::SSUCCESS, "ScriptManager");
    return true;
}

void ScriptManager::LoadScripts(bool reinstallURIs) {
    LoadedScripts.clear();
    QString scriptsDir = QString::fromStdString(GetScriptsDir());
    QDir dir(scriptsDir);

    for (const QString &filename : dir.entryList({"*.lua"}, QDir::Files)) {
        std::string fullPath = (scriptsDir + "/" + filename).toStdString();
        BootstrapScript info;
        if (ReadMetadata(fullPath, info)) {
            LoadedScripts.push_back(info);
            Logger::Log("Loaded script: " + info.title, Logger::LogSeverity::SINFO, "ScriptManager");
        }
    }

    if (!reinstallURIs) return;
    
    // reinstall all URIs at launch incase roblox default bootstrapper/sober or whatever breaks them
    for (const auto &script : ScriptManager::LoadedScripts) {
        URIHandler::InstallURIs(script.title, script.uris);
    }
}

ScriptManager::BootstrapScript* ScriptManager::FindFirstScriptByName(std::string scriptName) {
    QString safeScriptName = QString::fromStdString(scriptName).toLower().replace(" ", "-");
    for (auto &script : ScriptManager::LoadedScripts) {
        QString safeTitle = QString::fromStdString(script.title).toLower().replace(" ", "-");
        if (safeTitle == safeScriptName) return &script;
    }
    return NULL; // nuh
}