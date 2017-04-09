#include <exception>
#include <string>
#include <map>
#include <fstream>
#include <vector>

class Properties
{
	std::map<std::string,std::string> properties;
	std::ifstream propertiesFile;
	std::string propertiesFileName;
	//void reloadIfNecessary();
public:
	Properties(const std::string&);
	~Properties();
	void reload();
	std::string getProperty(const std::string&) const;
	std::vector<std::string> getProperties() const;
	static const Properties& getDefault();
};

//EXCEPTIONS

//Base exception for this exceptions in Properties

class PropertiesException : public std::exception {
protected:
	char *whatMsg;
public:
	PropertiesException(const std::string&);
	~PropertiesException();
	const char* what() const throw();
};

//Expansions of the base exception class

class PropertyNotFoundException : public PropertiesException {
	const std::string propertyName;
public:
	PropertyNotFoundException(const std::string&);
};

class PropertySyntaxException : public PropertiesException {
	const std::string lineContent;
	const unsigned int line;
public:
	PropertySyntaxException(unsigned int, const std::string&);
};

class PropertyDuplicated : public PropertiesException {
	const std::string propertyName;
	const unsigned int line;
public:
	PropertyDuplicated(unsigned int l, const std::string&);
};

class PropertiesFileReadingError : public PropertiesException {
	const std::string propertiesFile;
public:
	PropertiesFileReadingError(const std::string&);
};