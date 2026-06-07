#pragma once

// roblox_launcher_setup_fixed.hpp
// Builds and registers a macOS .app bundle that intercepts roblox:// and
// roblox-player:// URI schemes and forwards them to nativestrapper.
//

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
// Config
// ─────────────────────────────────────────────────────────────────────────────

struct LauncherConfig {
    std::string appName  = "NativeStrapper URI Handler";
    std::string bundleId = "com.scene.nativestrapper.urihandler";
    std::string execName = "NativeStrapperURIHandler";
    std::string version  = "1.0";
    std::string bootstrapScriptTitle;
    std::vector<std::string> uriSchemes = {"roblox", "roblox-player"};

    // Candidate paths for nativestrapper, tried in order by the generated app.
    // Supported placeholders: $HOME, $APP_PARENT_DIR, $EXEC_DIR.
    std::vector<std::string> nativestrapperPaths = {
        // Beside the .app, e.g. ~/Applications/nativestrapper
        "$APP_PARENT_DIR/nativestrapper",
        "$APP_PARENT_DIR/nativestrapper.app/Contents/MacOS/nativestrapper",
        "$APP_PARENT_DIR/NativeStrapper.app/Contents/MacOS/nativestrapper",
        // Same directory as the app executable, e.g. RobloxLauncher.app/Contents/MacOS
        "$EXEC_DIR/nativestrapper",
        // Explicit home-dir shorthand
        "$HOME/nativestrapper",
        // Common install locations
        "/usr/local/bin/nativestrapper",
        "/opt/homebrew/bin/nativestrapper",
        "/usr/bin/nativestrapper",
    };

    // Where to install the .app; defaults to ~/Applications.
    std::optional<fs::path> installDir;

    // If true, claim the URI schemes even if another Roblox handler is present.
    bool forceDefaultHandler = true;

    // If true, keep the generated Objective-C++ launcher source in Resources.
    bool keepGeneratedSource = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

namespace detail {

inline fs::path homeDir() {
    const char* h = ::getenv("HOME");
    if (!h || h[0] == '\0') {
        throw std::runtime_error("$HOME is not set");
    }
    return fs::path(h);
}

inline void runCmd(const std::string& cmd) {
    int rc = ::system(cmd.c_str());
    if (rc != 0) {
        throw std::runtime_error("Command failed (exit " + std::to_string(rc) + "): " + cmd);
    }
}

inline std::string shellQuote(const fs::path& p) {
    std::string s = p.string();
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out += c;
    }
    out += "'";
    return out;
}

inline void stripQuarantine(const fs::path& root) {
    if (!fs::exists(root)) return;

    for (auto& entry : fs::recursive_directory_iterator(
             root, fs::directory_options::skip_permission_denied)) {
        ::removexattr(entry.path().c_str(), "com.apple.quarantine", XATTR_NOFOLLOW);
    }
    ::removexattr(root.c_str(), "com.apple.quarantine", XATTR_NOFOLLOW);
}

inline std::string xmlEscape(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default:   out += c;        break;
        }
    }
    return out;
}

inline std::string cStringLiteral(std::string_view s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20 || c >= 0x7f) {
                    char buf[5];
                    std::snprintf(buf, sizeof(buf), "\\x%02x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    out += "\"";
    return out;
}

struct CFStr {
    CFStringRef ref = nullptr;

    explicit CFStr(const std::string& s)
        : ref(CFStringCreateWithCString(kCFAllocatorDefault,
                                        s.c_str(),
                                        kCFStringEncodingUTF8)) {
        if (!ref) {
            throw std::runtime_error("CFStringCreateWithCString failed for: " + s);
        }
    }

    ~CFStr() { if (ref) CFRelease(ref); }
    CFStr(const CFStr&) = delete;
    CFStr& operator=(const CFStr&) = delete;
    operator CFStringRef() const { return ref; }
};

struct CFUrl {
    CFURLRef ref = nullptr;

    explicit CFUrl(const fs::path& p) {
        CFStr s(p.string());
        ref = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                            s,
                                            kCFURLPOSIXPathStyle,
                                            true);
        if (!ref) {
            throw std::runtime_error("CFURLCreateWithFileSystemPath failed for: " + p.string());
        }
    }

    ~CFUrl() { if (ref) CFRelease(ref); }
    CFUrl(const CFUrl&) = delete;
    CFUrl& operator=(const CFUrl&) = delete;
    operator CFURLRef() const { return ref; }
};

} // namespace detail

// ─────────────────────────────────────────────────────────────────────────────
// Step 1 — Directory tree
// ─────────────────────────────────────────────────────────────────────────────

