#pragma once
#include <string>

namespace ShellUtils {
    // returns the output of a command in an  std::string
    std::string RunCommand(std::string &cmd);
}
