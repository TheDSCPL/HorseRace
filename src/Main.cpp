#include "../headers/Sockets.hpp"
#include "../headers/DBMS.hpp"
#include "../headers/Log.hpp"
#include "../headers/Race.hpp"
#include "../headers/Properties.hpp"
#include "../headers/Client.hpp"
#include "../headers/SHA.hpp"
#include "../headers/Utils.hpp"

#include <ctime>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
#include <stdarg.h>

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

    //cout << Utils::stod("-9.1234") << endl;

    SQLServer::server().start();

    string query = "SELECT * FROM test1 WHERE p_key = $1;";

    SQLServer::server().requestNewPreparedStatement("printMe",query);

    const PreparedStatement* preparedStatement = SQLServer::server().getPreparedStatement("printMe");

    if(!preparedStatement) {
        cerr << "NULL" << endl;
        return 1;
    }

    SQLResult res = preparedStatement->run({"1"});

    cout << res;

    //res.getResultTable().print(cout,false);

    /*cout << Utils::b2s(res.hasError()) << " " << res.getErrorMessage() << endl;

    res.getResultTable().print(cout);*/

    /*srand(time(NULL));

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

	/*cout << SHA_256_digest("password") << endl;
    cout << SHA_512_digest("password") << endl;
    cout << SHA_256_digest("ole") << endl;
    cout << SHA_256_digest("ola") << endl;
    cout << SHA_256_digest("ola") << endl;*/

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