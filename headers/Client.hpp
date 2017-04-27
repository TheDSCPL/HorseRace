#ifndef CLIENT_HPP
#define CLIENT_HPP 1
#include <vector>
#include <typeinfo>
#include <boost/any.hpp>
#include <set>
#include "../headers/Constants.hpp"
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

class IAmAbstract {
protected:
    virtual void helloImAnAbstractClass() = 0;
};

class PrivilegeGroup : IAmAbstract {
protected:
    PrivilegeGroup(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    virtual std::string getGroupHelp() const = 0;
    void writeline(int socketfd, std::string line,bool paragraph=true) const;
    //virtual someType getCommands const = 0;
};

class BasicUser : public PrivilegeGroup {
protected:
    BasicUser(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    std::string getGroupHelp() const;
public:
    static void clear(int sid);
    void clear() const;
    bool quit() const;
    virtual void help() const = 0;
};

class LoggedOut : protected PrivilegeGroup{
protected:
    LoggedOut(int client_socket);

    std::string getGroupHelp() const;
public:

    void regist(std::string login_name, std::string pass, std::string name, bool ad = false,
                int cr = Constants::DEFAULT_CREDITS) const;

    void login(std::string login_name, std::string pass);
};

class LoggedIn : protected PrivilegeGroup {
protected:
    LoggedIn(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    std::string getGroupHelp() const;
public:

    void watch_race(int r_i);

    void logout();
};

class SelfManagement : protected PrivilegeGroup {
protected:
    SelfManagement(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    std::string getGroupHelp() const;
public:

    void passwd(std::string old_pass, std::string new_pass) const;
};

class UsersManagement : protected PrivilegeGroup {
protected:
    UsersManagement(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    std::string getGroupHelp() const;
public:

    int static get_user_id(std::string login_name);
    void passwd(int id, std::string old_pass, std::string new_pass) const;
    void passwd(std::string login_name, std::string old_pass, std::string new_pass) const
    {
        passwd(get_user_id(login_name),old_pass,new_pass);
    }
    static bool is_admin(int id);
    static bool is_admin(std::string login_name)
    {
        return is_admin(get_user_id(login_name));
    }
    void change_admin(int id, bool ad) const;
    void change_admin(std::string login_name, bool ad) const
    {
        change_admin(get_user_id(login_name),ad);
    }
    void add_credits(int u_i, double cr) const;
    void show_users(std::string opt) const;  //"l" for showing only logged users
    bool check_user(int u_i) const;
};

class HorsesManagement : protected PrivilegeGroup {
protected:
    HorsesManagement(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    std::string getGroupHelp() const;
public:

    void add_horse(double speed , std::string h_name) const; //name space contain spaces
    std::string get_horse_name(int horse) const;
    int get_horse_id(std::string h_name) const;
    bool check_horse(int h_i) const;
    bool check_horse_available(int h_i) const;
    std::set<int> check_all_horses_available(int r_i) const;    //returns the horse_id's of the horses that are not available
    void show_horses(int n=0) const; //show horses and their ranks. show only the best n horses. n<=0 to show all
};

class RacesManagement : protected PrivilegeGroup {
protected:
    RacesManagement(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    std::string getGroupHelp() const;
public:

    void add_race(int laps) const;
    void add_to_race(int race,std::vector<int> horses) const;
    void start_race(int r_i) const;

    void show_races(std::string flag) const; //ALL, ACTIVE
    void show_race_info(int r_i) const;
    bool get_race_started(int r_i) const;
    int get_race_laps(int r_i) const;
    static bool check_race(int r_i) const; //checks if a race exists
    std::string get_race_date(int r_i) const;
    int get_num_horses_on_race(int r_i) const;
    bool check_race_started(int r_i) const;
    void show_horses_on_race(int r_i) const;
};

class BetsManagement : protected PrivilegeGroup {
protected:
    BetsManagement(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    std::string getGroupHelp() const;
public:

    void bet(int u_i,int h_i,int r_i, double cr) const; //horse_id,race_id,credits to bet

    double check_bet(int u_i,int h_i,int r_i) const;
    double get_user_credits(int u_i) const;
    void show_user_bets(int u_i,int lim) const;
};

class LoggedOutClient : public BasicUser, public LoggedOut {
    void helloImAnAbstractClass() {}
public:
    LoggedOutClient(int client_socket);

    void help() const;
};

class RegularClient : public BasicUser, public LoggedIn, public SelfManagement {
    void helloImAnAbstractClass() {}
public:
    RegularClient(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    void help() const;
};

class AdminClient : public RegularClient, public UsersManagement, public HorsesManagement, public RacesManagement, public BetsManagement {
    void helloImAnAbstractClass() {}
public:
    AdminClient(int client_socket, int user_id = Constants::LOGGED_OFF, std::string username = "", std::string name = "");

    void help() const;
};

class ClientContainer {
    BasicUser* currentUser = NULL;
public:
    int user_id;
    const int client_socket;
    std::string username, name;

    ClientContainer(int cl_so);
    BasicUser* getCurrentUser();
    void changeUserType(int type, int c_so, int u_id);
    ~ClientContainer();
};







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
    void writeline(int socketfd, std::string line,bool paragraph=true) const;
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

class ClientError : public std::exception {
    const std::string whatMessage;
public:
    ClientError(const std::string&);
    const char* what() const throw();
};

bool check_horse_in_race(int h_i, int r_i);

#endif