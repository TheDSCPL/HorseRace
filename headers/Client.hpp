#ifndef CLIENT_HPP
#define CLIENT_HPP 1
#include <vector>
#include <typeinfo>
#include <boost/any.hpp>
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

class Client {
    const static std::string getUserId;
    const static std::string checkUserAndPass;
    const static std::string changePassword;
    const static std::string changeAdmin;
    const static std::string getHorseId;
    const static std::string insertHorse;
    const static std::string insertHorseInRace;
    const static std::string insertRace;
    const static std::string getLatestRaceId;
    const static std::string checkRaceExists;
    const static std::string checkHorseExists;
    const static std::string checkUserExists;
    const static std::string getUserCredits;
    const static std::string getRaceDate;
    const static std::string getNumHorsesOnRace;
    const static std::string getHorsesOnRace;
    const static std::string checkBetExists;
    const static std::string addCredits;
    const static std::string checkRaceStarted;
    const static std::string removeBet;
    const static std::string changeBet;

    void static initPreparedStatements();
public:
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
    bool is_admin(std::string login_name)
    {
      return is_admin(get_user_id(login_name));
    }
    void change_admin(int id, bool ad);
    void change_admin(std::string login_name, bool ad)
    {
      change_admin(get_user_id(login_name),ad);
    }

    //data management
    void add_horse(double speed , std::string h_name); //name space contain spaces
    void add_race(int laps);
    void add_to_race(int race,std::vector<int> horses);
    void add_credits(int u_i, double cr);
    void bet(int u_i,int h_i,int r_i, double cr); //horse_id,race_id,credits to bet
    //void remove_horse_from_race(int horse, int race);
    void start_race(int r_i);

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
    void writeline(int socketfd, std::string line,bool paragraph=true);
};

class arg  //funciona
{
    boost::any data;

  public:
    //constructors
    arg();
    arg(boost::any);
    arg(const arg&);

    bool operator=(const boost::any& st) {
      return this->set(st);
    }

    bool set(const boost::any& st);
    //public functions
    template <typename T> T get() const;
    bool isEmpty() const;
    bool isOfType(const std::type_info&) const;
};

bool check_horse_in_race(int h_i, int r_i);

#endif