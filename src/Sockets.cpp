#include "../headers/Sockets.hpp"
#include "../headers/Log.hpp"
#include "../headers/Client.hpp"
#include "../headers/Properties.hpp"
#include "../headers/Utils.hpp"
#include "../headers/Constants.hpp"
#include "../headers/Thread.hpp"
#include "../headers/GarbageCollector.hpp"

using namespace Constants;

#define BUFFER_SIZE (1025)

#define vcout if(false) cerr

pthread_mutex_t cout_mutex;

Network::Network() : Flag_shutdown(false), server_t(0), serverThread([this]() { server_routine(); }) {
    clog("Entered Network's constructor");
    // Inicializar o socket
    // sockfd - id do socket principal do servidor
    // Se retornar < 0 ocorreu um erro
    pthread_mutex_init(&cout_mutex, NULL);

    start_server();
}

Network::~Network() {
    vcout << "EXITING! Entered Network's destructor" << endl;
    clog("Entered Network's destructor");

    shutdown_server();

    clog("Ending all threads");
    vcout << "Ending all threads" << endl;
    while (socket_threads.size()) {
        pthread_join(*socket_threads.begin(), NULL);
        socket_threads.erase(*socket_threads.begin());
    }
    clog("Done ending threads");
    vcout << "Done ending threads" << endl;

    pthread_mutex_destroy(&cout_mutex);
    //vcout << "Closing server socket." << endl;
    close(sockfd);
    clog("Network destroyed");
    vcout << "Network destroyed" << endl;
}

void Network::start_server() {
    clog("Starting server (\"start_server()\")");
    if (serverThread.isRunning()) {
        return;
    }
    const int enable_resuse_addr = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable_resuse_addr, sizeof(int));
    if (sockfd < 0) {
        clog("Error creating server socket");
        cerr << "Error creating server socket" << endl;
        exit(-1);
    }

    // Criar a estrutura que guarda o endereço do servidor
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) Utils::stoi(Properties::getDefault().getProperty("PORT")));
    client_addr_length = sizeof(cli_addr);

    // Fazer bind do socket. Apenas nesta altura é que o socket fica ativo
    // mas ainda não estamos a tentar receber ligações.
    //  Se retornar < 0 ocorreu um erro
    int res = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (res < 0) {
        clog("Error binding server to socket");
        cerr << "Error binding server to socket" << endl;
        exit(-1);
    }

    // Indicar que queremos escutar no socket com um backlog de 5 (podem
    // ficar até 5 ligações pendentes antes de fazermos accept
    listen(sockfd, 5);

    serverThread.start();

    cout << "Telnet server started on " << get_ip(sockfd) << ':' << Properties::getDefault().getProperty("PORT")
         << ". Accepting clients." << endl << endl;
    clog("Telnet server started on " << get_ip(sockfd) << ':' << Properties::getDefault().getProperty("PORT")
                                     << ". Accepting clients.");
}

void Network::writeline(int so, string line, bool paragraph) { // Envia uma string para um socket
    if (so < 0) {
        LOCK;
        cerr << line << endl;
        UNLOCK;
    }
    else {
        string tosend = "\u001B[48D" + line;
        if (paragraph) tosend += Utils::getStdEndlString();
        ::write(so, tosend.c_str(), tosend.length());
    }
}

void Network::write(int socketfd, std::string tosend) {
    if (socketfd < 0) {
        LOCK;
        cerr << tosend;
        UNLOCK;
    } else {
        ::write(socketfd, tosend.c_str(), tosend.length());
    }
}

void Network::close(int socketId) {
    shutdown(socketId, 2);
    ::close(socketId);
}