inline void createBundleDirectories(const fs::path& appPath) {
    fs::create_directories(appPath / "Contents" / "MacOS");
    fs::create_directories(appPath / "Contents" / "Resources");
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 2 — Info.plist
// ─────────────────────────────────────────────────────────────────────────────

inline void writeInfoPlist(const fs::path& appPath, const LauncherConfig& cfg) {
    auto x = [](const std::string& s) { return detail::xmlEscape(s); };

    std::ostringstream o;
    o << R"PLIST(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>)PLIST" << x(cfg.appName) << R"PLIST(</string>
    <key>CFBundleDisplayName</key>
    <string>)PLIST" << x(cfg.appName) << R"PLIST(</string>
    <key>CFBundleIdentifier</key>
    <string>)PLIST" << x(cfg.bundleId) << R"PLIST(</string>
    <key>CFBundleVersion</key>
    <string>)PLIST" << x(cfg.version) << R"PLIST(</string>
    <key>CFBundleShortVersionString</key>
    <string>)PLIST" << x(cfg.version) << R"PLIST(</string>
    <key>CFBundleExecutable</key>
    <string>)PLIST" << x(cfg.execName) << R"PLIST(</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>

    <!-- Run as a background agent: no Dock icon, no menu bar. -->
    <key>LSUIElement</key>
    <true/>
    <key>LSApplicationCategoryType</key>
    <string>public.app-category.utilities</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.15</string>
    <key>NSHighResolutionCapable</key>
    <true/>

    <key>CFBundleURLTypes</key>
    <array>
        <dict>
            <key>CFBundleURLName</key>
            <string>)PLIST" << x(cfg.bundleId) << R"PLIST(</string>
            <key>CFBundleURLSchemes</key>
            <array>
)PLIST";

    for (const auto& scheme : cfg.uriSchemes) {
        o << "                <string>" << x(scheme) << "</string>\n";
    }

    o << R"PLIST(            </array>
            <key>CFBundleTypeRole</key>
            <string>Viewer</string>
            <key>LSHandlerRank</key>
            <string>Owner</string>
        </dict>
    </array>
</dict>
</plist>
)PLIST";

    fs::path out = appPath / "Contents" / "Info.plist";
    std::ofstream f(out);
    if (!f) throw std::runtime_error("Cannot write Info.plist: " + out.string());
    f << o.str();
    if (!f) throw std::runtime_error("Write error for Info.plist: " + out.string());
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 3 — Native Cocoa URL-handler executable
// ─────────────────────────────────────────────────────────────────────────────

inline void writeNativeLauncherSource(const fs::path& sourcePath, const LauncherConfig& cfg) {
    std::ofstream f(sourcePath);
    if (!f) throw std::runtime_error("Cannot write launcher source: " + sourcePath.string());

    f << R"OBJCXX(#import <Cocoa/Cocoa.h>
#include <spawn.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern char** environ;

static bool gHandledURL = false;
static std::ofstream gLog;

static std::string nsToStd(NSString* s) {
    if (!s) return std::string();
    const char* u = [s UTF8String];
    return u ? std::string(u) : std::string();
}

static void openLog() {
    NSString* home = NSHomeDirectory();
    NSString* logDir = [home stringByAppendingPathComponent:@"Library/Logs/RobloxLauncher"];
    [[NSFileManager defaultManager] createDirectoryAtPath:logDir
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];
    NSString* logPath = [logDir stringByAppendingPathComponent:@"launcher.log"];
    gLog.open([logPath fileSystemRepresentation], std::ios::app);
}

static void logLine(const std::string& s) {
    if (gLog) {
        gLog << s << "\n";
        gLog.flush();
    }
}

static std::string nowString() {
    NSDateFormatter* fmt = [[NSDateFormatter alloc] init];
    [fmt setDateFormat:@"yyyy-MM-dd HH:mm:ss"];
    return nsToStd([fmt stringFromDate:[NSDate date]]);
}

static bool startsWith(const std::string& s, const std::string& prefix) {
    return s.rfind(prefix, 0) == 0;
}

static void replaceAll(std::string& s, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    std::size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
}

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) out.push_back(item);
    return out;
}

static bool isExecutable(const std::string& path) {
    struct stat st{};
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode) && ::access(path.c_str(), X_OK) == 0;
}

static std::string dirnameOf(NSString* path) {
    if (!path) return std::string();
    return nsToStd([path stringByDeletingLastPathComponent]);
}

static std::string expandCandidate(std::string candidate) {
    std::string home = nsToStd(NSHomeDirectory());
    std::string execDir = dirnameOf([[NSBundle mainBundle] executablePath]);
    std::string appParentDir = dirnameOf([[NSBundle mainBundle] bundlePath]);

    replaceAll(candidate, "$HOME", home);
    if (candidate == "~") candidate = home;
    else if (startsWith(candidate, "~/")) candidate = home + candidate.substr(1);
    replaceAll(candidate, "$EXEC_DIR", execDir);
    replaceAll(candidate, "$APP_PARENT_DIR", appParentDir);

    // Compatibility with the old config string from the shell-script version.
    replaceAll(candidate, "$(dirname \"$0\")", execDir);
    replaceAll(candidate, "$(dirname '$0')", execDir);

    return candidate;
}

