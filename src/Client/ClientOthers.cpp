#include "../../headers/Constants.hpp"
#include "../../headers/Client.hpp"
#include "../../headers/Utils.hpp"
#include "../../headers/DBMS.hpp"
#include <sstream>

using namespace std;
using namespace Constants;

//---------------------------ClientContainer---------------------------//


ClientContainer::ClientContainer(int cl_so) :
        Connection(cl_so),
        currentUser(nullptr),
        user_id(LOGGED_OFF),
        username(""),
        name("") {
    //currentUser = new LoggedOutClient(*this);
}

ClientContainer::~ClientContainer() {
//    if(currentUser != nullptr)
//        delete currentUser;
//    currentUser = nullptr;
}

SuperClient *const ClientContainer::getCurrentUser() {
    return currentUser;
}

void ClientContainer::changeUserType(Constants::ClientType type) {
    using namespace Constants;
    currentUser = nullptr;
    switch (type) {
        case ClientType::LOGGED_OUT:
            currentUser = new LoggedOutClient(*this);
            break;
        case ClientType::REGULAR_CLIENT:
            currentUser = new RegularClient(*this);
            break;
        case ClientType::ADMIN_CLIENT:
            //currentUser = new RegularClient(*this);
            currentUser = new AdminClient(*this);
            break;
    }
}

bool ClientContainer::isInServer() const {
    return socketId == Constants::IN_SERVER;
}

