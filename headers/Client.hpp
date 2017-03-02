#ifndef CLIENT_HPP
#define CLIENT_HPP 1

#include "../headers/Sockets.hpp"
#include "../headers/DBMS.hpp"
#include "../headers/Race.hpp"
#include <vector>
//#include <boost/variant.hpp>
//using namespace boost;

#define EXECUTE res=SQL_server_class::server().executeSQL(query.str());

typedef struct tm TM;

#define DEFAULT_CREDITS 0
#define LOGGED_OFF (-5)
#define IN_SERVER (-6)

typedef enum {_int_,_bool_,_string_,_double_,_v_int_} arg_types;

class arg  //funciona
{
  public:
    //public variables
    const arg_types arg_type; //int=0 bool=1 string=2 double=3
  
  private:
    //private variables
    int i;
    bool b;
    string s;
    double d;
    vector<int> v;   

  public:
    //constructors
    arg(int in)          : arg_type(_int_)     , i(in) {}
    arg(bool bo)         : arg_type(_bool_)    , b(bo) {}
    arg(string st)       : arg_type(_string_)  , s(st) {}
    arg(double db)       : arg_type(_double_)  , d(db) {}
    arg(vector<int> vi)  : arg_type(_v_int_)   , v(vi) {}

    bool operator=(string st)
    {
      return this->set_string(st);
    }
    bool operator=(bool bo)
    {
      return this->set_bool(bo);
    }
    bool operator=(int in)
    {
      return this->set_int(in);
    }
    bool operator=(double db)
    {
      return this->set_double(db);
    }
    bool operator=(vector<int> vi)
    {
      return this->set_v_int(vi);
    }

    bool set_int(int in);
    bool set_bool(bool bo);
    bool set_string(string st);
    bool set_double(double db);
    bool set_v_int(vector<int> vi);
    //public functions
    int arg_int();
    string arg_string();
    bool arg_bool();
    double arg_double();
    vector<int> arg_v_int();
};

typedef struct
{
  string cmd;
  vector<arg> args;
}parsed_command;

class Client
{
  	//friend class Network;

  public:
    int client_socket;

  //private:
  	int user_id;
  	string username, name;

  public:

    Client(int=IN_SERVER);
    ~Client();

    bool parse( string ins );

  //private:
    //session management
    void regist(string login_name , string pass , string name , bool ad=false , int cr=DEFAULT_CREDITS);  //registers a user
    void login(string login_name,string pass);
    void logout();
    void help();
    void clear();
    bool quit();

    parsed_command get_parsed_command(string input);

    //user management
    int get_user_id(string login_name);
    void passwd(int id,string old_pass, string new_pass);
    void passwd(string login_name,string old_pass, string new_pass)
    {
      passwd(get_user_id(login_name),old_pass,new_pass);
    }
    bool is_admin(int id);
    bool is_admin()
    {
      return is_admin(user_id);
    }
    bool is_admin(string login_name)
    {
      return is_admin(get_user_id(login_name));
    }
    void change_admin(int id, bool ad);
    void change_admin(string login_name, bool ad)
    {
      change_admin(get_user_id(login_name),ad);
    }

    //data management
    void add_horse(double speed , string h_name); //name space contain spaces
    void add_race(int laps);
    void add_to_race(int race,vector<int> horses);
    void add_credits(int u_i, double cr);
    void bet(int u_i,int h_i,int r_i, double cr); //horse_id,race_id,credits to bet
    //void remove_horse_from_race(int horse, int race);
    void start_race(int r_i);

    //get data
    //void show_clients(int flag);  //ALL for all sockets, LOGGED_IN for logged in sockets, LOGGED_OFF for logged off sockets
    void show_races(string flag); //ALL, ACTIVE
    void show_race_info(int r_i);
    bool get_race_started(int r_i);
    int get_race_laps(int r_i);
    string get_horse_name(int horse);
    int get_horse_id(string h_name);
    bool check_race(int r_i); //checks if a race exists
    bool check_horse(int h_i);
    bool check_user(int u_i);
    bool check_horse_available(int h_i);
    int check_all_horses_available(int r_i);    //returns the horse_id of the 1st horse that is not available OR returns <=0 if all are available
    
    double check_bet(int u_i,int h_i,int r_i);
    void watch_race(int r_i);
    double get_user_credits(int u_i);
    string get_race_date(int r_i);
    int get_num_horses_on_race(int r_i);
    bool check_race_started(int r_i);
    void show_horses(int n=0); //show horses and their ranks. show only the best n horses. n<=0 to show all
    void show_user_bets(int u_i,int lim);
    void show_users(string opt);  //"l" for showing only logged users
    void show_horses_on_race(int r_i);
    void sql_query(string q);

    //miscellaneous
    void writeline(int socketfd, string line,bool paragraph=true);
};

bool check_horse_in_race(int h_i, int r_i);

#endif