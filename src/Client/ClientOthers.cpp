//#include "../../headers/Constants.hpp"
//#include "../../headers/Client.hpp"
//#include "../../headers/Utils.hpp"
//#include <sstream>
//
//using namespace std;
//using namespace Constants;
//
////---------------------------ClientContainer---------------------------//
//
//
//ClientContainer::ClientContainer(int cl_so) :
//        currentUser(new LoggedOutClient(*this)),
//        client_socket(cl_so),
//        username(""),
//        name(""),
//        user_id(LOGGED_OFF) {}
//
//unique_ptr<BasicUser>& ClientContainer::getCurrentUser() {
//    return currentUser;
//}
//
//void ClientContainer::changeUserType(Constants::ClientType type) {
//    using namespace Constants;
//    switch (type) {
//        case ClientType::LOGGED_OUT:
//            currentUser = unique_ptr<BasicUser>(new LoggedOutClient(*this));
//            break;
//        case ClientType::REGULAR_CLIENT:
//            currentUser = unique_ptr<BasicUser>(new RegularClient(*this));
//            break;
//        case ClientType::ADMIN_CLIENT:
//            currentUser = unique_ptr<BasicUser>(new AdminClient(*this));
//            break;
//    }
//}
//
//void ClientContainer::clearCommands() {
//    for(Command* c : commands)
//        if(c)
//            delete c;
//    commands.clear();
//}
//
//void ClientContainer::addCommand(const std::string & name,
//                                 const std::string & description,
//                                 const std::vector<std::string> & notes,
//                                 std::function<void()> _run) {
//    commands.push_back(new Command(*this, name, description, notes, _run));
//}
//
//ClientContainer::~ClientContainer() {
//    for(Command * c :commands)
//        if(c)
//            delete c;
//}
//
////---------------------------Command---------------------------//
//
//Command::Command(ClientContainer & container,
//const std::string & name,
//const std::string & description,
//const std::vector<std::string> & notes,
//        std::function<void()> _run) : clientContainer(container),
//name(name),
//description(description),
//notes(notes),
//_run(_run) {}
//
//std::string Command::getHelp() const {
//    stringstream ret;
//    ret << Utils::makeCommandDescription(name,description) << endl;
//    for(const string& s : notes) {
//        ret << Utils::makeCommandDescriptionNote(s) << endl;
//    }
//    return ret.str();
//}
//
//void Command::run() const {
//    _run();
//}
//
////---------------------------ClientError---------------------------//
//
//ClientError::ClientError(const std::string & s) : whatMessage(s) {}
//
//const char* ClientError::what() const throw() {
//    return whatMessage.c_str();
//}
//
//void ClientError::throwIf(bool condition, const string& error) const throw(ClientError) {
//    if(condition)
//        throw ClientError(error);
//}