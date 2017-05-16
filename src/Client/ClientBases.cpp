#include "../../headers/Client.hpp"
#include "../../headers/Sockets.hpp"
#include "../../headers/Utils.hpp"

using namespace std;

//---------------------------PrivilegeGroup---------------------------//

PrivilegeGroup::PrivilegeGroup(const std::string &groupName, ClientContainer &clientContainer)
        : clientContainer(clientContainer), commands(), groupName(groupName) {}

PrivilegeGroup::~PrivilegeGroup() {
    if (!commands.empty()) {
        for (Command *c : commands)
            delete (c);
        commands.clear();
    }
}

std::vector<Command *> &PrivilegeGroup::getCommands() const {
    return commands;
}

std::string PrivilegeGroup::getGroupHelp() const {
    stringstream ret;
    ret << Utils::makeHeader(groupName);
    for (Command *c : getCommands())
        ret << c->getHelp();
    return ret.str();
}

void PrivilegeGroup::showMessageDBMSError() const {
    clientContainer << Connection::GOTO_BEGIN << "Internal SQL error! Contact admin!" << Connection::endl;
}

const std::string &PrivilegeGroup::getGroupName() const {
    return groupName;
}

PrivilegeGroup *PrivilegeGroup::getThis() {
    return this;
}