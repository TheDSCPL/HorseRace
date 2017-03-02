#ifndef LOG_HPP
#define LOG_HPP 1

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <fstream>
#include <sstream>


/*
THIS IS A THREAD-SAFE LIBRARY TO CREATING LOG FILES.
"Log" IS A SINGLETON CLASS SO THE REDIRECTION OF "clog" IS NEVER REPEATED

-ALWAYS USE "clog(...)" INSTEAD OF  "clog << ..." BECAUSE THE SECOND IS NOT THREAD SAFE
-AS A SINGLETON, THIS CLASS CANNOT BE CALLED DIRECTLY ("Log l")
-TO USE THIS CLASS CALL "Log::start()" LIKE SO: "Log::start().stamp()", FOR EXAMPLE.
*/


#ifndef LOG_CPP	//coisas que só são feitas FORA do log.cpp
//the next define would be an excellect idea if this wasn't a threaded program. therefore it should not be used
//#define clog Log::start().stamp(); clog
//this is un uglier solution but it is thread-safe
#define clog(A) {pthread_mutex_lock(&Log::start().logmutex); Log::start().stamp(); clog << A << "\r" << endl; pthread_mutex_unlock(&Log::start().logmutex);}
#endif

#ifdef LOG_CPP	//coisas que só são feitas DENTRO do log.cpp
#define LOG_FILE_NAME "BOP2_proj_log"
#define LOG_FILES_PATH "logs"
typedef struct tm TM;
#endif


using namespace std;

class Log	//singleton
{
public:
	static Log& start()
	{
		static Log l;
		return l;
	}
	void stamp();
	pthread_mutex_t logmutex;
private:	//singleton stuff
	Log(Log const&);              // Don't Implement
	void operator=(Log const&); // Don't implement
	Log();
	~Log();
private:
	pthread_mutex_t cmutex,smutex;
	ofstream log_file;
	streambuf *restore;
	bool first_writing;
};

#endif