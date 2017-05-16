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
#include <functional>
#include "Utils.hpp"
#include "Thread.hpp"

//--------------------------DEFINE--------------------------//
//TODO: Deprecated!! Both of these defines. Actually, remove EVERY SINGLE DEFINE!
#define LOCK pthread_mutex_lock(&cout_mutex);
#define UNLOCK pthread_mutex_unlock(&cout_mutex);

//--------------------------TYPEDEFS--------------------------//
//typedef struct tm TM; //already inside client.hpp

//TODO: Deprecated!! Take this off of the global namespace, dumbass!
extern pthread_mutex_t cout_mutex;

//--------------------------CLASSES--------------------------//
class Connection;

class ClientContainer;

class Network {
    friend class Connection;
    Network();

    ~Network();

    Network(Network const &);              // Don't Implement
    void operator=(Network const &); // Don't implement

    socklen_t client_addr_length;
    struct sockaddr_in serv_addr, cli_addr;
    bool Flag_shutdown;
    pthread_t server_t;
    Thread serverThread;

    void server_routine();

    void cliente(int newsockfd);

    static void *cliente_redirect(void *c) { //http://stackoverflow.com/a/1151615
        //vcout << "Client_redirect" << endl;
        ((Network *) c)->cliente(0);
        return NULL;
    }

public:
    static Network &server()
    {
        static Network l;
        return l;
    }

    //TODO: Change to map<Connection*,ClientContainer*>
    std::map<int, int> clients; //key=socket, value=user_id if logged and LOGGED_OFF if not

    std::set<int> socket_threads;

    int sockfd;

    void start_server();

    void shutdown_server();

    bool isRunning();

    static void writeline(int socketfd, std::string line, bool paragraph = true);

    static void write(int socketfd, std::string line);

    static void close(int socketId);

    void broadcast(int origin, std::string text);

    static bool readline(int socketfd, std::string &line);

    std::string get_ip(int);
};

class Connection;

class Connection {
protected:
    friend class Network;

    int socketId;
    volatile bool closed;
    static std::set<Connection *> connections;

    Connection(int socketId);

    virtual ~Connection();

    void throwIfClosed() const;

    //'msg' doesn't need the ": "
    Connection(const Connection &);  //don't implement. I don't want Connection objects to be copied
    void operator=(Connection const &); // Don't implement
public:

    int getSocketId() const;

    int getInt(const std::string &msg, bool positive = false) const;

    double getDouble(const std::string &msg, bool positive = false) const;

    bool getBool(const std::string &msg) const;

    std::string getString(const std::string &msg) const;

    const Connection &operator<<(const char *out) const {
        throwIfClosed();
        Network::write(socketId, out);
        return *this;
    }

    const Connection &operator<<(const std::string &out) const {
        return *this << out.c_str();
    }

    const Connection &operator<<(const int &i) const {
        return *this << std::to_string(i);
    }

    const Connection &operator<<(const double &d) const {
        return *this << std::to_string(d);
    }

    const Connection &operator<<(const bool &b) const {
        std::string s(b ? "true" : "false");
        return *this << s;
    }

    //Reads an entire line from the socket
    const Connection &operator>>(std::string &s) const {
        throwIfClosed();
        if (!Network::readline(socketId, s))
            throw std::runtime_error("Couldn't read from socket " + socketId);
        //else
        //    std::cout << "READ" << std::endl;
        return *this;
    }

    const Connection &operator>>(int &i) const {
        std::string temp;
        *this >> temp;
        i = Utils::stoi(temp);
        return *this;
    }

    const Connection &operator>>(double &d) const {
        std::string temp;
        *this >> temp;
        d = Utils::stod(temp);
        return *this;
    }

    const Connection &operator>>(bool &b) const {
        std::string temp;
        *this >> temp;
        b = Utils::s2b(temp);
        return *this;
    }

    void close();

    void clear();

    static const std::string endl;
    static const std::string GOTO_BEGIN;
};

class ConnectionClosed : public std::exception {
    const std::string whatMessage;
public:
    ConnectionClosed(int socketId);

    const char *what() const throw();
};

#endif