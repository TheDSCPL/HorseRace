#ifndef CLIENT_HPP
#define CLIENT_HPP 1
#include <vector>
#include <typeinfo>
#include <boost/any.hpp>
#include <set>
#include <bits/unique_ptr.h>
#include <functional>
#include "../headers/Constants.hpp"
#include "Sockets.hpp"
//#include <boost/variant.hpp>
//using namespace boost;

typedef struct tm TM;

class arg;
typedef struct
{
  std::string cmd;
  std::vector<arg> args;
}parsed_command;

/*class Commands {
    Commands();
    Commands(const Commands&);
    void operator=(Commands const&);
public:
    static vector<arg>& getCommands();
}*/

//BASE CLASSES

class ClientContainer;

class IAmAbstract {
protected:
    virtual void helloImAnAbstractClass() = 0;
};

class Command;

class PrivilegeGroup : public virtual IAmAbstract {
protected:
    PrivilegeGroup(const std::string &groupName, ClientContainer &);

    virtual ~PrivilegeGroup();

    ClientContainer &clientContainer;

    std::vector<Command *> &getCommands() const;

    mutable std::vector<Command *> commands;
    const std::string groupName;

    void showMessageDBMSError() const;

public:
    std::string getGroupHelp() const;

    const std::string &getGroupName() const;

    PrivilegeGroup *getThis();
};

//PRIVILEGE GROUPS

//class BasicUser : public PrivilegeGroup {
//protected:
//    BasicUser(ClientContainer&);
//public:
//    void clear();
//    bool quit();
//    static void f();
//};

class BasicU : public PrivilegeGroup {
protected:
    BasicU(ClientContainer &clientContainer);

public:

    void clear();

    void quit();
};

class LoggedOut : public PrivilegeGroup {
protected:
    LoggedOut(ClientContainer &clientContainer);

public:
    void regist();

    void login();
};

class LoggedIn : public PrivilegeGroup {
protected:
    LoggedIn(ClientContainer &clientContainer);

public:
    void watch_race();

    void show_horses() const;

    void showMyUserID() const;

    void showUserCredits() const;

    void logout();
};

class SelfManagement : public PrivilegeGroup {
protected:
    SelfManagement(ClientContainer &);

public:

    void passwd() const;
    //Add bet functions here
};

class UsersManagement : public PrivilegeGroup {
protected:
    UsersManagement(ClientContainer &);

public:
    static int get_user_id(std::string login_name);

    static void passwd(std::string login_name, std::string old_pass, std::string new_pass);

    void passwdOther() const;

    static bool is_admin(int id);

    static bool is_admin(std::string login_name) {
        return is_admin(get_user_id(login_name));
    }

    void isAdmin() const;

    void showUserCredits() const;

    static double get_user_credits(int ui);

    static void change_admin(int id, bool ad);

    static void change_admin(std::string login_name, bool ad) {
        change_admin(get_user_id(login_name), ad);
    }

    void change_admin() const;

    static void add_credits(int u_i, double cr);

    void add_credits() const;

    //void show_users(std::string opt) const;  //"l" for showing only logged users
    static bool check_user(int u_i);
};

class HorsesManagement : public PrivilegeGroup {
protected:
    HorsesManagement(ClientContainer &);

    static int get_horse_id(std::string h_name);
public:

    void add_horse() const;

    void add_horse(double speed, std::string h_name) const;
    static bool check_horse(int h_i);

    static std::string
    show_horses(unsigned int n); //show horses and their ranks. show only the best n horses. n==0 to show all
};

class RacesManagement : public PrivilegeGroup {
    std::set<int> check_all_horses_available(int r_i) const;

    void add_race(int laps) const;

    void add_to_race(int race, std::set<int> horses) const;

    void start_race(int r_i) const;

    static void show_races(ClientContainer &clientContainer, bool activeOnly);

    static void show_race_info(ClientContainer &clientContainer, int r_i);

    bool get_race_started(int r_i) const;

    int get_race_laps(int r_i) const;

    static bool check_horse_in_race(int h_i, int r_i);

    bool check_horse_available(int h_i) const;

    std::string get_race_date(int r_i) const;

    int get_num_horses_on_race(int r_i) const;

    bool check_race_started(int r_i) const;

    static void show_horses_on_race(ClientContainer &clientContainer, int r_i);

protected:
    RacesManagement(ClientContainer &);

public:
    void add_race() const;

    void add_to_race() const;

    void start_race() const;

    static void showAllRaces(ClientContainer &clientContainer);

    static void showActiveRaces(ClientContainer &clientContainer);

    static void showRaceInfo(ClientContainer &clientContainer);

    static void showHorsesOnRace(ClientContainer &clientContainer);

    static bool check_race(int r_i); //checks if a race exists
};

class OtherAdmin : public PrivilegeGroup {
protected:
    OtherAdmin(ClientContainer &);

public:
    void run_sql() const;

    void startServer();

    void shutdownServer();
};

//class BetsManagement : public PrivilegeGroup {
//protected:
//    BetsManagement(ClientContainer&);
//
//    std::string getGroupHelp() const;
//public:
//
//    void bet(int u_i,int h_i,int r_i, double cr) const; //horse_id,race_id,credits to bet
//
//    double check_bet(int u_i,int h_i,int r_i) const;
//    double get_user_credits(int u_i) const;
//    void show_user_bets(int u_i,int lim) const;
//};
//
//CLIENTS

class CommandSet {
    PrivilegeGroup *group;
    std::vector<Command *> commands;
public:
    CommandSet(PrivilegeGroup *group, const std::vector<Command *> &commands) : group(group), commands(
            std::vector<Command *>(commands)) {}

    PrivilegeGroup *getGroup() const { return group; }

