#include "../../headers/Constants.hpp"
#include "../../headers/Client.hpp"
#include "../../headers/Utils.hpp"
#include "../../headers/DBMS.hpp"
#include "../../headers/Log.hpp"
#include <sstream>
#include "../../headers/Sockets.hpp"
#include "../../headers/Race.hpp"

using namespace std;
using namespace Constants;

BasicU::BasicU(ClientContainer &clientContainer) :
        PrivilegeGroup("Terminal management", clientContainer) {

    Command *_clear_command = new Command("clear", "Clears the terminal window", {},
                                          [this]() { const_cast<BasicU *>(this)->clear(); });
    commands.push_back(_clear_command);

    Command *_quit_command = new Command("quit", "Quits the connection to the server", {},
                                         [this]() { const_cast<BasicU *>(this)->quit(); });
    commands.push_back(_quit_command);

}

void BasicU::clear() {
    clientContainer.clear();
}

void BasicU::quit() {
    throw QuitClient();
}

LoggedOut::LoggedOut(ClientContainer &clientContainer) :
        PrivilegeGroup("Log in options", clientContainer) {

    Command *_regist = new Command("regist", "Registers a new user", {},
                                   [this]() { const_cast<LoggedOut *>(this)->regist(); });
    commands.push_back(_regist);

    vector<string> _login_notes;
    if (clientContainer.isInServer())
        _login_notes.push_back("Only admins can login in the server");
    Command *_login = new Command("login", "Logs the user in", _login_notes,
                                  [this]() { const_cast<LoggedOut *>(this)->login(); });
    commands.push_back(_login);

}

void LoggedOut::regist() {
    using namespace SQLPreparedStatementsNames;

    std::string login_name = clientContainer.getString("Insert login_name");
    std::string pass = clientContainer.getString("Insert password");
    std::string name = clientContainer.getString("Insert your name");
    bool ad = Constants::DEFAULT_ADMIN;
    int cr = Constants::DEFAULT_CREDITS;

    if (UsersManagement::get_user_id(login_name) > 0) {
        clientContainer << Connection::GOTO_BEGIN << "User already exists!" << Connection::endl;
        return;
    }

    try {
        const PreparedStatement *ps = S.getPreparedStatement(registerNewUser);
        ClientError::throwIf(!ps);
        transform(login_name.begin(), login_name.end(), login_name.begin(), ::tolower);

        SQLResult res = ps->run({name, login_name, pass, Utils::b2s(ad), to_string(cr)});

        ClientError::throwIf(res.hasError(), "Error while running the command: " + res.getErrorMessage());

        clientContainer.user_id = UsersManagement::get_user_id(login_name);
        clientContainer.username = login_name;
        clientContainer.name = name;
        clientContainer.changeUserType(ad ? ClientType::ADMIN_CLIENT : ClientType::REGULAR_CLIENT);
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the register method: " + err.what());
        showMessageDBMSError();
    } catch (ClientError &err) {
        err.show(clientContainer);
    }
}

void LoggedOut::login() {
    using namespace SQLPreparedStatementsNames;

    std::string login_name = clientContainer.getString("Login");
    std::string pass = clientContainer.getString("Password");

    const PreparedStatement *ps = S.getPreparedStatement(checkUserAndPass);
    ClientError::throwIf(!ps);
    try {
        transform(login_name.begin(), login_name.end(), login_name.begin(), ::tolower);
        SQLResult res = ps->run({login_name, pass});
        ClientError::throwIf(
                res.hasError() || !res.hasTableResult(),
                "Error while running the command: " + res.getErrorMessage()
        );
        const SQLResultTable &table = res.getResultTable();
        if (!table.getNumberOfTuples()) {
            clientContainer << Connection::GOTO_BEGIN << "Wrong credentials!" << Connection::endl;
        } else {
            clientContainer.user_id = table[0]->getInt(0);
            clientContainer.name = table[0]->getString(1);
            clientContainer.username = login_name;
            clientContainer.changeUserType(
                    table[0]->getBool(2) ? ClientType::ADMIN_CLIENT : ClientType::REGULAR_CLIENT);
            clientContainer << Connection::GOTO_BEGIN << "Welcome, " << clientContainer.name << "!" << Connection::endl;
        }
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the register method: " + err.what());
        showMessageDBMSError();
    } catch (ClientError &err) {
        err.show(clientContainer);
    }
}

