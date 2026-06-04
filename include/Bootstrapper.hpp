#pragma once

#include "NativeStrapper.hpp"
#include <string>

namespace Bootstrapper {
    enum BootstrapResult {
        BOOTSTRAP_SUCCESS,
        BOOTSTRAP_SCRIPT_NOT_FOUND,
        BOOTSTRAP_SCRIPT_ERR
    };
    extern std::string Exception;

    BootstrapResult MainBootstrap(NativeStrapper::ArgConfig*);
}