void ClientContainer::initPreparedStatements() {
    using namespace SQLPreparedStatementsNames;
    //name, username, password, admin, credits
    if (!S.getPreparedStatement(registerNewUser)) {
        stringstream query;
        query << "INSERT INTO users VALUES (DEFAULT, $1, $2, $3, $4, $5);" << endl;
        S.requestNewPreparedStatement(registerNewUser, query.str());
    }
    //login_name
    if (!S.getPreparedStatement(getUserId)) {
        stringstream query;
        query << "SELECT user_id, name" << endl;
        query << "FROM users" << endl;
        query << "WHERE username = $1;";
        S.requestNewPreparedStatement(getUserId, query.str());
    }
    //username, hashed_pass
    if (!S.getPreparedStatement(checkUserAndPass)) {
        stringstream query;
        query << "SELECT user_id, name, admin" << endl;
        query << "FROM users" << endl;
        query << "WHERE username = $1 AND pass = $2;" << endl;
        S.requestNewPreparedStatement(checkUserAndPass, query.str());
    }
    //user_id
    if (!S.getPreparedStatement(checkAdmin)) {
        stringstream query;
        query << "SELECT admin FROM users WHERE user_id = $1;" << endl;
        S.requestNewPreparedStatement(checkAdmin, query.str());
    }
    //user_id, new_pass, old_pass
    if (!S.getPreparedStatement(changePassword)) {
        stringstream query;
        query << "UPDATE users" << endl;
        query << "SET pass = $2" << endl;
        query << "WHERE user_id = $1 AND pass = $3;";
        S.requestNewPreparedStatement(changePassword, query.str());
    }
    //user_id, admin
    if (!S.getPreparedStatement(changeAdmin)) {
        stringstream query;
        query << "UPDATE users" << endl;
        query << "SET admin = $2" << endl;
        query << "WHERE user_id = $1;";
        S.requestNewPreparedStatement(changeAdmin, query.str());
    }
    //horse_name
    if (!S.getPreparedStatement(getHorseId)) {
        stringstream query;
        query << "SELECT horse_id" << endl;
        query << "FROM horses" << endl;
        query << "WHERE name = $1;";
        S.requestNewPreparedStatement(getHorseId, query.str());
    }
    //horse_name, speed
    if (!S.getPreparedStatement(insertHorse)) {
        stringstream query;
        query << "INSERT INTO horses VALUES (DEFAULT,$1,$2);" << endl;
        S.requestNewPreparedStatement(insertHorse, query.str());
    }
    //horse_id, race_id, state
    if (!S.getPreparedStatement(insertHorseInRace)) {
        stringstream query;
        query << "INSERT INTO are_on VALUES ($1,$2,$3,NULL);" << endl;
        S.requestNewPreparedStatement(insertHorseInRace, query.str());
    }
    //laps
    if (!S.getPreparedStatement(insertRace)) {
        stringstream query;
        query << "INSERT INTO races VALUES (DEFAULT,$1,DEFAULT,DEFAULT);" << endl;
        S.requestNewPreparedStatement(insertRace, query.str());
    }
    //horse_id, race_id
    if (!S.getPreparedStatement(checkHorseOnRace)) {
        stringstream query;
        query << "SELECT *" << endl;
        query << "FROM are_on" << endl;
        query << "WHERE horse_id=$1 AND race_id=$2";
        S.requestNewPreparedStatement(checkHorseOnRace, query.str());
    }
    //
    if (!S.getPreparedStatement(getLatestHorseId)) {
        stringstream query;
        query << "SELECT MAX(horse_id)" << endl;
        query << "FROM horses;" << endl;
        S.requestNewPreparedStatement(getLatestHorseId, query.str());
    }
    //
    if (!S.getPreparedStatement(getLatestRaceId)) {
        stringstream query;
        query << "SELECT race_id" << endl;
        query << "FROM races" << endl;
        query << "ORDER BY race_id DESC" << endl;
        query << "LIMIT 1;" << endl;
        S.requestNewPreparedStatement(getLatestRaceId, query.str());
    }
    //race_id
    if (!S.getPreparedStatement(checkRaceExists)) {
        stringstream query;
        query << "SELECT race_id" << endl;
        query << "FROM races" << endl;
        query << "WHERE race_id = $1;";
        S.requestNewPreparedStatement(checkRaceExists, query.str());
    }
    //horse_id
    if (!S.getPreparedStatement(checkHorseExists)) {
        stringstream query;
        query << "SELECT horse_id" << endl;
        query << "FROM horses" << endl;
        query << "WHERE horse_id = $1;";
        S.requestNewPreparedStatement(checkHorseExists, query.str());
    }
    //user_id
    if (!S.getPreparedStatement(checkUserExists)) {
        stringstream query;
        query << "SELECT user_id" << endl;
        query << "FROM users" << endl;
        query << "WHERE user_id = $1;";
        S.requestNewPreparedStatement(checkUserExists, query.str());
    }
    //user_id
    if (!S.getPreparedStatement(getUserCredits)) {
        stringstream query;
        query << "SELECT credits" << endl;
        query << "FROM users" << endl;
        query << "WHERE user_id = $1;";
        S.requestNewPreparedStatement(getUserCredits, query.str());
    }
    //race_id
    if (!S.getPreparedStatement(getRaceDate)) {
        stringstream query;
        query << "SELECT time_created" << endl;
        query << "FROM races" << endl;
        query << "WHERE race_id = $1;";
        S.requestNewPreparedStatement(getRaceDate, query.str());
    }
    //race_id
    if (!S.getPreparedStatement(getNumHorsesOnRace)) {
        stringstream query;
        query << "SELECT  COUNT(horse_id)" << endl;
        query << "FROM    are_on" << endl;
        query << "WHERE   race_id = $1;";
        S.requestNewPreparedStatement(getNumHorsesOnRace, query.str());
    }
    //race_id
    if (!S.getPreparedStatement(getHorsesOnRace)) {
        stringstream query;
        query << "SELECT horse_id,name,speed" << endl;
        query << "FROM are_on JOIN horses USING (horse_id)" << endl;
        query << "WHERE race_id = $1" << endl;
        query << "ORDER BY horse_id;" << endl;
        S.requestNewPreparedStatement(getHorsesOnRace, query.str());
    }
    //user_id, horse_id, race_id
    if (!S.getPreparedStatement(checkBetExists)) {
        stringstream query;
        query << "SELECT bet" << endl;
        query << "FROM bets" << endl;
        query << "WHERE user_id = $1 AND horse_id = $2 AND race_id = $3;";
        S.requestNewPreparedStatement(checkBetExists, query.str());
    }
    //user_id, credits delta
    if (!S.getPreparedStatement(addCredits)) {
        stringstream query;
        query << "UPDATE users" << endl;
        query << "SET credits = credits + $2" << endl;
        query << "WHERE user_id = $1;" << endl;
        S.requestNewPreparedStatement(addCredits, query.str());
    }
    //race_id
    if (!S.getPreparedStatement(checkRaceStarted)) {
        stringstream query;
        query << "SELECT started FROM races WHERE race_id = $1;";
        S.requestNewPreparedStatement(checkRaceStarted, query.str());
    }
    //user_id, horse_id, race_id, new_bet
    if (!S.getPreparedStatement(changeBet)) {
        stringstream query;
        query << "SELECT changeBet($1,$2,$3,$4);" << endl;
        S.requestNewPreparedStatement(changeBet, query.str());
    }
    //
    if (!S.getPreparedStatement(getAllRaces)) {
        stringstream query;
        query << "SELECT *" << endl;
        query << "FROM races" << endl;
        query << "ORDER BY race_id;" << endl;
        S.requestNewPreparedStatement(getAllRaces, query.str());
    }
    //race_id
    if (!S.getPreparedStatement(hasRaceStarted)) {
        stringstream query;
        query << "SELECT started" << endl;
        query << "FROM races" << endl;
        query << "WHERE race_id = $1;";
        S.requestNewPreparedStatement(hasRaceStarted, query.str());
    }
    //race_id
    if (!S.getPreparedStatement(getRaceLaps)) {
        stringstream query;
        query << "SELECT laps" << endl;
        query << "FROM races" << endl;
        query << "WHERE race_id = $1;";
        S.requestNewPreparedStatement(getRaceLaps, query.str());
    }
    //maxNumberOfHorses (if 0 or negative, shows 5 horses)
    if (!S.getPreparedStatement(getHorseRanks)) {
        stringstream query;
        query << "SELECT *" << endl;
        query << "FROM horse_ranks" << endl;
        query << "LIMIT CASE WHEN $1 <= 0 THEN 5 ELSE $1 END;";
        S.requestNewPreparedStatement(getHorseRanks, query.str());
    }
    //user_id, maxNumberOfHorses
    if (!S.getPreparedStatement(getBetsPerUser)) {
        stringstream query;
        query << "SELECT *" << endl;
        query << "FROM bets" << endl;
        query << "WHERE user_id = $1" << endl;
        query << "ORDER BY race_id DESC , horse_id" << endl;
        query << "LIMIT CASE WHEN $2 < 0 THEN 5 ELSE $2 END;";
        S.requestNewPreparedStatement(getBetsPerUser, query.str());
    }
    //
    if (!S.getPreparedStatement(getAllUsers)) {
        stringstream query;
        query << "SELECT user_id,name,username,admin,credits" << endl;
        query << "FROM users" << endl;
        query << "ORDER BY user_id;" << endl;
        S.requestNewPreparedStatement(getAllUsers, query.str());
    }
    /*if (!S.getPreparedStatement(getLoggedInUsers_dropTempTableIfExists)) {
        stringstream query;
        query << "DROP TABLE IF EXISTS foo;" << endl;
        S.requestNewPreparedStatement(getLoggedInUsers_dropTempTableIfExists, query.str());
    }
    //
    if (!S.getPreparedStatement(getLoggedInUsers_createTempTable)) {
        stringstream query;
        query << "CREATE TEMP TABLE foo (" << endl;
        query << "user_id integer NOT NULL," << endl;
        query << "client_socket varchar NOT NULL);" << endl;
        S.requestNewPreparedStatement(getLoggedInUsers_createTempTable, query.str());
    }
    //user_id, socket OR "'SERVER'" if logged in the server
    if (!S.getPreparedStatement(getLoggedInUsers_insertIntoTempTable)) {
        stringstream query;
        query << "INSERT INTO foo VALUES($1,$2);";
        S.requestNewPreparedStatement(getLoggedInUsers_insertIntoTempTable, query.str());
    }
    //
    if (!S.getPreparedStatement(getLoggedInUsers_getLoggedInUsers)) {
        stringstream query;
        query << "SELECT user_id,client_socket,name,username,admin,credits" << endl;
        query << "FROM users RIGHT JOIN foo USING (user_id)" << endl;
        query << "ORDER BY user_id;" << endl;
        S.requestNewPreparedStatement(getLoggedInUsers_getLoggedInUsers, query.str());
    }
    //
    if (!S.getPreparedStatement(getLoggedInUsers_destroyTempTable)) {
        stringstream query;
        query << "DISCARD TEMP;" << endl;
        S.requestNewPreparedStatement(getLoggedInUsers_destroyTempTable, query.str());
    }*/
}


