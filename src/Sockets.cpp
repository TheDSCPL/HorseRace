#include "../headers/Sockets.hpp"
#include "../headers/DBMS.hpp"
#include "../headers/Log.hpp"
#include "../headers/Client.hpp"
#include "../headers/Properties.hpp"
#include "../headers/Utils.hpp"
#include "../headers/Constants.hpp"

using namespace Constants;

#define BUFFER_SIZE (1025)

#define vcout if(false) cerr

pthread_mutex_t cout_mutex;

Network::Network() : server_running(false), Flag_shutdown(false), server_t(0) {
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
    shutdown(sockfd, 2);
    close(sockfd);
    clog("Network destroyed");
    vcout << "Network destroyed" << endl;
}

void Network::start_server() {
    clog("Starting server (\"start_server()\")");
    //S; //Start SQL server;
    if (server_running) {
        cerr << "Server's already running" << endl;
        clog("Error: start_server() -> Server is already running");
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

    if (pthread_create(&server_t, NULL, server_routine_redirect, (void *) this)) {
        cerr << "Error while starting server thread" << endl;
        clog("Error: start_server() -> Couldn't start a server thread");
        return;
    }
    //socket_threads.insert(server_t);

    cout << "Telnet server started on " << get_ip(sockfd) << ':' << Properties::getDefault().getProperty("PORT")
         << ". Accepting clients." << endl << endl;
    clog("Telnet server started on " << get_ip(sockfd) << ':' << Properties::getDefault().getProperty("PORT")
                                     << ". Accepting clients.");
    server_running = true;
}

void Network::writeline(int so, string line, bool paragraph) { // Envia uma string para um socket
    if (so < 0) {
        LOCK;
        cerr << line << endl;
        UNLOCK;
    }
    else {
        string tosend = "\u001B[48D" + line;
        if (paragraph) tosend += "\r\n";
        write(so, tosend.c_str(), tosend.length());
    }
}

char *Network::get_ip(int socket_ref) {
    //int fd;
    struct ifreq ifr;

    socket_ref = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

    ioctl(socket_ref, SIOCGIFADDR, &ifr);

    //close(socket_ref);

    return inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);
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
            if (n == 0)
                return false; // nada para ser lido -> socket fechado
            if (!isprint(buffer[0]))  //para a eliminar o lixo que aparece com telnet windows
                continue;
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
    while (true) {
        // Aceitar uma nova ligação. O endereço do cliente fica guardado em
        // cli_addr - endereço do cliente
        // newsockfd - id do socket que comunica com este cliente

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &client_addr_length);
        //vcout << "client accepted" << endl;

        if (Flag_shutdown || newsockfd < 0) {
            //vcout << "Shutdown signal recieved." << endl;
            break;
        }

        setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR, &enable_resuse_addr, sizeof(int));

        // Criar uma thread para tratar dos pedidos do novo cliente
        pthread_t thread;
        pthread_create(&thread, NULL, cliente_redirect, (void *) this);
        socket_threads.insert(thread);
        /*vcout << "Waiting" << endl;
        WAIT;
        vcout << "Done waiting" << endl;*/
    }
    shutdown(sockfd, 2);
    close(sockfd);
    //shutdown_server();
    //vcout << "Leaving server routine" << endl;
    server_running = Flag_shutdown = false;
    server_t = 0;

    //socket_threads.erase(pthread_self());
    clog("Leaving server routine");
}

void Network::shutdown_server() {
    if (!server_running)
        return;
    clog("Shutting down Network server");

    //termina o accept do server
    Flag_shutdown = true;
    int enable_resuse_addr = 1;
    int dummy = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(dummy, SOL_SOCKET, SO_REUSEADDR, &enable_resuse_addr, sizeof(int));
    connect(dummy, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    pthread_join(server_t, NULL);
    cout << "Network server is now shutdown!" << endl;
    shutdown(dummy, 2);
    close(dummy);

    vcout << "clients size=" << clients.size() << endl;
    int s_u_i = clients[IN_SERVER]; //to save the user that was logged in the server because i don't want to close the server client, just the network clients
    while (clients.size()) //fecha todas as sockets, terminando os reads e consequentemente as threads dos clientes
    {
        vcout << "aqui" << endl;
        if (clients.begin()->first != IN_SERVER) {
            shutdown(clients.begin()->first, 2);
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

    server_running = Flag_shutdown = false;
    server_t = 0;

    clog("Network server successfully shutdown");
}

// Trata de receber dados de um cliente cujo socketid foi passado como parâmetro
void Network::cliente() {
    int client_socket = newsockfd;

    //LOCK;cerr << "\u001B[8DNew client accepted on socket " << client_socket << "." << endl << "> ";UNLOCK;
    clog("New client accepted on socket " << client_socket << ". Thread where this socket is being heard: "
                                          << pthread_self());

    Client c(client_socket);  //start a new Clinet object
    string input;


    //--------------------------------CLIENT ROUTINE-----------------------------//
    //write(client_socket,"\u001B[44;31;0m",strlen("\u001B[44;31;0m"));
    write(client_socket, "> ", 2);

    while (readline(client_socket, input)) {
        //cout << input.size() << endl;
        if (input == "" || input == "\n" || input == "\r" || input == "\r\n")
            continue;
        if (!c.parse(input))
            break;
        //cout << "Socket " << client_socket << " said: " << input << endl;
        //broadcast(client_socket, input);
        write(client_socket, "> ", 2);
    }
    /*write(client_socket,"\u001B[2J\u001B[H",strlen("\u001B[2J\u001B[H"));
    writeline(client_socket,"ola");
    char s[]="nana\u001B[48D ole";
    write(client_socket,s,strlen(s));*/

    //----------------------------------CLIENT DISCONNECTING PROCESS---------------------------------//

    clients.erase(client_socket);
    shutdown(client_socket, 2);
    close(client_socket);
    socket_threads.erase(pthread_self());
}