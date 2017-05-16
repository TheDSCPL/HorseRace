#ifndef _RACE_HPP_
#define _RACE_HPP_ 1
#include <map>
#include <string>
#include <set>
#include "Sockets.hpp"

//#define MIN_HORSES_ON_RACE (3)
//#define MAX_HORSES_ON_RACE (10)
#define LAP_SCREEN_LENGTH (36)
#define LAP_REAL_LENGTH (80)
#define RACE_DELAY (10)
#define TS (0.1)

#define RED_BG string("\u001B[41m")
#define GREEN_BG string("\u001B[42m")
#define YELLOW_BG string("\u001B[43m")
#define BLUE_BG string("\u001B[44m")
#define BLACK_LT string("\u001B[30m")
#define RESET_COLORS string("\u001B[37;40m")
#define RESET_ATTR string("\u001B[0m")
#define BOLD string("\u001B[1m")
#define COUNT_DOWN_POSITION 2,3
#define TITLE_POSITION 32,1
#define FIRST_TRACK_LINE (5)
#define FIRST_TRACK_COLUMN (1)
#define LEADERBOARD_WIDTH (19)
#define WINDOW_WIDTH (80)
#define MAX_DEVIATION (0.6)
#define FEE (0.05)

typedef struct
{
	double speed;
	double state;
	std::string name;
	double position=0;	//physically in the race
	int place=-1;		//place in the leaderboards
	bool running=true;
	double bets=0;
}horse_info;

class Race
{
	int next_place;
	const int race_id;
	pthread_t race_t;
	const int laps;
	double bets;

	static void* race_routine_redirect(void* t)
	{
		((Race*)t)->race_routine();
		return NULL;
	}
	void race_routine();
	void broadcast(std::string s,int column=-1,int line=-1);
	void print_tracks();
	void print_title();

    void writeline(Connection *, std::string s, int column = -1, int line = -1);
	void count_down(int secs);
	void clear();
	void print_update_places();
	double mod (double a,double b);
	bool every_horse_stopped();
	double get_horse_bets(int h_i,int r_i);
	double get_race_bets(int r_i);
	void add_balance(int r_i,int u_i, int h_i, double cr);
	void add_credits(int u_i, double cr);
public:
    int getRaceId() const;

    std::set<Connection *> sockets_watching;    //socktfd of all the clients watching the race
	bool finished=false;
	std::map<int,horse_info> horses;	//horse_id,horse_info

	Race(int r_i,int l);	//race_id , laps
	~Race();
};

extern std::map<Connection *, class Race *> races;

#endif