string Network::get_ip(int socket_ref) {
    //int fd;
    struct ifreq ifr;

    socket_ref = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

    ioctl(socket_ref, SIOCGIFADDR, &ifr);

    //close(socket_ref);

    static Mutex m;
    m.lock();   //because 'inet_ntoa' uses a shared char* buffer which means it is susceptible to data race problems
    string ret = string(inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
    m.unlock();

    return ret;
}

bool Network::readline(int socketfd,
                       string &line) { // Lê uma linha de um socket e retorna false se o socket se tiver fechado

    // buffer de tamanho BUFFER_SIZE para ter espaço para o \0 que indica o fim de string
    char buffer[BUFFER_SIZE];

    /* inicializar a string */
    line = "";

    if (socketfd <= 0) {
        cin >> line;
    } else { // Enquanto não encontrarmos o fim de linha vamos lendo mais dados da stream
        while (line.find('\n') == string::npos) {
            //cerr << "r" << endl;
            // leu n carateres. se for zero chegamos ao fim
            int n = read(socketfd, buffer, BUFFER_SIZE - 1); // ler do socket
            if (n <= 0)
                throw ConnectionClosed(socketfd); // nada para ser lido -> socket fechado
            if (buffer[0] == '\n' || buffer[0] == '\r')
                break;
            if (!isprint(buffer[0]))  //para a eliminar o lixo que aparece com telnet windows
            {
                cerr << "ignored" << endl;
                continue;
            }
            buffer[n] = 0; // colocar o \0 no fim do buffer
            line += buffer; // acrescentar os dados lidos à string
        }
    }

    // Retirar o \r\n (lemos uma linha mas não precisamos do \r\n)
    while (line.back() == '\r' || line.back() == '\n') {
        line.erase(line.end() - 1);
    }

    return true;
}

void Network::broadcast(int origin, string text) { // Envia uma mensagem para todos os clientes ligados exceto 1
    // Usamos um ostringstream para construir uma string
    // Funciona como um cout mas em vez de imprimir no ecrã
    // imprime numa string

    if (clients[origin] == LOGGED_OFF) clog("Socket " << origin << " (not logged in) broadcasted: \"" << text << "\"")
    else clog("Socket " << origin << " (user_id=" << clients[origin] << ") broadcasted: \"" << text << "\"")

    stringstream message;
    message << origin << " said: " << text;

    // Iterador para sets de inteiros
    map<int, int>::iterator it;
    for (it = clients.begin(); it != clients.end(); it++)
        if (it->first != origin) writeline(it->first, message.str());
}

void Network::server_routine() {
    clog("Network server thread running");
    int enable_resuse_addr = 1;

    GarbageCollector<Thread> gc([](const Thread *t) -> bool {
        return t->isRunning();
    }, [](Thread *t) -> void {
        t->cancel();
    });
    gc.setAutoShakeDownState(false);
    gc.setAutoShakeDownPeriod(30000);
    gc.setAutoShakeDownState(true);

    while (true) {
        // Aceitar uma nova ligação. O endereço do cliente fica guardado em
        // cli_addr - endereço do cliente
        // newsockfd - id do socket que comunica com este cliente

        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &client_addr_length);
        //cout << "client accepted " << newsockfd << endl;

        if (Flag_shutdown || newsockfd < 0) {
            //vcout << "Shutdown signal recieved." << endl;
            break;
        }

        setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR, &enable_resuse_addr, sizeof(int));

        Thread *newClient = new Thread(
                [this, newsockfd]() {
                    cliente(newsockfd);
                }, [this, newClient]() {
                    //cout << "ON_STOP" << endl;
                }
        );
        newClient->start();
        gc.add(newClient);
    }
    close(sockfd);
    shutdown_server();
    //vcout << "Leaving server routine" << endl;
    Flag_shutdown = false;
    server_t = 0;

    clog("Leaving server routine");
}

