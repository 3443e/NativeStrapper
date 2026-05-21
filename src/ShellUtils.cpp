#include <array>
#include "ShellUtils.hpp"

// somewhere from https://stackoverflow.com/ boo 
std::string ShellUtils::RunCommand(std::string &cmd) {
    std::array<char, 256> buffer;
    std::string result;
    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    // trim trailing newline/whitespace
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) {
        result.pop_back();
    }
    return result;
}
