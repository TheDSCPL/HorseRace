#include "../headers/Utils.hpp"
#include <algorithm>
#include <cxxabi.h>
#include <cctype>
#include <stdexcept>
#include <iostream>

using namespace std;

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
        return true;

    auto it = s.begin();
    if(*it == '-')
        it++;   //if it's negative, ignore the first character ('-')

    for( ; it != s.end() ; it++)
        if(!isdigit(*it))
            return false;

    return true;
}

bool Utils::isDouble(const string & s) {
    if(s.empty())
        return true;

    auto it = s.begin();
    if(*it == '-')
        it++;   //if it's negative, ignore the first character ('-')

    bool foundDot = false;

    for( ; it != s.end() ; it++) {
        if(*it == '.') {
            if(foundDot)
                return false;   //more than one dot found!
            foundDot = true;
            continue;
        }
        if(!isdigit(*it))
            return false;
    }

    return true;
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