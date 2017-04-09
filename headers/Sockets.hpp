#ifndef SOCKETS_HPP
#define SOCKETS_HPP 1

//--------------------------INCLUDES--------------------------//

#include <iostream> // cout
#include <sstream> // stringstream
#include <stdlib.h>  // exit
#include <string.h> // bzero
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <map>
#include <set>

//--------------------------DEFINE--------------------------//
//TODO: Deprecated!! Both of these defines. Actually, remove EVERY SINGLE DEFINE!
#define LOCK pthread_mutex_lock(&cout_mutex);
#define UNLOCK pthread_mutex_unlock(&cout_mutex);

//--------------------------TYPEDEFS--------------------------//
//typedef struct tm TM; //already inside client.hpp

//TODO: Deprecated!! Take this off of the global namespace, dumbass!
extern pthread_mutex_t cout_mutex;

//--------------------------CLASSES--------------------------//
class Network {

    //Singleton
private:
    Network();

    ~Network();

    Network(Network const &);              // Don't Implement
    void operator=(Network const &); // Don't implement

public:
    static Network &server() //to use this class
    {
        static Network l;
        return l;
    }

public:

    std::map<int, int> clients; //key=socket, value=user_id if logged and LOGGED_OFF if not

    std::set<pthread_t> socket_threads;

    int sockfd;

    void start_server();

    void shutdown_server();

    void writeline(int socketfd, std::string line, bool paragraph = true);

    void broadcast(int origin, std::string text);

    bool readline(int socketfd, std::string &line);

    bool srv_running() {
        return server_running;
    }

    char *get_ip(int);

private:

    int newsockfd;
    socklen_t client_addr_length;
    struct sockaddr_in serv_addr, cli_addr;
    bool server_running, Flag_shutdown;
    pthread_t server_t;

    void server_routine();

    static void *server_routine_redirect(void *t) {
        //vcout << "server_redirect" << endl;
        ((Network *) t)->server_routine();
        return NULL;
    }

    void cliente();

    static void *cliente_redirect(void *c) { //http://stackoverflow.com/a/1151615
        //vcout << "Client_redirect" << endl;
        ((Network *) c)->cliente();
        return NULL;
    }
};


#endif