static const char* kCandidates[] = {
)OBJCXX";

    for (const auto& p : cfg.nativestrapperPaths) {
        f << "    " << detail::cStringLiteral(p) << ",\n";
    }

    f << R"OBJCXX(};

static const char* kBootstrapScript = )OBJCXX" << detail::cStringLiteral(cfg.bootstrapScriptTitle) << R"OBJCXX(;

static std::string findNativestrapper() {
    for (const char* raw : kCandidates) {
        std::string expanded = expandCandidate(raw ? raw : "");
        if (isExecutable(expanded)) return expanded;
    }
    return std::string();
}

static bool spawnProcess(const std::string& exe, const std::vector<std::string>& args) {
    std::vector<std::string> owned;
    owned.reserve(args.size() + 1);
    owned.push_back(exe);
    owned.insert(owned.end(), args.begin(), args.end());

    std::vector<char*> argv;
    argv.reserve(owned.size() + 1);
    for (std::string& s : owned) argv.push_back(s.data());
    argv.push_back(nullptr);

    pid_t pid = 0;
    int rc = ::posix_spawn(&pid, exe.c_str(), nullptr, nullptr, argv.data(), environ);
    if (rc != 0) {
        logLine("ERROR: posix_spawn failed: " + std::string(std::strerror(rc)));
        return false;
    }

    logLine("spawned pid: " + std::to_string(static_cast<long long>(pid)));
    return true;
}

static void showAlert(const std::string& message) {
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Roblox Launcher"];
    [alert setInformativeText:[NSString stringWithUTF8String:message.c_str()]];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert runModal];
}

static void terminateSoon() {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.25 * NSEC_PER_SEC)),
                   dispatch_get_main_queue(), ^{
                       [NSApp terminate:nil];
                   });
}

static void dispatchURI(const std::string& uri) {
    logLine("──────────────────────────────────────────");
    logLine(nowString() + " launcher invoked");
    logLine("uri: " + uri);

    std::string strapper = findNativestrapper();
    if (strapper.empty()) {
        logLine("ERROR: nativestrapper not found in any candidate path.");
        for (const char* raw : kCandidates) {
            logLine("searched: " + expandCandidate(raw ? raw : ""));
        }
        showAlert("nativestrapper could not be found. Please re-install or update the launcher config.");
        return;
    }

    logLine("strapper: " + strapper);

    if (std::strlen(kBootstrapScript) == 0) {
        spawnProcess(strapper, {uri});
        return;
    }

    if (startsWith(uri, "roblox://")) {
        spawnProcess(strapper, {"--bootstrap-script", kBootstrapScript, uri});
        return;
    }

    if (startsWith(uri, "roblox-player://")) {
        spawnProcess(strapper, {"--bootstrap-script", kBootstrapScript, uri});
        return;
    }

    logLine("WARNING: unrecognized URI scheme");
}

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate

- (instancetype)init {
    self = [super init];
    if (self) {
        [[NSAppleEventManager sharedAppleEventManager]
            setEventHandler:self
                andSelector:@selector(handleGetURLEvent:withReplyEvent:)
              forEventClass:kInternetEventClass
                 andEventID:kAEGetURL];
    }
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    // If the app was opened manually, do not keep an invisible process alive forever.
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3.0 * NSEC_PER_SEC)),
                   dispatch_get_main_queue(), ^{
                       if (!gHandledURL) [NSApp terminate:nil];
                   });
}

- (void)handleGetURLEvent:(NSAppleEventDescriptor*)event
           withReplyEvent:(NSAppleEventDescriptor*)replyEvent {
    gHandledURL = true;
    NSString* urlString = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
    if (!urlString || [urlString length] == 0) {
        logLine("WARNING: received empty URL event");
        terminateSoon();
        return;
    }

    dispatchURI(nsToStd(urlString));
    terminateSoon();
}

@end

int main(int argc, char** argv) {
    @autoreleasepool {
        openLog();
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

        AppDelegate* delegate = [[AppDelegate alloc] init];
        [NSApp setDelegate:delegate];
        [NSApp run];
    }
    return 0;
}
)OBJCXX";

    if (!f) throw std::runtime_error("Write error for launcher source: " + sourcePath.string());
}

