//#include "../../headers/Client.hpp"
//
////---------------------------LoggedOutClient---------------------------//
//
//LoggedOutClient::LoggedOutClient(ClientContainer &clientContainer) :
//        BasicUser(clientContainer),
//        LoggedOut(clientContainer) {}
//
//void LoggedOutClient::help() const {
//
//}
//
////---------------------------RegularClient---------------------------//
//
//RegularClient::RegularClient(ClientContainer &clientContainer) :
//        BasicUser(clientContainer),
//        LoggedIn(clientContainer),
//        SelfManagement(clientContainer) {}
//
////---------------------------AdminClient---------------------------//
//
//AdminClient::AdminClient(ClientContainer &clientContainer) :
//        RegularClient(clientContainer),
//        UsersManagement(clientContainer),
//        HorsesManagement(clientContainer),
//        RacesManagement(clientContainer),
//        BetsManagement(clientContainer) {}