//---------------------------LoggedIn---------------------------//

LoggedIn::LoggedIn(ClientContainer &clientContainer) : PrivilegeGroup("Normal user options", clientContainer) {

    Command *_watch_race = new Command("watch_race", "start watching a race", {},
                                       [this]() { const_cast<LoggedIn *>(this)->watch_race(); });
    commands.push_back(_watch_race);

    Command *_logout = new Command("logout", "logs off the user", {},
                                   [this]() { const_cast<LoggedIn *>(this)->logout(); });
    commands.push_back(_logout);

}

void LoggedIn::watch_race() {
    int r_i = clientContainer.getInt("Race_id");
    if (!RacesManagement::check_race(r_i)) {
        clientContainer << Connection::GOTO_BEGIN << "Invalid race." << Connection::endl;
        return;
    }
    auto temp = races.find(r_i);
    if (temp == races.end()) {
        clientContainer << Connection::GOTO_BEGIN << "Race not active." << Connection::endl;
        return;
    }
    if (temp->second->sockets_watching.find(clientContainer.getSocketId()) != temp->second->sockets_watching.end()) {
        clientContainer << Connection::GOTO_BEGIN << "Client already watching the race." << Connection::endl;
        return;
    }
    clientContainer.clear();
    string dummy;
    temp->second->sockets_watching.insert(clientContainer.getSocketId());
    clientContainer >> dummy;
    //Network::server().readline(clientContainer.getSocketId(),dummy);
    temp->second->sockets_watching.erase(clientContainer.getSocketId());
    usleep(100);
    clientContainer.clear();
}

void LoggedIn::logout() {
    Network::server().clients[clientContainer.getSocketId()] = clientContainer.user_id = LOGGED_OFF;
    clog("Socket " << clientContainer.getSocketId() << " which was logged with user_id=" << clientContainer.user_id
                   << ", username \"" << clientContainer.username << "\" and name \"" << clientContainer.name
                   << "\" has logged off");
    clientContainer.name = "";
    clientContainer.username = "";
    clientContainer.user_id = Constants::LOGGED_OFF;
    clientContainer << Connection::GOTO_BEGIN << "You are now logged off!" << Connection::endl;
    clientContainer.changeUserType(ClientType::LOGGED_OUT);
}

//---------------------------SelfManagement---------------------------//

SelfManagement::SelfManagement(ClientContainer &clientContainer) : PrivilegeGroup("Account management",
                                                                                  clientContainer) {

    Command *_logout = new Command("passwd", "changes the user's password", {},
                                   [this]() { const_cast<SelfManagement *>(this)->passwd(); });
    commands.push_back(_logout);

}

void SelfManagement::passwd() const {
    using namespace SQLPreparedStatementsNames;

    string old_pass = clientContainer.getString("Old password");
    string new_pass = clientContainer.getString("New password");

    try {
        UsersManagement::passwd(clientContainer.username, old_pass, new_pass);
    } catch (ClientMessage cm) {
        cm.show(clientContainer);
    }
}

//---------------------------UsersManagement---------------------------//

