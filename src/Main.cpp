#include "../headers/Sockets.hpp"
#include "../headers/DBMS.hpp"
#include "../headers/Log.hpp"
#include "../headers/Race.hpp"
#include "../headers/Properties.hpp"
#include "../headers/Client.hpp"
#include "../libs/SHA-256.hpp"

#include <ctime>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>

#ifndef vcout //verbose cout.
#define vcout if(false) cout
#endif

#define WATCHDOG_TIMER (1.5)

void* watchdog_routine(void* v)
{
	usleep(WATCHDOG_TIMER*1000000);
	clog("WATCHDOG!!!");
	exit(-1);
	return NULL;
}

int main(int argc, char *argv[])
{
	cout.rdbuf()->pubsetbuf(NULL,0);
	setvbuf ( stdout , NULL , _IONBF , 0 );

	srand(time(NULL));

	system("clear");

	clog( "Program has started" );

	Network::server();	//creates the sockets server. check sockets.hpp for the definition of this macro//

	//--------------------------------SERVER CLIENT ROUTINE---------------------------------//

 	Client c;

 	cout << "> ";

	for( string input ; getline(cin,input) ; cout << "> " )
	{
		if(!c.parse(input))
			break;
	}

	//------------------------------SHUTTING DOWN------------------------------//

	pthread_t watchdog;
	pthread_create(&watchdog,NULL,watchdog_routine,NULL);

	while(races.size())	usleep(0.5*1000000);

	cout << SHA_256_digest("ola","adeus") << endl;

	/*try {
		//Properties p("p.properties");
		const Properties& p = Properties::getDefault();
		list<string> l = p.getProperties();
		for(list<string>::iterator it = l.begin() ; it != l.end() ; it++)
			cout << *it << endl;
		cout << p.getProperty("oli") << endl;
	} catch (const PropertiesException& ex) {
		cerr << "Error! What: " << ex.what() << endl;
	}*/
	
  return 0; 
}