void Network::shutdown_server() {
    if (!serverThread.isRunning())
        return;
    clog("Shutting down Network server");

    //termina o accept do server
    Flag_shutdown = true;
    int enable_resuse_addr = 1;
    int dummy = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(dummy, SOL_SOCKET, SO_REUSEADDR, &enable_resuse_addr, sizeof(int));
    connect(dummy, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    serverThread.join();
    cout << "Network server is now shutdown!" << endl;
    close(dummy);

    vcout << "clients size=" << clients.size() << endl;
    int s_u_i = clients[IN_SERVER]; //to save the user that was logged in the server because i don't want to close the server client, just the network clients
    while (clients.size()) //fecha todas as sockets, terminando os reads e consequentemente as threads dos clientes
    {
        if (clients.begin()->first != IN_SERVER) {
            close(clients.begin()->first);
            cerr << "closed socket " << clients.begin()->first << endl;
        }
        clients.erase(clients.begin()->first);
    }
    clients.insert(make_pair(IN_SERVER, s_u_i));

    vcout << "Joining clients threads" << endl;

    for (auto it = socket_threads.begin(); it != socket_threads.end(); it++) {
        pthread_join(*it, NULL);
    }

    vcout << "All threads joined" << endl;

    clog("All clients kicked");

    Flag_shutdown = false;
    server_t = 0;

    clog("Network server successfully shutdown");
}

bool Network::isRunning() {
    return serverThread.isRunning();
}

// Trata de receber dados de um cliente cujo socketid foi passado como parâmetro
void Network::cliente(int newsockfd) {
    //unique_ptr<ClientContainer> clientContainer(new ClientContainer(newsockfd));
    ClientContainer *clientContainer = new ClientContainer(newsockfd);

    //LOCK;cerr << "\u001B[8DNew client accepted on socket " << client_socket << "." << endl << "> ";UNLOCK;
    clog("New client accepted on socket " << clientContainer->socketId << ". Thread where this socket is being heard: "
                                          << pthread_self());

    //--------------------------------CLIENT ROUTINE-----------------------------//
    //write(client_socket,"\u001B[44;31;0m",strlen("\u001B[44;31;0m"));
    //*clientContainer << "> ";

    clientContainer->clear();
    while (1) {
        try {
            vector<CommandSet> &commadSets = clientContainer->getCommandSets();
            int counter = 0, input;
            *clientContainer << Connection::endl << Connection::GOTO_BEGIN << counter++ << " - quit"
                             << Connection::endl;
            for (auto &c : commadSets)
                *clientContainer << Connection::GOTO_BEGIN << counter++ << " - " << c.getGroup()->getGroupName()
                                 << Connection::endl;
            *clientContainer << Connection::GOTO_BEGIN << counter++ << " - help" << Connection::endl;

            input = clientContainer->getInt("Select option");

            if (input == 0)
                break;
            else if (input > (int) commadSets.size() + 1 || input < 0) {
                clientContainer->clear();
                *clientContainer << Connection::GOTO_BEGIN << "Wrong choice. Pick again!";
                continue;
            } else {
                if (input == (int) commadSets.size() + 1) {  //help
                    clientContainer->clear();
                    for (const CommandSet &cs : commadSets) {
                        *clientContainer << Connection::GOTO_BEGIN << cs.getGroup()->getGroupHelp();
                    }
                } else {    //one of the command sets
                    CommandSet &selectedCommandSet = commadSets.at((size_t) (input) - 1);
                    *clientContainer << Connection::endl;
                    clientContainer->clear();
                    repeat_sub_menu:
                    counter = 0;
                    *clientContainer << Connection::GOTO_BEGIN << Utils::makeHeader(
                            string("Sub-menu \"") + selectedCommandSet.getGroup()->getGroupName() + "\"")
                                     << Connection::endl;
                    *clientContainer << Connection::endl << Connection::GOTO_BEGIN << counter++ << " - back"
                                     << Connection::endl;
                    for (auto &c : selectedCommandSet.getCommands()) {
                        *clientContainer << Connection::GOTO_BEGIN << counter++ << " - " << c->getName()
                                         << Connection::endl;
                    }
                    *clientContainer << Connection::GOTO_BEGIN << counter++ << " - help" << Connection::endl;

                    input = clientContainer->getInt("Select option");

                    if (input == 0) { //back
                        clientContainer->clear();
                        continue;
                    } else if (input > (int) selectedCommandSet.getCommands().size() + 1 || input < 0) {
                        clientContainer->clear();
                        *clientContainer << Connection::GOTO_BEGIN << "Wrong choice. Pick again!";
                        goto repeat_sub_menu;
                    } else if (input == (int) selectedCommandSet.getCommands().size() + 1) {  //help
                        clientContainer->clear();
                        *clientContainer << Connection::GOTO_BEGIN << Utils::makeHeader(
                                string("Sub-menu \"") + selectedCommandSet.getGroup()->getGroupName() + "\"'s help:")
                                         << Connection::endl;
                        for (const Command *c : selectedCommandSet.getCommands()) {
                            *clientContainer << Connection::GOTO_BEGIN << c->getHelp();
                        }
                    } else {    //one of the commands
                        try {
                            selectedCommandSet.getCommands().at((size_t) input - 1)->run();
                        } catch (ClientMessage &cm) {
                            cm.show(*clientContainer);
                        }
                    }
                }
            }

        } catch (ClientError &ce) {
            ce.show(*clientContainer);
        } catch (ConnectionClosed &cc) {
            cerr << "ConnectionClosed exception!" << endl;
            break;
        } catch (QuitClient &qc) {
            break;
        }
    }
    delete clientContainer;
    /*write(client_socket,"\u001B[2J\u001B[H",strlen("\u001B[2J\u001B[H"));
    writeline(client_socket,"ola");
    char s[]="nana\u001B[48D ole";
    write(client_socket,s,strlen(s));*/

    //----------------------------------CLIENT DISCONNECTING PROCESS---------------------------------//

    clientContainer->close();
    //cout << "CLIENTE() ENDED!" << endl;
    /*clients.erase(clientContainer);
    close(clientContainer);
    socket_threads.erase(pthread_self());*/
}

//// Trata de receber dados de um cliente cujo socketid foi passado como parâmetro
//void Network::cliente(int client_socket) {
//    //LOCK;cerr << "\u001B[8DNew client accepted on socket " << client_socket << "." << endl << "> ";UNLOCK;
//    clog("New client accepted on socket " << client_socket << ". Thread where this socket is being heard: "
//                                          << pthread_self());
//
//    Client c(client_socket);  //start a new Clinet object
//    string input;
//
//
//    //--------------------------------CLIENT ROUTINE-----------------------------//
//    //write(client_socket,"\u001B[44;31;0m",strlen("\u001B[44;31;0m"));
//    write(client_socket, "> ");
//
//    while (readline(client_socket, input)) {
//        //cout << input.size() << endl;
//        if (input == "" || input == "\n" || input == "\r" || input == "\r\n")
//            continue;
//        if (!c.parse(input))
//            break;
//        //cout << "Socket " << client_socket << " said: " << input << endl;
//        //broadcast(client_socket, input);
//        write(client_socket, "> ");
//    }
//    /*write(client_socket,"\u001B[2J\u001B[H",strlen("\u001B[2J\u001B[H"));
//    writeline(client_socket,"ola");
//    char s[]="nana\u001B[48D ole";
//    write(client_socket,s,strlen(s));*/
//
//    //----------------------------------CLIENT DISCONNECTING PROCESS---------------------------------//
//
//    clients.erase(client_socket);
//    close(client_socket);
//    socket_threads.erase(pthread_self());
//}

set<Connection *> Connection::connections = set<Connection *>();

Connection::Connection(int socketId) : socketId(socketId), closed(false) {
    connections.insert(this);
}

Connection::~Connection() {
    close();
    connections.erase(this);
}

void Connection::throwIfClosed() const {
    if (closed)
        throw (runtime_error("Tried to use Connection but it is closed!"));
}

int Connection::getSocketId() const {
    return socketId;
}

//TODO: if EOF, throw something to cancel the command

int Connection::getInt(const string &msg) const {
    int ret;
    do {
        try {
            *this << msg << ": ";
            *this >> ret;
        } catch (logic_error &le) {
            *this << Connection::GOTO_BEGIN << "Only integers are allowed!" << Connection::endl;
            continue;
        }
        break;
    } while (1);
    return ret;
}

double Connection::getDouble(const string &msg) const {
    double ret;
    do {
        try {
            *this << msg << ": ";
            *this >> ret;
        } catch (logic_error &le) {
            *this << Connection::GOTO_BEGIN << "Only floating point values are allowed!" << Connection::endl;
            continue;
        }
        break;
    } while (1);
    return ret;
}

bool Connection::getBool(const string &msg) const {
    bool ret;
    do {
        try {
            *this << msg << ": ";
            *this >> ret;
        } catch (logic_error &le) {
            *this << Connection::GOTO_BEGIN << "Only booleans are allowed!" << Connection::endl;
            continue;
        }
        break;
    } while (1);
    return ret;
}

string Connection::getString(const string &msg) const {
    string ret;
    do {
        *this << msg << ": ";
        *this >> ret;
    } while (ret.empty() || Utils::isOnlyParagraphs(ret));
    return ret;
}

void Connection::close() {
    if (closed)
        return;
    closed = true;
    if (socketId > 0)
        Network::close(socketId);
    else
        Network::server().shutdown_server();
}

void Connection::clear() {
    if (socketId > 0) {
        char clear_str[] = "\u001B[2J\u001B[H";
        write(socketId, clear_str, strlen(clear_str));
    } else
        system("clear");
}

const string Connection::endl = Utils::getStdEndlString();

const string Connection::GOTO_BEGIN = "\u001B[48D";

ConnectionClosed::ConnectionClosed(int socketId) : whatMessage(
        string("Conection with socket ") + socketId + "was closed!") {}

const char *ConnectionClosed::what() const throw() {
    return whatMessage.c_str();
}