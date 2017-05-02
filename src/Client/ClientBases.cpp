//#include "../../headers/Client.hpp"
//#include "../../headers/Sockets.hpp"
//#include "../../headers/Utils.hpp"
//
//using namespace std;
//
////---------------------------PrivilegeGroup---------------------------//
//
//PrivilegeGroup::PrivilegeGroup(ClientContainer& clientContainer) : clientContainer(clientContainer) {}
//
//void PrivilegeGroup::writeline(int socketfd, string line,bool paragraph) const {
//    Network::server().writeline(socketfd,line,paragraph);
//}
//
//void PrivilegeGroup::writeline(std::string line, bool paragraph) const {
//    Network::server().writeline(clientContainer.client_socket,line,paragraph);
//}
//
////---------------------------BasicUser---------------------------//
//
//BasicUser::BasicUser(ClientContainer&) :
//        PrivilegeGroup(clientContainer) {}
//
//std::string BasicUser::getGroupHelp() const {
//    stringstream ret;
//    ret << Utils::makeHeader("Basic User Operations") << endl;
//    ret << Utils::makeCommandDescription("clear","Clears the window") << endl;
//    ret << Utils::makeCommandDescription("quit","Quits the connection to the server") << endl;
//    return ret.str();
//}
//
//void BasicUser::clear(int sid) {
//    if(sid>0)
//    {
//        char clear_str[]="\u001B[2J\u001B[H";
//        write(sid,clear_str,strlen(clear_str));
//    }
//    else
//        system("clear");
//}
//
//void BasicUser::clear() const {
//    clear(clientContainer.client_socket);
//}
//
//bool BasicUser::quit() const	//returns true for quitting and false for not quitting (not enough permissions)
//{
//    if(clientContainer.client_socket<=0)	//if not in server
//        Network::server().shutdown_server();
//    return true;
//}
