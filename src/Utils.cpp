#include "../headers/Utils.hpp"
#include <algorithm>
#include <cxxabi.h>
#include <cctype>

using namespace std;

bool Utils::is_bool(string const& s)
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
  if(_s=="true")
    return true;
  return false;
}

string Utils::demangle(string const& to_demangle)	//to know what was the unknow exception was when "catch(...) activates	//http://stackoverflow.com/a/24997351/6302540
{
    int status = 0;
    char * buff = __cxxabiv1::__cxa_demangle(to_demangle.c_str(), NULL, NULL, &status);
    string demangled = buff;
    free(buff);
    return demangled;
}

unsigned int Utils::getNumberOfArgs(std::string const & query) {
    int count = 0;
    query.length();
    for(int i = 0 ; i<query.length()-1 /*the last character is never an argument so -1*/ ; i++) {
        if(query[i] == '$') {
            if(isdigit(query[i+1])) {
                count++;
            }
            i++; //yes, I want to increment i by 2 in this case because if I find a dollar sign, than the next character will never be an argument
        }
    }
    return count;
}