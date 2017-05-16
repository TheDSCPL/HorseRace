#include "../../headers/Client.hpp"

std::vector<CommandSet> &SuperClient::getCommandSets() {
    return commandSet;
}

//---------------------------LoggedOutClient---------------------------//

LoggedOutClient::LoggedOutClient(ClientContainer &clientContainer) :
        BasicU(clientContainer),
        LoggedOut(clientContainer) {
    commandSet.push_back({BasicU::getThis(), BasicU::getCommands()});
    commandSet.push_back({LoggedOut::getThis(), LoggedOut::getCommands()});
}

//---------------------------RegularClient---------------------------//

RegularClient::RegularClient(ClientContainer &clientContainer) :
        BasicU(clientContainer),
        LoggedIn(clientContainer),
        SelfManagement(clientContainer) {
    commandSet.push_back({BasicU::getThis(), BasicU::getCommands()});
    commandSet.push_back({LoggedIn::getThis(), LoggedIn::getCommands()});
    commandSet.push_back({SelfManagement::getThis(), SelfManagement::getCommands()});
}

//---------------------------AdminClient---------------------------//

AdminClient::AdminClient(ClientContainer &clientContainer) :
        BasicU(clientContainer),
        LoggedIn(clientContainer),
        SelfManagement(clientContainer),
        UsersManagement(clientContainer),
        HorsesManagement(clientContainer),
        //BetsManagement(clientContainer),
        RacesManagement(clientContainer) {
    commandSet.push_back({BasicU::getThis(), BasicU::getCommands()});
    commandSet.push_back({LoggedIn::getThis(), LoggedIn::getCommands()});
    commandSet.push_back({SelfManagement::getThis(), SelfManagement::getCommands()});
    commandSet.push_back({UsersManagement::getThis(), UsersManagement::getCommands()});
    commandSet.push_back({HorsesManagement::getThis(), HorsesManagement::getCommands()});
//    commandSet.push_back({BetsManagement::getThis(), BetsManagement::getCommands()});
    commandSet.push_back({RacesManagement::getThis(), RacesManagement::getCommands()});
}