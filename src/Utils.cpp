#include <cctype>
#include <algorithm>
#include <cxxabi.h>
#include <stdexcept>
#include <iostream>
#include <set>
#include <chrono>
#include <thread>
#include <sstream>
#include <unistd.h>
#include "../headers/Utils.hpp"
#include "../headers/DBMS.hpp"

using namespace std;

std::string operator+(const std::string &s, int i) { return s + std::to_string(i); }

std::string operator+(int i, const std::string &s) { return std::to_string(i) + s; }

bool Utils::is_bool(string const &s)
{
    string _s(s);
	transform(_s.begin(),_s.end(),_s.begin(),::tolower); //tolower every char in the string
	if(_s=="true" || _s=="false")
		return true;
	return false;
}

string Utils::b2s(bool b)
{
  string s(b? "true" : "false");
  return s;
}

bool Utils::s2b(string const& s)
{ //returns true if _s is "true" or "t". this function IS NOT case sensitive
    string _s(s);
    transform(_s.begin(),_s.end(),_s.begin(),::tolower); //tolower every char in the string
    if(_s=="true" || _s=="t")
        return true;
    else if(_s=="false" || _s=="f")
        return false;
    else
        throw logic_error("\"" + s + "\" cannot be converted to boolean!");
}

double Utils::tenPower(const long n) {
    double ret = 1.0;
    if(n < 0)
        for(int i = 0 ; i<(0-n) ; i++)
            ret/=10;
    else if(n > 0)
        for(int i = 0 ; i<n ; i++)
            ret*=10;
    return ret;
}

int Utils::max(int a, int b) {
    return a>b?a:b;
}

unsigned long Utils::max(unsigned long a, unsigned long b) {
    return a>b?a:b;
}

double Utils::stod(const std::string &s) {
    if(!isDouble(s))
        throw logic_error("Tried to get double but it's not double.");

    unsigned long dotIndex = s.find('.');
    if(dotIndex == s.npos)
        dotIndex = s.length();
    double ret = 0;
    /*if(dotIndex == s.npos)  //doesn't have a dot -> integer
        return atoi(s.c_str());*/

    bool minus = (*s.begin() == '-');

    double pt = tenPower((int)dotIndex - s.length());

    for(string::const_reverse_iterator rit = s.rbegin() ; rit != s.rend() ; rit++) {
        if(isdigit(*rit)) {
            //cout << "rit=" << *rit << " ret=" << ret << " pt=" << pt << " +=" << pt*(*rit-'0') << endl;
            ret += (pt*(*rit-'0'));
            pt*=10;
        }
        //pt*=10;
    }

    if(dotIndex != s.length())
        ret*=10;

    return minus?-ret:ret;
}

int Utils::stoi(const std::string &s) {
    if(!isInt(s))
        throw logic_error("Tried to get int but it's not int.");

    int ret = 0;
    /*if(dotIndex == s.npos)  //doesn't have a dot -> integer
        return atoi(s.c_str());*/

    bool minus = (*s.begin() == '-');

    int pt = 1;

    for(string::const_reverse_iterator rit = s.rbegin() ; rit != s.rend() ; rit++) {
        if(isdigit(*rit)) {
            ret += (pt*(*rit-'0'));
            pt*=10;
        }
    }

    return minus?-ret:ret;
}

bool Utils::isInt(const string& s) {
    if(s.empty())
        return false;

    auto it = s.begin();
    if(*it == '-')
        it++;   //if it's negative, ignore the first character ('-')

    bool hadAtLeastOneDigit = false;
    for (; it != s.end(); it++) {
        hadAtLeastOneDigit = true;
        if(!isdigit(*it))
            return false;
    }

    return hadAtLeastOneDigit;
}

bool Utils::isDouble(const string & s) {
    if(s.empty())
        return false;

    auto it = s.begin();
    if(*it == '-')
        it++;   //if it's negative, ignore the first character ('-')

    bool foundDot = false;
    bool hadAtLeastOneDigit = false;

    for( ; it != s.end() ; it++) {
        if(*it == '.') {
            if(foundDot)
                return false;   //more than one dot found!
            foundDot = true;
            continue;
        }
        hadAtLeastOneDigit = true;
        if(!isdigit(*it))
            return false;
    }

    return hadAtLeastOneDigit;
}

string Utils::demangle(string const& to_demangle)	//to know what was the unknow exception was when "catch(...) activates	//http://stackoverflow.com/a/24997351/6302540
{
    int status = 0;
    char * buff = __cxxabiv1::__cxa_demangle(to_demangle.c_str(), NULL, NULL, &status);
    string demangled = buff;
    free(buff);
    return demangled;
}

unsigned int Utils::getNumberOfArgs(std::string const &query) {
    if (query.empty())
        return 0;
    query.length();
    set<int> args;
    for (unsigned int i = 0; i < query.length() - 1 /*the last character is never an argument so -1*/ ; i++) {
        if(query[i] == '$') {
            string n;
            while (isdigit(query[i + 1])) {
                n += query[i + 1];
                i++;
            }
            if (!n.empty()) {
                args.insert(atoi(n.c_str()));
            } else {
                i++; //yes, I want to increment i by 2 in this case because if I find an empty dollar sign, than the next character will never be an argument
            }
        }
    }
    return (int) args.size();
}

template<typename T>
std::vector<T *> Utils::copyDynamicVector(const std::vector<T *> &v) {
    vector<T *> ret;
    for (unsigned int i = 0; i < v.size(); i++)
        ret.push_back(new T(*v[i]));
    return ret;
}

template std::vector<string *> Utils::copyDynamicVector(const std::vector<string *> &);

template std::vector<Tuple *> Utils::copyDynamicVector(const std::vector<Tuple *> &);

//https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_codes	//http://www.termsys.demon.co.uk/vtansi.htm

std::string Utils::makeHeader(const std::string & s) {
    return "\r\n\u001B[30;41m----------" + s + "---------\u001B[0m\r\n";
}

std::string Utils::makeSubtitle(const std::string &s) {
    return "\u001B[30;42m" + s + "\u001B[0m";
}

std::string Utils::makeCommandDescription(const std::string & methodName, const std::string & description) {
    return "\u001B[30;42m->\"" + methodName + "\"\u001B[0m - " + description;
}

std::string Utils::makeCommandDescriptionNote(const std::string &note) {
    return string("\t\t") + "Note: " + note;
}

std::string Utils::getStdEndlString() {
    stringstream ret;
    ret << std::endl;
    return ret.str();
}

bool Utils::isOnlyParagraphs(const std::string &s) {
    if (s.empty())
        return false;
    for (const char c : s)
        if (c != '\r' && c != '\n')
            return false;
    return true;
}

template<typename T>
T *Utils::ptr(T &obj) { return &obj; }

template<typename T>
T *Utils::ptr(T *obj) { return obj; }

template<class Child, class Parent>
bool Utils::instanceof(Parent parent) {
    return dynamic_cast<Child *>(Utils::ptr(parent)) != nullptr;
}