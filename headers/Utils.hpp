#include <string>

namespace Utils {
    bool is_bool(std::string const &);

    std::string b2s(bool);

    bool s2b(std::string const&);

    double tenPower(const long);

    // Same as atoi but for double.
    // Only use dots (".") for separating integer part from decimal part.
    double atod(const std::string&);

    // If the string is empty, returns true. Can consider negative numbers
    bool isInt(const std::string&);

    // If the string is empty, returns true. Can consider negative numbers.
    // Is true even if there is no decimal part.
    // Only use dots (".") for separating integer part from decimal part.
    bool isDouble(const std::string&);

    std::string demangle(std::string const&);

    unsigned int getNumberOfArgs(std::string const&);
}