inline void compileNativeLauncher(const fs::path& appPath, const LauncherConfig& cfg) {
    fs::path sourcePath = appPath / "Contents" / "Resources" / "launcher.mm";
    fs::path execPath   = appPath / "Contents" / "MacOS" / cfg.execName;

    writeNativeLauncherSource(sourcePath, cfg);

    std::string cmd =
        "xcrun clang++ -std=c++17 -fobjc-arc "
        "-framework Cocoa "
        "-o " + detail::shellQuote(execPath) + " " + detail::shellQuote(sourcePath);

    detail::runCmd(cmd);

    if (::chmod(execPath.c_str(), 0755) != 0) {
        throw std::runtime_error("chmod failed for: " + execPath.string());
    }

    if (!cfg.keepGeneratedSource) {
        std::error_code ec;
        fs::remove(sourcePath, ec);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Step 4 — Launch Services registration
// ─────────────────────────────────────────────────────────────────────────────

inline void registerWithLaunchServices(const fs::path& appPath) {
    detail::CFUrl url(appPath);
    OSStatus st = LSRegisterURL(url, true);
    if (st != noErr) {
        throw std::runtime_error("LSRegisterURL failed (OSStatus " +
                                 std::to_string(st) + ") for: " + appPath.string());
    }
}

inline void forceDefaultHandler(const std::string& bundleId,
                                const std::vector<std::string>& schemes) {
    void* cs = ::dlopen("/System/Library/Frameworks/CoreServices.framework/CoreServices",
                        RTLD_LAZY | RTLD_LOCAL);
    if (!cs) {
        std::fprintf(stderr, "WARN: dlopen CoreServices failed — skipping forceDefaultHandler\n");
        return;
    }

    using SetHandlerFn = OSStatus (*)(CFStringRef scheme, CFStringRef bundleId);
    using CopyHandlerFn = CFStringRef (*)(CFStringRef scheme);
    auto setHandler = reinterpret_cast<SetHandlerFn>(
        ::dlsym(cs, "LSSetDefaultHandlerForURLScheme"));
    auto copyHandler = reinterpret_cast<CopyHandlerFn>(
        ::dlsym(cs, "LSCopyDefaultHandlerForURLScheme"));

    if (!setHandler) {
        std::fprintf(stderr, "WARN: LSSetDefaultHandlerForURLScheme not found — skipping\n");
        ::dlclose(cs);
        return;
    }

    detail::CFStr bid(bundleId);
    for (const auto& scheme : schemes) {
        detail::CFStr sch(scheme);
        OSStatus st = setHandler(sch, bid);
        if (st != noErr) {
            std::fprintf(stderr,
                         "WARN: LSSetDefaultHandlerForURLScheme(%s) → %d\n",
                         scheme.c_str(),
                         static_cast<int>(st));
        } else {
            std::printf("  Default handler set: %s:// → %s\n",
                        scheme.c_str(),
                        bundleId.c_str());
        }

        if (copyHandler) {
            CFStringRef current = copyHandler(sch);
            if (current) {
                char currentBundle[512] = {};
                CFStringGetCString(current,
                                    currentBundle,
                                    sizeof(currentBundle),
                                    kCFStringEncodingUTF8);
                CFRelease(current);

                if (bundleId != currentBundle) {
                    std::fprintf(stderr,
                                 "WARN: Launch Services still reports %s:// → %s\n",
                                 scheme.c_str(),
                                 currentBundle);
                }
            }
        }
    }

    ::dlclose(cs);
}

// ─────────────────────────────────────────────────────────────────────────────
// Top-level build function
// ─────────────────────────────────────────────────────────────────────────────

inline void buildLauncherApp(const LauncherConfig& cfg = LauncherConfig{}) {
    fs::path installDir = cfg.installDir.value_or(detail::homeDir() / "Applications");
    fs::path appPath = installDir / (cfg.appName + ".app");

    std::printf("Building %s\n  → %s\n", cfg.appName.c_str(), appPath.c_str());

    std::error_code ec;
    if (fs::exists(appPath)) {
        fs::remove_all(appPath, ec);
        if (ec) throw std::runtime_error("Cannot remove existing bundle: " + ec.message());
        std::printf("  Removed existing bundle.\n");
    }

    std::printf("  Creating directory tree...\n");
    createBundleDirectories(appPath);

    std::printf("  Writing Info.plist...\n");
    writeInfoPlist(appPath, cfg);

    std::printf("  Building native URL handler...\n");
    compileNativeLauncher(appPath, cfg);

    std::printf("  Stripping quarantine flags...\n");
    detail::stripQuarantine(appPath);

    std::printf("  Registering with Launch Services...\n");
    registerWithLaunchServices(appPath);

    if (cfg.forceDefaultHandler) {
        std::printf("  Claiming URI schemes...\n");
        forceDefaultHandler(cfg.bundleId, cfg.uriSchemes);
    }

    std::printf("✅ Done — %s installed at %s\n", cfg.appName.c_str(), appPath.c_str());
}

inline int runSetup(const LauncherConfig& cfg = LauncherConfig{}) {
    try {
        buildLauncherApp(cfg);
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "❌ Setup failed: %s\n", e.what());
        return 1;
    }
}
