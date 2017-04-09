#include "../headers/Properties.hpp"

#include <iostream>
#include <cstring> //strdup
#include <cstdlib> //free

using namespace std;

Properties::Properties(const std::string& pf) : propertiesFileName(pf) {
	if(propertiesFile.is_open())
		propertiesFile.close();
	propertiesFile.open(pf);
	reload();
}

Properties::~Properties() {
	if(propertiesFile.is_open())
		propertiesFile.close();
}

const Properties& Properties::getDefault() {
	static Properties defaultProperties("default.properties");
	return defaultProperties;
}

void trimWhiteSpaces(string& s) {
	while(!s.empty() && s[0] == ' ') {
		s.erase(0,1);
	}
	while(!s.empty() && s[s.length() - 1] == ' ') {
		s.erase(s.length() - 1,1);
	}
}

/*void Properties::reloadIfNecessary() {
	if(!propertiesFile.is_open() || properties.empty())
		reload();
}*/

void Properties::reload() {
	properties.clear();
	if(propertiesFile.is_open())
		propertiesFile.close();
	propertiesFile.open(propertiesFileName);
	if(!propertiesFile.is_open())
		throw PropertiesFileReadingError(propertiesFileName);
	for(unsigned int line = 1; !propertiesFile.eof(); line++) {
		char temp[256];
		propertiesFile.getline(temp,255);
		//if( propertiesFile.eof() ) break;
		string tempS = string(temp);
		trimWhiteSpaces(tempS);
		if(tempS.empty())
			continue;
		size_t eqPos = tempS.find_first_of('=');
		if(eqPos == 0 || eqPos == string::npos || eqPos >= tempS.length() - 1)
			throw PropertySyntaxException(line,tempS);
		string key = tempS.substr(0, eqPos);
		string value = tempS.substr(eqPos + 1);
		trimWhiteSpaces(key);
		trimWhiteSpaces(value);
		if(key.empty() || value.empty())
			throw PropertySyntaxException(line,tempS);

		pair<map<string,string>::iterator,bool> ret = properties.insert(pair<string,string>(key,value));
		if(!ret.second)
			throw PropertyDuplicated(line,key);
	}
}

string Properties::getProperty(const std::string& p) const {
	//reloadIfNecessary();
	map<string,string>::const_iterator it = properties.find(p);
	if(it == properties.end())
		throw PropertyNotFoundException(p);
	return it->second;
}

vector<string> Properties::getProperties() const {
	//reloadIfNecessary();
	vector<string> ret;
	for(map<string,string>::const_iterator it = properties.begin(); it != properties.end(); it++)
		ret.push_back(it->first);
	return ret;
}

//EXCEPTIONS

PropertiesException::PropertiesException(const string& p) : whatMsg( strdup(p.c_str()) ) {}

PropertiesException::~PropertiesException() {
	if(whatMsg)free(whatMsg);
}

const char* PropertiesException::what() const throw() {
	return whatMsg;
}

PropertyNotFoundException::PropertyNotFoundException(const string& p) :
	PropertiesException("Property \"" + p + "\" not found"),
	propertyName(p) {}

PropertySyntaxException::PropertySyntaxException(unsigned int l, const string& p) :
	PropertiesException("Line " + to_string(l) + " has invalid syntax!\nLine's content: \"" + p + "\""),
	lineContent(p),
	line(l) {}

PropertyDuplicated::PropertyDuplicated(unsigned int l, const string& p) :
	PropertiesException("Property \"" + p + "\" is repeated on line " + to_string(l)),
	propertyName(p),
	line(l) {}

PropertiesFileReadingError::PropertiesFileReadingError(const std::string& pf) :
	PropertiesException("Property file \"" + pf + "\" could't be oppened"),
	propertiesFile(pf) {}