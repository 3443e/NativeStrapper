// platform_default_handler_mac.mm
#import <AppKit/AppKit.h>
#import <CoreServices/CoreServices.h>   // LSRegisterURL, LSSetDefaultHandlerForURLScheme
#include "platform_default_handler.h"

#if defined(__APPLE__)   // belt-and-suspenders; build system already gates this file

namespace platform {

bool setAsDefaultHandler(const std::string& scheme) {
    @autoreleasepool {
        NSString* nsScheme = [NSString stringWithUTF8String:scheme.c_str()];
        NSURL*    appURL   = [[NSBundle mainBundle] bundleURL];

        // Launch Services must know about us before we can claim the scheme.
        LSRegisterURL((__bridge CFURLRef)appURL, true);

        if (@available(macOS 12.0, *)) {
            __block bool ok = false;
            dispatch_semaphore_t sem = dispatch_semaphore_create(0);
            [[NSWorkspace sharedWorkspace]
                  setDefaultApplicationAtURL:appURL
                        toOpenURLsWithScheme:nsScheme
                           completionHandler:^(NSError* _Nullable error) {
                ok = (error == nil);
                if (error) NSLog(@"setDefaultApplication failed: %@", error);
                dispatch_semaphore_signal(sem);
            }];
            dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);  // see caveat below
            return ok;
        } else {
            // Fallback for macOS 11 and earlier (deprecated but functional).
            NSString* bundleID = [[NSBundle mainBundle] bundleIdentifier];
            OSStatus st = LSSetDefaultHandlerForURLScheme(
                (__bridge CFStringRef)nsScheme,
                (__bridge CFStringRef)bundleID);
            return st == noErr;
        }
    }
}

std::string currentDefaultHandler(const std::string& scheme) {
    @autoreleasepool {
        NSString* probe = [NSString stringWithFormat:@"%s:", scheme.c_str()];
        NSURL* handler = [[NSWorkspace sharedWorkspace]
                            URLForApplicationToOpenURL:[NSURL URLWithString:probe]];
        return handler ? std::string(handler.path.UTF8String) : std::string();
    }
}

} // namespace platform
#endif