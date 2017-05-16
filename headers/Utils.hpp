#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

std::string operator+(const std::string &s, int i);

std::string operator+(int i, const std::string &s);

namespace Utils {
    bool is_bool(std::string const &);

    std::string b2s(bool);

    bool s2b(std::string const&);

    double tenPower(const long);

    int max(int a, int b);

    unsigned long max(unsigned long a, unsigned long b);

    // Converts a string to double.
    // Throws logic_error if the string isn't a double
    // Only use dots (".") for separating integer part from decimal part.
    double stod(const std::string &);

    int stoi(const std::string &s);

    // If the string is empty, returns true. Can consider negative numbers
    bool isInt(const std::string&);

    // If the string is empty, returns true. Can consider negative numbers.
    // Is true even if there is no decimal part.
    // Only use dots (".") for separating integer part from decimal part.
    bool isDouble(const std::string&);

    std::string demangle(std::string const&);

    unsigned int getNumberOfArgs(std::string const &);

    template<typename T>
    std::vector<T *> copyDynamicVector(const std::vector<T *> &);

    std::string makeHeader(const std::string &);

    std::string makeSubtitle(const std::string &);

    std::string makeCommandDescription(const std::string&, const std::string&);

    std::string makeCommandDescriptionNote(const std::string &);

    std::string getStdEndlString();

    const std::string endl = getStdEndlString();

    bool isOnlyParagraphs(const std::string &s);

    template<typename T>
    T *ptr(T &obj);

    template<typename T>
    T *ptr(T *obj);

    //Only need to give the Child template type. Parent is inferred from 'parent'
    template<class Child, class Parent>
    bool instanceof(Parent parent);
}

#endif