UsersManagement::UsersManagement(ClientContainer &clientContainer) : PrivilegeGroup("Manage other users",
                                                                                    clientContainer) {
    /*vector<string> _login_notes;
    if(clientContainer.isInServer())
        _login_notes.push_back("Only admins can login in the server");
    Command* _login = new Command("login", "Logs the user in", _login_notes, [this](){const_cast<LoggedOut*>(this)->login();});
    commands.push_back(_login);*/
    Command *_passwdOther = new Command("passwdOther", "change a user's password", {},
                                        [this]() { const_cast<UsersManagement *>(this)->passwdOther(); });
    commands.push_back(_passwdOther);

    Command *_isAdmin = new Command("isAdmin", "check if a user is an admin", {},
                                    [this]() { const_cast<UsersManagement *>(this)->isAdmin(); });
    commands.push_back(_isAdmin);

    vector<string> _change_admin_notes;
    if (clientContainer.isInServer())
        _change_admin_notes.push_back("lpcsd can never stop being the admin! he's all powerful");
    Command *_change_admin = new Command("change_admin", "change a user's admin privileges", _change_admin_notes,
                                         [this]() { const_cast<UsersManagement *>(this)->change_admin(); });
    commands.push_back(_change_admin);

    Command *_add_credits = new Command("add_credits", "add/remove (negative value) credits to/from a user", {},
                                        [this]() { const_cast<UsersManagement *>(this)->add_credits(); });
    commands.push_back(_add_credits);
}

//std::string UsersManagement::getGroupHelp() const {
//    stringstream ret;
//    ret << Utils::makeCommandDescription("show_users","show all users in the system") << endl;
//    ret << Utils::makeCommandDescription("show_logged_users","show all users that are logged in") << endl;
//    ret << Utils::makeCommandDescription("show_bets_other <user_id> <lim>","same as \"show_bets_other <lim>\" but see another user's ") << endl;
//    ret << Utils::makeCommandDescription("add_horse <speed> <horse name>","add a new horse to the system") << endl;
//    ret << Utils::makeCommandDescription("bet_other <user_id> <horse_id> <race_id> <ammount>","") << endl;
//    ret << Utils::makeCommandDescription("","") << endl;
//    return ret.str();
//}

int UsersManagement::get_user_id(std::string login_name) {
    using namespace SQLPreparedStatementsNames;
    const PreparedStatement *ps = S.getPreparedStatement(getUserId);
    ClientError::throwIf(!ps);
    try {
        transform(login_name.begin(), login_name.end(), login_name.begin(), ::tolower);
        SQLResult res = ps->run({login_name});
        ClientError::throwIf(
                res.hasError() || !res.hasTableResult(),
                "Error while running the command: " + res.getErrorMessage()
        );
        const SQLResultTable &table = res.getResultTable();
        if (!table.getNumberOfTuples()) {
            return Constants::LOGGED_OFF;
        } else {
            return table[0]->getInt(0);
        }
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the register method: " + err.what());
        throw ClientError();
    }
}

