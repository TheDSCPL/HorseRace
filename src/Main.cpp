#include "../headers/Sockets.hpp"
#include "../headers/DBMS.hpp"
#include "../headers/Log.hpp"
#include "../headers/Race.hpp"
#include "../headers/Properties.hpp"
#include "../headers/Client.hpp"
#include "../headers/SHA.hpp"
#include "../headers/Utils.hpp"
#include "../headers/Thread.hpp"

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

/*void count(int a, ThreadCondition& t) {
    Mutex m;
    m.lock();
    t.timedWait(m,2500);
    //t.wait(m);
    for(int i = 1 ; i<=5 ; i++) {
        if(i>1)
            Thread::usleep(500L+(long)((double)rand()/RAND_MAX*500));
        cerr << a << ": " << i << endl;
    }
    cerr << a << " is finished" << endl;
    m.unlock();
}*/

int main(int argc, char *argv[])
{
    srand(time(NULL));

    cout.rdbuf()->pubsetbuf(NULL, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    //cout << Utils::stod("-9.1234") << endl;

    SQLServer::server().start();

    /*ThreadCondition t;

    Thread t1([&t](){
        count(1,t);
    }), t2([&t](){
        count(2,t);
    });

    t1.start();
    t2.start();

    cerr << "Main's first \"come on in\"" << endl;
    t.signal();

    //Thread::usleep(3000);

    //cerr << "Main's second \"come on in\"" << endl;
    //t.signal();

    t1.join();
    t2.join();*/

    /*string query = "SELECT * FROM test1 WHERE p_key = $1;";

    SQLServer::server().requestNewPreparedStatement("printMe",query);

    const PreparedStatement* preparedStatement = SQLServer::server().getPreparedStatement("printMe");

    if(!preparedStatement) {
        cerr << "NULL" << endl;
        return 1;
    }

    SQLResult res = preparedStatement->run({"1"});

    cout << res;*/



    /*try {
        Client::initPreparedStatements();
        const PreparedStatement *ps = S.getPreparedStatement("CHANGE_BET");
        if (!ps)
        {
            cout << "No such ps" << endl;
            return 1;
        }
        SQLResult sqlResult = ps->run({"1", "2", "3", "00"});
        const SQLResultTable &sqlResultTable = sqlResult.getResultTable();
        cout << sqlResult.hasTableResult() << endl;
        //cout << sqlResultTable[0]->getInt(0) << endl;
        cout << sqlResultTable[0]->getString(0) << endl;
    } catch (int) {
        cerr << "exception" << endl;
    }*/


    //res.getResultTable().print(cout,false);

    /*cout << Utils::b2s(res.hasError()) << " " << res.getErrorMessage() << endl;

    res.getResultTable().print(cout);*/

    system("clear");

    S.start();
    Client::initPreparedStatements();

    clog("Program has started");

    Network::server();    //creates the sockets server. check sockets.hpp for the definition of this macro//

    //--------------------------------SERVER CLIENT ROUTINE---------------------------------//

    Client c;

    cout << "> ";

    for (string input; getline(cin, input); cout << "> ") {
        if (!c.parse(input))
            break;
    }

    //------------------------------SHUTTING DOWN------------------------------//

    Network::server().shutdown_server();

    pthread_t watchdog;
    pthread_create(&watchdog, NULL, watchdog_routine, NULL);

    while (races.size()) usleep(500);

    /*Thread askForInput ([](){
        char a;
        cout << "I'm waiting for input!" << endl;
        cin >> a;
    });
    Thread watchDog ([&askForInput](){
        Thread::usleep(5000);
        cout << "Oh no you're not!" << endl;
        askForInput.cancel();
    });

    watchDog.start();
    askForInput.start();
    askForInput.join();
    watchDog.cancel();
    watchDog.join();*/

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