#include <string>

namespace Utils {
    bool is_bool(std::string const&);

    std::string b2s(bool);

    bool s2b(std::string const&);

    std::string demangle(std::string const&);

    unsigned int getNumberOfArgs(std::string const&);
}