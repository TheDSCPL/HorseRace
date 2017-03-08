#include "../headers/Utils.hpp"
#include <algorithm>

using namespace std;

bool is_bool(string s)
{
	transform(s.begin(),s.end(),s.begin(),::tolower); //tolower every char in the string
	if(s=="true" || s=="false")
		return true;
	return false;
}

string b2s(bool b)
{
  string s(b? "true" : "false");
  return s;
}

bool s2b(string s)
{ //returns true if s is "true" or "t". this function IS NOT case sensitive
  transform(s.begin(),s.end(),s.begin(),::tolower); //tolower every char in the string
  if(s=="true")
    return true;
  return false;
}