void UsersManagement::passwd(string username, std::string old_pass, std::string new_pass) {
    using namespace SQLPreparedStatementsNames;

    if (!UsersManagement::check_user(get_user_id(username)))
        throw ClientMessage("The inserted user doesn't exist!");


    try {
        //Check old password
        {
            const PreparedStatement *ps = S.getPreparedStatement(checkUserAndPass);
            if (!ps)
                throw ClientError("Something went wrong!");
            SQLResult res = ps->run({username, old_pass});
            if (!res || res.hasError() || !res.hasTableResult()) {
                throw ClientError("Error while running the command: " + res.getErrorMessage());
            }
            const SQLResultTable &table = res.getResultTable();
            if (!table.getNumberOfTuples()) {
                throw ClientMessage("The inserted previous password is incorrect!");
            }
        }

        //change the password
        {
            const PreparedStatement *ps = S.getPreparedStatement(changePassword);
            ClientError::throwIf(!ps);
            SQLResult res = ps->run({to_string(get_user_id(username)), new_pass, old_pass});
            ClientError::throwIf(res.hasError());
            throw ClientMessage("Your password was successfully changed!");
        }
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the passwd method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

void UsersManagement::passwdOther() const {
    using namespace SQLPreparedStatementsNames;

    string usern = clientContainer.getString("Username");
    string old_pass = clientContainer.getString("Old password");
    string new_pass = clientContainer.getString("New password");

    try {
        UsersManagement::passwd(usern, old_pass, new_pass);
    } catch (ClientMessage cm) {
        cm.show(clientContainer);
    }
}

bool UsersManagement::is_admin(int id) {
    using namespace SQLPreparedStatementsNames;

    try {
        //Check old password
        {
            const PreparedStatement *ps = S.getPreparedStatement(checkAdmin);
            if (!ps)
                throw ClientError("Something went wrong!");
            SQLResult res = ps->run({to_string(id)});
            if (!res || res.hasError() || !res.hasTableResult()) {
                throw ClientError("Error while running the command: " + res.getErrorMessage());
            }
            const SQLResultTable &table = res.getResultTable();
            if (!table.getNumberOfTuples()) {
                throw ClientMessage("The inserted username doesn't exist!");
            }
            return table[0]->getBool(0);
        }
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the is_admin method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

void UsersManagement::isAdmin() const {
    string usern = clientContainer.getString("Username");
    try {
        bool ad = UsersManagement::is_admin(usern);
        clientContainer << Connection::GOTO_BEGIN << "User is " << (ad ? ("") : ("not ")) << "admin"
                        << Connection::endl;
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

void UsersManagement::change_admin(int id, bool ad) {
    using namespace SQLPreparedStatementsNames;

    if (!UsersManagement::check_user(id))
        throw ClientMessage("The inserted user doesn't exist!");

    try {
        const PreparedStatement *ps = S.getPreparedStatement(changeAdmin);
        if (!ps)
            throw ClientError("Something went wrong!");
        SQLResult res = ps->run({to_string(id), Utils::b2s(ad)});
        if (!res || res.hasError()) {
            throw ClientError("Error while running the command: " + res.getErrorMessage());
        } else
            throw ClientMessage("Client administrative powers were changed");
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the change_admin method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

void UsersManagement::change_admin() const {
    string usern = clientContainer.getString("Username");
    bool adm = clientContainer.getBool("Has administrative powers (true/false):");

    try {
        UsersManagement::change_admin(usern, adm);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

void UsersManagement::add_credits(int u_i, double cr) {
    using namespace SQLPreparedStatementsNames;

    if (!UsersManagement::check_user(u_i))
        throw ClientMessage("The inserted user doesn't exist!");

    try {
        const PreparedStatement *ps = S.getPreparedStatement(addCredits);
        if (!ps)
            throw ClientError("Something went wrong!");
        SQLResult res = ps->run({to_string(u_i), to_string(cr)});
        if (!res || res.hasError()) {
            throw ClientError("Error while running the command: " + res.getErrorMessage());
        } else
            throw ClientMessage("Credits updated");
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the change_admin method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

void UsersManagement::add_credits() const {
    string usern = clientContainer.getString("Username");
    double delta = clientContainer.getDouble("Credits to add/remove");

    try {
        UsersManagement::add_credits(get_user_id(usern), delta);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

bool UsersManagement::check_user(int u_i) {
    using namespace SQLPreparedStatementsNames;

    try {
        //Check old password
        {
            const PreparedStatement *ps1 = S.getPreparedStatement(checkUserExists);
            if (!ps1)
                throw ClientError("Something went wrong!");
            SQLResult res1 = ps1->run({to_string(u_i)});
            if (!res1 || res1.hasError() || !res1.getResultTable()) {
                throw ClientError("Error while running the command: " + res1.getErrorMessage());
            }
            const SQLResultTable &table1 = res1.getResultTable();
            return !!table1.getNumberOfTuples();
        }
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the change_admin method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

//---------------------------HorsesManagement---------------------------//

//HorsesManagement::HorsesManagement(ClientContainer& clientContainer) : PrivilegeGroup(clientContainer) {}

bool HorsesManagement::check_horse(int h_i) {
    using namespace SQLPreparedStatementsNames;

    try {
        //Check old password
        {
            const PreparedStatement *ps1 = S.getPreparedStatement(checkHorseExists);
            if (!ps1)
                throw ClientError("Something went wrong!");
            SQLResult res1 = ps1->run({to_string(h_i)});
            if (!res1 || res1.hasError() || !res1.getResultTable()) {
                throw ClientError("Error while running the command: " + res1.getErrorMessage());
            }
            const SQLResultTable &table1 = res1.getResultTable();
            return table1.getNumberOfTuples() != 0;
        }
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the change_admin method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

//---------------------------RacesManagement---------------------------//

RacesManagement::RacesManagement(ClientContainer &clientContainer) : PrivilegeGroup("Races management",
                                                                                    clientContainer) {
    Command *_createRace = new Command("CreateRace", "creates a new race", {},
                                       [this]() { const_cast<RacesManagement *>(this)->add_race(); });
    commands.push_back(_createRace);
}

void RacesManagement::add_race(int laps) const {
    using namespace SQLPreparedStatementsNames;

    try {
        {
            const PreparedStatement *ps = S.getPreparedStatement(insertRace);
            if (!ps)
                throw ClientError("Something went wrong!");
            SQLResult res = ps->run({to_string(laps)});
            if (!res || res.hasError()) {
                throw ClientError("Error while running the command: " + res.getErrorMessage());
            }
        }
        {
            const PreparedStatement *ps = S.getPreparedStatement(SQLPreparedStatementsNames::getLatestRaceId);
            if (!ps)
                throw ClientError("Something went wrong!");
            SQLResult res = ps->run({to_string(laps)});
            if (!res || res.hasError() || !res.hasTableResult() || !res.getResultTable().getNumberOfTuples()) {
                throw ClientError("Error while running the command: " + res.getErrorMessage());
            }
            throw ClientMessage(string("Added race with id ") + res.getResultTable()[0]->getInt(0));
        }

    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the change_admin method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

void RacesManagement::add_to_race(int race, std::set<int> horses) const {
    using namespace SQLPreparedStatementsNames;

    if (!RacesManagement::check_race(race)) {
        clientContainer << Connection::GOTO_BEGIN << "Error: Inserted race doesn't exist!" << Connection::endl;
        return;
    }

    const PreparedStatement *ps = S.getPreparedStatement(SQLPreparedStatementsNames::insertHorseInRace);
    if (!ps)
        throw ClientError("Something went wrong!");

    for (int hi : horses) {
        if (get_num_horses_on_race(race) >= MAX_HORSES_ON_RACE) {
            clientContainer << Connection::GOTO_BEGIN
                            << "Couldn't add all of the inserted horses because the race is full (max is "
                            << MAX_HORSES_ON_RACE << " horses per race)" << Connection::endl;
            return;
        }
        if (!HorsesManagement::check_horse(hi)) {
            clientContainer << "Horse number " << hi << " doesn't exist. Ignoring..." << Connection::endl;
        } else if (RacesManagement::check_horse_in_race(hi, race)) {
            clientContainer << Connection::GOTO_BEGIN << "Horse number " << hi << " is already in the race " << race
                            << Connection::endl;
        } else {
            try {
                double state = (double) rand() / RAND_MAX / 2 + 0.5;    //random number between [0.5;1[
                SQLResult res = ps->run({to_string(hi), to_string(race), to_string(state)});
                if (!res || res.hasError()) {
                    throw ClientError("Error while running the command: " + res.getErrorMessage());
                }
                clientContainer << "Horse number " << hi << " added to race " << race << Connection::endl;
            } catch (DBMSErrorRunningPreparedStatement &err) {
                clog((string) "Error in the change_admin method: " + err.what());
                throw ClientError("Something went wrong!");
            }
        }
    }
}

void RacesManagement::start_race(int r_i) const {
    if (!check_race(r_i)) {
        clientContainer << Connection::GOTO_BEGIN << "Invalid race_id." << Connection::endl;
        return;
    }
    if (races.find(r_i) != races.end()) {
        clientContainer << Connection::GOTO_BEGIN << "Race already active." << Connection::endl;
        return;
    }
    if (get_race_started(r_i)) {
        clientContainer << Connection::GOTO_BEGIN << "Race already started once." << Connection::endl;
        return;
    }
    if (get_num_horses_on_race(r_i) < MIN_HORSES_ON_RACE) {
        clientContainer << Connection::GOTO_BEGIN << "Race doesn't have enough participants yet." << Connection::endl;
        return;
    }
    set<int> h = check_all_horses_available(r_i);
    if (!h.empty()) {
        clientContainer << Connection::GOTO_BEGIN
                        << "Race can't be started because the following horses are currently running: ";
        bool first = true;
        for (int hi : h) {
            if (!first)
                clientContainer << ", ";
            first = false;
            clientContainer << hi;
        }
        clientContainer << Connection::endl;
        return;
    }
    races.insert(pair<int, class Race *>(r_i, new Race(r_i, get_race_laps(r_i))));
    clientContainer << Connection::GOTO_BEGIN << "Race started." << Connection::endl;
}

set<int> RacesManagement::check_all_horses_available(int r_i) const {
    using namespace SQLPreparedStatementsNames;

    try {
        set<int> ret;
        const PreparedStatement *ps = S.getPreparedStatement(getHorsesOnRace);
        if (!ps)
            throw ClientError("Something went wrong!");
        SQLResult res = ps->run({to_string(r_i)});
        if (!res || res.hasError() || !res.hasTableResult()) {
            throw ClientError("Error while running the command: " + res.getErrorMessage());
        }
        const SQLResultTable &table = res.getResultTable();
        int n = table.getNumberOfTuples();
        for (int i = 0; i < n; i++)
            if (!check_horse_available(table[i]->getInt(0)))
                ret.insert(table[i]->getInt(0));
        return ret;
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the change_admin method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

bool RacesManagement::check_horse_available(int h_i) const {
    if (!HorsesManagement::check_horse(h_i))
        return false;
    for (auto &it:races) {
        if (it.second->finished)
            continue;    //if it is already finished, then the horse can be available even if it was in this race
        for (auto &i : it.second->horses) {
            if (i.first == h_i)
                return false;
        }
    }
    return true;
}

std::string RacesManagement::get_race_date(int r_i) const {
    if (!RacesManagement::check_race(r_i))
        throw ClientMessage("Inserted race doesn't exist");

    using namespace SQLPreparedStatementsNames;

    try {
        const PreparedStatement *ps = S.getPreparedStatement(getRaceDate);
        if (!ps)
            throw ClientError("Something went wrong!");
        SQLResult res = ps->run({to_string(r_i)});
        if (!res || res.hasError() || !res.hasTableResult() || res.getResultTable().getNumberOfTuples() <= 0) {
            throw ClientError("Error while running the command: " + res.getErrorMessage());
        }
        return res.getResultTable()[0]->getString(0);
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the get_race_date method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

int RacesManagement::get_num_horses_on_race(int r_i) const {
    if (!RacesManagement::check_race(r_i))
        throw ClientMessage("Inserted race doesn't exist");

    using namespace SQLPreparedStatementsNames;

    try {
        const PreparedStatement *ps = S.getPreparedStatement(getNumHorsesOnRace);
        if (!ps)
            throw ClientError("Something went wrong!");
        SQLResult res = ps->run({to_string(r_i)});
        if (!res || res.hasError() || !res.hasTableResult() || res.getResultTable().getNumberOfTuples() <= 0) {
            throw ClientError("Error while running the command: " + res.getErrorMessage());
        }
        return res.getResultTable()[0]->getInt(0);
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the get_race_date method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

bool RacesManagement::check_race_started(int r_i) const {
    if (!RacesManagement::check_race(r_i))
        throw ClientMessage("Inserted race doesn't exist");

    using namespace SQLPreparedStatementsNames;

    try {
        const PreparedStatement *ps = S.getPreparedStatement(checkRaceStarted);
        if (!ps)
            throw ClientError("Something went wrong!");
        SQLResult res = ps->run({to_string(r_i)});
        if (!res || res.hasError() || !res.hasTableResult() || res.getResultTable().getNumberOfTuples() <= 0) {
            throw ClientError("Error while running the command: " + res.getErrorMessage());
        }
        return res.getResultTable()[0]->getBool(0);
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the check_race_started method: " + err.what());
        throw ClientError("Something went wrong!");
    }
}

void RacesManagement::show_horses_on_race(int r_i) const {
    if (!RacesManagement::check_race(r_i))
        throw ClientMessage("Inserted race doesn't exist");

    using namespace SQLPreparedStatementsNames;

    const PreparedStatement *ps = S.getPreparedStatement(getHorsesOnRace);
    ClientError::throwIf(!ps);
    try {
        SQLResult res = ps->run({});
        ClientError::throwIf(
                res.hasError() || !res.hasTableResult(),
                "Error while running the command: " + res.getErrorMessage()
        );
        stringstream temp;
        res.getResultTable().print(temp);
        throw ClientMessage(temp.str());
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the check_horse_in_race method: " + err.what());
        throw ClientError();
    }
}

void RacesManagement::show_races(bool activeOnly) const {
    if (!activeOnly) { //all races
        using namespace SQLPreparedStatementsNames;

        const PreparedStatement *ps = S.getPreparedStatement(getAllRaces);
        ClientError::throwIf(!ps);
        try {
            SQLResult res = ps->run({});
            ClientError::throwIf(
                    res.hasError() || !res.hasTableResult(),
                    "Error while running the command: " + res.getErrorMessage()
            );
            stringstream temp;
            res.getResultTable().print(temp);
            throw ClientMessage(temp.str());
        } catch (DBMSErrorRunningPreparedStatement &err) {
            clog((string) "Error in the check_horse_in_race method: " + err.what());
            throw ClientError();
        }
    } else { //only active races
        if (races.size() == 0)
            throw ClientMessage("There are no active races at the moment.");
        stringstream output;
        output << "These are the active races:" << endl;
        for (auto &it:races) {
            output << "race_id=" + to_string(it.first) + " | #horses=" + to_string(it.second->horses.size()) << endl;
        }
        output << "To go watch any of these races, use the command \"watch_race <race_id>\"." << endl;
        throw ClientMessage(output.str());
    }
}

void RacesManagement::show_race_info(int r_i) const {
    if (!RacesManagement::check_race(r_i))
        throw ClientMessage("Inserted race doesn't exist");

    clientContainer << Connection::GOTO_BEGIN << "#horses=" << get_num_horses_on_race(r_i) << Connection::endl;
    clientContainer << Connection::GOTO_BEGIN << "Laps=" << get_race_laps(r_i) << Connection::endl;
    clientContainer << Connection::GOTO_BEGIN << "Time created=" << get_race_date(r_i) << Connection::endl;
    clientContainer << Connection::GOTO_BEGIN << "Started=" << get_race_started(r_i) << Connection::endl;
    auto temp = races.find(r_i);
    bool temp_b = temp != races.end();
    clientContainer << Connection::GOTO_BEGIN << "Active now=" << temp_b << Connection::endl;
    if (temp_b)
        clientContainer << Connection::GOTO_BEGIN << "Finished=" << temp->second->finished << Connection::endl;
    clientContainer << Connection::GOTO_BEGIN << "Horses on race:" << Connection::endl;
    show_horses_on_race(r_i);
}

bool RacesManagement::get_race_started(int r_i) const {
    using namespace SQLPreparedStatementsNames;

    if (!RacesManagement::check_race(r_i))
        throw ClientMessage("Inserted race doesn't exist");

    const PreparedStatement *ps = S.getPreparedStatement(checkRaceStarted);
    ClientError::throwIf(!ps);
    try {
        SQLResult res = ps->run({to_string(r_i)});
        ClientError::throwIf(
                res.hasError() || !res.hasTableResult() || res.getResultTable().getNumberOfTuples() <= 0,
                "Error while running the command: " + res.getErrorMessage()
        );
        const SQLResultTable &table = res.getResultTable();
        return table[0]->getBool(0);
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the check_horse_in_race method: " + err.what());
        throw ClientError();
    }
}

int RacesManagement::get_race_laps(int r_i) const {
    using namespace SQLPreparedStatementsNames;

    const PreparedStatement *ps = S.getPreparedStatement(getRaceLaps);
    ClientError::throwIf(!ps);
    try {
        SQLResult res = ps->run({to_string(r_i)});
        ClientError::throwIf(
                res.hasError() || !res.hasTableResult() || res.getResultTable().getNumberOfTuples() <= 0,
                "Error while running the command: " + res.getErrorMessage()
        );
        const SQLResultTable &table = res.getResultTable();
        return table[0]->getInt(0);
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the check_horse_in_race method: " + err.what());
        throw ClientError();
    }
}

bool RacesManagement::check_race(int r_i) {
    using namespace SQLPreparedStatementsNames;
    const PreparedStatement *ps = S.getPreparedStatement(checkRaceExists);
    ClientError::throwIf(!ps);
    try {
        SQLResult res = ps->run({to_string(r_i)});
        ClientError::throwIf(
                res.hasError() || !res.hasTableResult(),
                "Error while running the command: " + res.getErrorMessage()
        );
        const SQLResultTable &table = res.getResultTable();
        return table.getNumberOfTuples() != 0;
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the check_race method: " + err.what());
        throw ClientError();
    }
}

bool RacesManagement::check_horse_in_race(int h_i, int r_i) {
    using namespace SQLPreparedStatementsNames;

    const PreparedStatement *ps = S.getPreparedStatement(checkHorseOnRace);
    ClientError::throwIf(!ps);
    try {
        SQLResult res = ps->run({to_string(h_i), to_string(r_i)});
        ClientError::throwIf(
                res.hasError() || !res.hasTableResult(),
                "Error while running the command: " + res.getErrorMessage()
        );
        const SQLResultTable &table = res.getResultTable();
        return table.getNumberOfTuples() != 0;
    } catch (DBMSErrorRunningPreparedStatement &err) {
        clog((string) "Error in the check_horse_in_race method: " + err.what());
        throw ClientError();
    }
}


void RacesManagement::add_race() const {
    int laps = clientContainer.getInt("Number of laps");

    try {
        add_race(laps);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

void RacesManagement::add_to_race() const {

    int race = clientContainer.getInt("Race id");
    set<int> horses;
    for (int i = 0; 1; i++) {
        int hi = clientContainer.getInt(string("Horse #") + i);
        if (hi < 0)
            break;
        horses.insert(hi);
    }

    try {
        add_to_race(race, horses);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

void RacesManagement::start_race() const {
    int race = clientContainer.getInt("Race id");

    try {
        start_race(race);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

void RacesManagement::showAllRaces() const {
    try {
        show_races(false);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

void RacesManagement::showActiveRaces() const {
    try {
        show_races(true);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

void RacesManagement::showRaceInfo() const {
    int race = clientContainer.getInt("Race id");

    try {
        show_race_info(race);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

void RacesManagement::showHorsesOnRace() const {
    int race = clientContainer.getInt("Race id");

    try {
        show_horses_on_race(race);
    } catch (ClientMessage &cm) {
        cm.show(clientContainer);
    }
}

//////---------------------------BetsManagement---------------------------//
////
////BetsManagement::BetsManagement(ClientContainer& clientContainer) : PrivilegeGroup(clientContainer) {}