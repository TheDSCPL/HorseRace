//#include "../../headers/Constants.hpp"
//#include "../../headers/Client.hpp"
//#include "../../headers/Utils.hpp"
//#include "../../headers/DBMS.hpp"
//#include "../../headers/Log.hpp"
//#include <sstream>
//#include "../../headers/Sockets.hpp"
//#include "../../headers/Race.hpp"
//
//using namespace std;
//using namespace Constants;
//
////---------------------------LoggedOut---------------------------//
////DONE
//
//LoggedOut::LoggedOut(ClientContainer &clientContainer) :
//        PrivilegeGroup(clientContainer) {}
//
//std::string LoggedOut::getGroupHelp() const {
//    stringstream ret;
//    ret << Utils::makeHeader("Logged out operations") << endl;
//    ret << Utils::makeCommandDescription("regist <username> <password> <name>","Registers a new user") << endl;
//    ret << Utils::makeCommandDescription("login <username> <password>","Logs the user in" + string((clientContainer.client_socket==Constants::IN_SERVER)?" | Note: Only admins can login in the server":""));
//    return ret.str();
//}
//
//void LoggedOut::regist(std::string login_name, std::string pass, std::string name, bool ad=false, int cr=Constants::DEFAULT_CREDITS) {
//    using namespace SQLPreparedStatementsNames;
//
//    if(UsersManagement::get_user_id(login_name) > 0) {
//        writeline("User already exists!");
//        return;
//    }
//
//    try {
//        const PreparedStatement* ps = S.getPreparedStatement(registerNewUser);
//        ClientError::throwIf(!ps);
//        transform(login_name.begin(),login_name.end(),login_name.begin(),::tolower);
//
//        SQLResult res = ps->run({name, login_name, pass, Utils::b2s(ad), to_string(cr)});
//
//        ClientError::throwIf(res.hasError(), "Error while running the command: " + res.getErrorMessage());
//
//        clientContainer.user_id = UsersManagement::get_user_id(login_name);
//        clientContainer.username = login_name;
//        clientContainer.name = name;
//        clientContainer.changeUserType(ClientType::REGULAR_CLIENT);
//    } catch (DBMSErrorRunningPreparedStatement& err) {
//        clog((string)"Error in the register method: " + err.what());
//        throw ClientError();
//    }
//}
//
//void LoggedOut::login(std::string login_name, std::string pass) {
//    using namespace SQLPreparedStatementsNames;
//    const PreparedStatement* ps = S.getPreparedStatement(checkUserAndPass);
//    ClientError::throwIf(!ps);
//    try {
//        transform(login_name.begin(),login_name.end(),login_name.begin(),::tolower);
//        SQLResult res = ps->run({login_name, pass});
//        ClientError::throwIf(
//                res.hasError() || !res.getResultTable(),
//                "Error while running the command: " + res.getErrorMessage()
//        );
//        const SQLResultTable& table = res.getResultTable();
//        if(table.getNumberOfTuples()) {
//            clientContainer.user_id = table[0]->getInt(0);
//            clientContainer.name = table[0]->getString(1);
//            clientContainer.username = login_name;
//            clientContainer.changeUserType(table[0]->getBool(2) ? ClientType::ADMIN_CLIENT : ClientType::REGULAR_CLIENT);
//        } else {
//
//        }
//    } catch (DBMSErrorRunningPreparedStatement& err) {
//        clog((string)"Error in the register method: " + err.what());
//        throw ClientError();
//    } catch (ClientError& err) {
//        throw;
//    }
//}
//
////---------------------------LoggedIn---------------------------//
//
//LoggedIn::LoggedIn(ClientContainer &clientContainer) : PrivilegeGroup(clientContainer) {}
//
//std::string LoggedIn::getGroupHelp() const {
//    stringstream ret;
//    ret << Utils::makeHeader("Logged in user operations") << endl;
//    ret << Utils::makeCommandDescription("watch_race <race_id>","start watching a race") << endl;
//    ret << Utils::makeCommandDescription("logout","logs off the user");
//    return ret.str();
//}
//
////TODO: review later
//void LoggedIn::watch_race(int r_i) {
//    if(!RacesManagement::check_race(r_i)) {
//        writeline("Invalid race.");
//        return;
//    }
//    auto temp=races.find(r_i);
//    if(temp==races.end())
//    {
//        writeline("Race not active.");
//        return;
//    }
//    if(temp->second->sockets_watching.find(clientContainer.client_socket)!=temp->second->sockets_watching.end())
//    {
//        writeline("Client already watching the race.");
//    }
//    BasicUser::clear(clientContainer.client_socket);
//    string dummy;
//    temp->second->sockets_watching.insert(clientContainer.client_socket);
//    Network::server().readline(clientContainer.client_socket,dummy);
//    usleep(100);
//    BasicUser::clear(clientContainer.client_socket);
//    temp=races.find(r_i);
//    if(temp==races.end())
//        return;
//    temp->second->sockets_watching.erase(clientContainer.client_socket);
//}
//
//void LoggedIn::logout() {
//    Network::server().clients[clientContainer.client_socket]=clientContainer.user_id=LOGGED_OFF;
//    clog("Socket " << clientContainer.client_socket << " which was logged with user_id=" << clientContainer.user_id << ", username \"" << clientContainer.username << "\" and name \"" << clientContainer.name << "\" has logged off");
//    clientContainer.name="";
//    clientContainer.username="";
//    clientContainer.user_id = Constants::LOGGED_OFF;
//    writeline("You are now logged off!");
//    clientContainer.changeUserType(ClientType::LOGGED_OUT);
//}
//
////---------------------------SelfManagement---------------------------//
//
//SelfManagement::SelfManagement(ClientContainer& clientContainer) : PrivilegeGroup(clientContainer) {}
//
//std::string SelfManagement::getGroupHelp() const {
//    stringstream ret;
//    ret << Utils::makeHeader("Self Account Management") << endl;
//    ret << Utils::makeCommandDescription("passwd <old_pass> <new_pass> <new_pass>","Change your password") << endl;
//    return ret.str();
//}
//
//void SelfManagement::passwd(std::string old_pass, std::string new_pass) const {
//    using namespace SQLPreparedStatementsNames;
//    try {
//        //Check old password
//        {
//            const PreparedStatement* ps = S.getPreparedStatement(checkUserAndPass);
//            if(!ps)
//                throw ClientError("Something went wrong!");
//            SQLResult res = ps->run({clientContainer.username, old_pass});
//            if(!res || res.hasError() || !res.getResultTable()) {
//                throw ClientError("Error while running the command: " + res.getErrorMessage());
//            }
//            const SQLResultTable& table = res.getResultTable();
//            if(!table.getNumberOfTuples()) {
//                writeline("The inserted previous password is incorrect!");
//            }
//        }
//
//        //change the password
//        {
//            const PreparedStatement* ps = S.getPreparedStatement(changePassword);
//            ClientError::throwIf(!ps);
//            SQLResult res = ps->run({to_string(clientContainer.user_id),new_pass, old_pass});
//            ClientError::throwIf(res.hasError());
//        }
//    } catch (DBMSErrorRunningPreparedStatement& err) {
//        clog((string)"Error in the register method: " + err.what());
//        throw ClientError("Something went wrong!");
//    } catch (ClientError& err) {
//        throw;
//    }
//}
//
////---------------------------UsersManagement---------------------------//
//
//UsersManagement::UsersManagement(ClientContainer& clientContainer) : PrivilegeGroup(clientContainer) {}
//
//std::string UsersManagement::getGroupHelp() const {
//    stringstream ret;
//    ret << Utils::makeHeader("Users Management") << endl;
//    ret << Utils::makeCommandDescription("change_admin <username> <bool>","to change user rights") << endl;
//    ret << Utils::makeCommandDescriptionNote("luis can never stop being the admin! he's all powerful") << endl;
//    ret << Utils::makeCommandDescription("change_admin_id <user_id> <bool>","to change user rights") << endl;
//    ret << Utils::makeCommandDescriptionNote("luis can never stop being the admin! he's all powerful") << endl;
//    ret << Utils::makeCommandDescription("passwd_other <username> <new_pass> <new_pass>","change a user's password") << endl;
//    ret << Utils::makeCommandDescription("passwd_other_id <username> <new_pass> <new_pass>","change a user's password") << endl;
//    ret << Utils::makeCommandDescription("show_users","show all users in the system") << endl;
//    ret << Utils::makeCommandDescription("show_logged_users","show all users that are logged in") << endl;
//    ret << Utils::makeCommandDescription("show_bets_other <user_id> <lim>","same as \"show_bets_other <lim>\" but see another user's ") << endl;
//    ret << Utils::makeCommandDescription("add_horse <speed> <horse name>","add a new horse to the system") << endl;
//    ret << Utils::makeCommandDescription("bet_other <user_id> <horse_id> <race_id> <ammount>","") << endl;
//    ret << Utils::makeCommandDescription("","") << endl;
//    return ret.str();
//}
//
////---------------------------HorsesManagement---------------------------//
//
//HorsesManagement::HorsesManagement(ClientContainer& clientContainer) : PrivilegeGroup(clientContainer) {}
//
////---------------------------RacesManagement---------------------------//
//
//RacesManagement::RacesManagement(ClientContainer& clientContainer) : PrivilegeGroup(clientContainer) {}
//
////---------------------------BetsManagement---------------------------//
//
//BetsManagement::BetsManagement(ClientContainer& clientContainer) : PrivilegeGroup(clientContainer) {}