    std::vector<Command *> getCommands() const { return commands; }
};

class SuperClient {
protected:
    std::vector<CommandSet> commandSet;
public:
    std::vector<CommandSet> &getCommandSets();
};

class LoggedOutClient : public SuperClient, public BasicU, public LoggedOut {
    void helloImAnAbstractClass() {}

public:
    LoggedOutClient(ClientContainer &clientContainer);
};

class RegularClient : public SuperClient, public BasicU, public LoggedIn, public SelfManagement {
    void helloImAnAbstractClass() {}

public:
    RegularClient(ClientContainer &clientContainer);
};

class AdminClient
        : public SuperClient,
          public BasicU,
          public LoggedIn,
          public SelfManagement,
          public UsersManagement,
          public HorsesManagement,
          public RacesManagement/*, public BetsManagement*/ {
    void helloImAnAbstractClass() {}

public:
    AdminClient(ClientContainer &clientContainer);
};


class ClientContainer : public Connection {
    SuperClient *currentUser;

    //Category,Command*
    //BasicUser* currentUser = NULL;
    ClientContainer(const ClientContainer &);

    void operator=(ClientContainer const &);

public:
    int user_id;
    std::string username, name;

    ClientContainer(int cl_so);

    SuperClient *const getCurrentUser();

    void changeUserType(Constants::ClientType type);

    bool isInServer() const;

    std::vector<CommandSet> &getCommandSets();

    ~ClientContainer();
};

//DEPRECATED CLIENT

class Client {

public:
    void static initPreparedStatements();
    int client_socket;

  	int user_id;
  	std::string username, name;

    Client(int=Constants::IN_SERVER);
    ~Client();

    bool parse( std::string ins );

  //private:
    //session management
    void regist(std::string login_name , std::string pass , std::string name , bool ad=false , int cr=Constants::DEFAULT_CREDITS);  //registers a user
    void login(std::string login_name, std::string pass);
    void logout();
    void help();
    void clear();
    bool quit();

    parsed_command get_parsed_command(std::string input);

    //user management
    int static get_user_id(std::string login_name);
    void passwd(int id, std::string old_pass, std::string new_pass);
    void passwd(std::string login_name, std::string old_pass, std::string new_pass)
    {
      passwd(get_user_id(login_name),old_pass,new_pass);
    }
    static bool is_admin(int id);
    bool is_admin()
    {
      return is_admin(user_id);
    }
    static bool is_admin(std::string login_name)
    {
      return is_admin(get_user_id(login_name));
    }
    void change_admin(int id, bool ad);
    void change_admin(std::string login_name, bool ad)
    {
      change_admin(get_user_id(login_name),ad);
    }
    void add_credits(int u_i, double cr);

    //data management
    void add_horse(double speed , std::string h_name); //name space contain spaces
    void add_race(int laps);
    void add_to_race(int race,std::vector<int> horses);
    void start_race(int r_i);
    void bet(int u_i,int h_i,int r_i, double cr); //horse_id,race_id,credits to bet

    //get data
    //void show_clients(int flag);  //ALL for all sockets, LOGGED_IN for logged in sockets, LOGGED_OFF for logged off sockets
    void show_races(std::string flag); //ALL, ACTIVE
    void show_race_info(int r_i);
    bool get_race_started(int r_i);
    int get_race_laps(int r_i);
    std::string get_horse_name(int horse);
    int get_horse_id(std::string h_name);
    bool check_race(int r_i); //checks if a race exists
    bool check_horse(int h_i);
    bool check_user(int u_i);
    bool check_horse_available(int h_i);
    int check_all_horses_available(int r_i);    //returns the horse_id of the 1st horse that is not available OR returns <=0 if all are available
    
    double check_bet(int u_i,int h_i,int r_i);
    void watch_race(int r_i);
    double get_user_credits(int u_i);
    std::string get_race_date(int r_i);
    int get_num_horses_on_race(int r_i);
    bool check_race_started(int r_i);
    void show_horses(int n=0); //show horses and their ranks. show only the best n horses. n<=0 to show all
    void show_user_bets(int u_i,int lim);
    void show_users(std::string opt);  //"l" for showing only logged users
    void show_horses_on_race(int r_i);
    void sql_query(std::string q);

    //miscellaneous
    void static writeline(int socketfd, std::string line, bool paragraph = true);

    void writeline(std::string line, bool paragraph = true) const;
};

class arg  //funciona
{
    boost::any data;

  public:
    //constructors
    arg();
    arg(boost::any);
    arg(const arg&);

    template<typename T>
    arg(const T &);

    /*bool operator=(const boost::any& st) {
      return this->set(st);
    }*/

    bool set(const boost::any& st);
    //public functions
    template <typename T> T get() const;
    bool isEmpty() const;
    bool isOfType(const std::type_info&) const;
};

class Command {
    friend class ClientContainer;

    std::string name;
    std::string description;
    std::vector<std::string> notes;
    std::function<void()> _run;

public:
    Command(const std::string &name, const std::string &description, const std::vector<std::string> &notes,
            std::function<void()>);

    std::string getHelp() const;

    std::string getName() const;

    void run() const;
};

class ClientError : public std::exception {
    const std::string whatMessage;
public:
    ClientError(const std::string & = Constants::defaultClientErrorWhatMessage);

    const char *what() const throw();

    static void throwIf(bool, const std::string & = Constants::defaultClientErrorWhatMessage);

    void show(const Connection &conn) const;
};

class ClientMessage : public std::exception {
    const std::string whatMessage;
public:
    ClientMessage(const std::string &);

    const char *what() const throw();

    void show(const Connection &conn) const;
};

class QuitClient {

};

bool check_horse_in_race(int h_i, int r_i);

#endif