std::vector<CommandSet> &ClientContainer::getCommandSets() {
    if (currentUser == nullptr)
        //currentUser = unique_ptr<SuperClient> (new LoggedOutClient(*this));
        currentUser = new LoggedOutClient(*this);
    return currentUser->getCommandSets();
}

////---------------------------Command---------------------------//

Command::Command(const std::string &name, const std::string &description,
                 const std::vector<std::string> &notes, std::function<void()> _run) : name(name),
                                                                                      description(description),
                                                                                      notes(notes), _run(_run) {}

std::string Command::getHelp() const {
    stringstream ret;
    ret << Utils::makeCommandDescription(name, description) << endl;
    for (const string &s: notes)
        ret << Utils::makeCommandDescriptionNote(s) << endl;
    return ret.str();
}

std::string Command::getName() const {
    return name;
}

void Command::run() const {
    if (_run)
        _run();
}

//---------------------------ClientError---------------------------//

ClientError::ClientError(const std::string &s) : whatMessage(s) {}

const char *ClientError::what() const throw() {
    return whatMessage.c_str();
}

void ClientError::throwIf(bool condition, const string &error) {
    if (condition)
        throw ClientError(error);
}

void ClientError::show(const Connection &conn) const {
    conn << Connection::GOTO_BEGIN << "Error: " << what() << Connection::endl;
}

//---------------------------ClientMessage---------------------------//

ClientMessage::ClientMessage(const std::string &s) : whatMessage(s) {}

const char *ClientMessage::what() const throw() {
    return whatMessage.c_str();
}

void ClientMessage::show(const Connection &conn) const {
    conn << Connection::GOTO_BEGIN << what() << Connection::endl;
}