#define LOG_CPP 1

#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include "../headers/Log.hpp"
#include "../headers/Properties.hpp"

string next_log_name(string beggining,string path="");
bool file_exists(string s);

void Log::stamp()
{
	pthread_mutex_lock(&smutex);
	if(first_writing)
	{
		clog << "<";
		first_writing=false;
	}
	else
		clog << endl << "<";
	time_t rawtime=time(NULL);
	TM *ti=localtime(&rawtime);
	clog << ti->tm_year+1900 << "-" << ti->tm_mon+1 << "-" << ti->tm_mday << " " << ti->tm_hour << ":"  << ti->tm_min << ":" << ti->tm_sec << "> ";
	pthread_mutex_unlock(&smutex);
}

Log::Log()
{
	pthread_mutex_init(&cmutex,NULL);
	pthread_mutex_lock(&cmutex);

	pthread_mutex_init(&smutex,NULL);
	pthread_mutex_init(&logmutex,NULL);

    system((string("mkdir -p ") + Properties::getDefault().getProperty("LOG_FILES_PATH")).c_str());
	restore=clog.rdbuf();
    log_file.open(next_log_name(Properties::getDefault().getProperty("LOG_FILE_NAME"),
                                Properties::getDefault().getProperty("LOG_FILES_PATH")), ofstream::out | ofstream::app);
	clog.rdbuf(log_file.rdbuf());
	first_writing=true;

	pthread_mutex_unlock(&cmutex);
}

Log::~Log()
{
	usleep(0.05*1000000);	//wait before leaving program so the "clog"'s called at the end of the program don't end up in "cout" instead of the log file
	clog.rdbuf(restore);	//IMPORTANT!!! Removing this line will result in Segmentaion fault
	log_file.close();
	pthread_mutex_destroy(&cmutex);
	pthread_mutex_destroy(&smutex);
	pthread_mutex_destroy(&logmutex);
}

bool file_exists(string s)
{
	//http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
	struct stat *buf=(struct stat*)calloc(1,sizeof(struct stat));
	bool ret = stat((const char*)s.c_str(),buf)==0;
	free(buf);
	return ret;
}

string next_log_name(string beggining,string path)
{
	const string months[12]={"January","February","March","April","May","June","July","August","September","October","November","December"};
	string default_name="";
	stringstream name;
	time_t rawtime=time(NULL);
	TM *ti=localtime(&rawtime);

	if(path.length()>0)
	{
		string::iterator it=path.end();
		if(*it!='/')
			path+="/";
	}

	name << path << beggining << "__" << ti->tm_mday << "_" << months[ti->tm_mon] << "_" << ti->tm_year+1900 << "__" << ti->tm_hour << "h" << ti->tm_min << "m";
	default_name=name.str();

	name.str("");
	name.clear();

	if(!file_exists(default_name+".log"))
		return default_name+".log";
	
	for(int i=1;true;i++)
	{
		name.str("");
		name.clear();
		name << default_name << "(" << i << ").log";
		if(!file_exists(name.str()))
			return name.str();
	}
}