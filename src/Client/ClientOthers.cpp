#include "../../headers/Constants.hpp"
#include "../../headers/Client.hpp"
#include "../../headers/Utils.hpp"
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