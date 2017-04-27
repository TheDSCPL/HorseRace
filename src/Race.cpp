#include "../headers/Race.hpp"
#include "../headers/Sockets.hpp"
#include "../headers/DBMS.hpp"
#include "../headers/Log.hpp"
#include "../headers/Client.hpp"
#include "../headers/Utils.hpp"

#define EXECUTE res=SQLServer::server().executeSQL(query.str());

map<int,class Race*> races;

void Race::writeline(int so, string s,int column,int line)
{ // Envia uma string para um socket
	if(Network::server().clients.find(so)==Network::server().clients.end())
		return;
	if(column>=0&&line>=0)
		s="\u001B[s\u001B[" + to_string(line) + ";" + to_string(column) + "H"  + s + "\u001B[u";
	if(so<0)
		{LOCK; cerr << s ;UNLOCK;}
	else
		write(so, s.c_str(), s.length());
}

void Race::count_down(int secs)
{
	if(secs<0)
		return;
	time_t _ti,_t;
	time(&_ti);
	int temp,prev=-1;
	do
	{
		time(&_t);
		temp=secs-(_t-_ti);
		if(temp!=prev)
		{
			print_title();
			print_update_places();
			print_tracks();
			prev=temp;
			if(temp==2) broadcast(BLUE_BG + "COUNT DOWN:" + RESET_COLORS + "----" + RED_BG + BLACK_LT + "READY!" + RESET_COLORS + "-" + RED_BG + "  "  + RESET_COLORS + "-__-__ " , COUNT_DOWN_POSITION);
			else if(temp==1) broadcast(BLUE_BG + "COUNT DOWN:" + RESET_COLORS + "----" + YELLOW_BG + BLACK_LT + "SET!" + RESET_COLORS + "---" + RED_BG + "  " + RESET_COLORS + "-" + YELLOW_BG + "  " + RESET_COLORS + "-__ " , COUNT_DOWN_POSITION);
			else if(temp==0) broadcast(BLUE_BG + "COUNT DOWN:" + RESET_COLORS + "----" + GREEN_BG + BLACK_LT + "GO!" + RESET_COLORS + "----" + RED_BG + "  " + RESET_COLORS + "-" + YELLOW_BG + "  " + RESET_COLORS + "-" + GREEN_BG + "  " + RESET_COLORS + " " , COUNT_DOWN_POSITION);
			else broadcast(BLUE_BG + "COUNT DOWN:" + RESET_COLORS + "----" + RED_BG + BLACK_LT + to_string(temp) + RESET_COLORS + "------" + RED_BG + "  "  + RESET_COLORS + "-__-__ " , COUNT_DOWN_POSITION);
		}
	}while(temp>0);
}

Race::Race(int r_i , int l) : race_id(r_i) , laps(l) , bets(get_race_bets(r_i)), next_place(-1)//, bets(0)
{
	horse_info temp;
	temp.place=-1;
	temp.position=0;
	temp.running=true;

	stringstream query;
	PGresult* res;

	query << "UPDATE races SET started=" << Utils::b2s(true) << " WHERE race_id=" << race_id;

	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		broadcast(RESET_ATTR + RED_BG);
		clear();
		broadcast(RESET_ATTR + BOLD + "FATAL ERROR OCCURRED. PLEASE CONTACT THE ADMIN." + RESET_ATTR,17,10);
		broadcast(RESET_ATTR);
	}
	PQclear(res);
	query.str("");query.clear();

	query << "SELECT horse_id,name,speed,state" << endl;
	query << "FROM are_on JOIN horses USING (horse_id)" << endl;
	query << "WHERE race_id=" << r_i << endl;
	query << "ORDER BY horse_id;" << endl;

	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		broadcast(RESET_ATTR + RED_BG);
		clear();
		broadcast(RESET_ATTR + BOLD + "FATAL ERROR OCCURRED. PLEASE CONTACT THE ADMIN." + RESET_ATTR,17,10);
		broadcast(RESET_ATTR);
	}

	int h_i;
	for(int i=0;i<PQntuples(res);i++)
	{
		temp.speed=stoi(PQgetvalue(res,i,2));
		temp.state=stod(PQgetvalue(res,i,3));
		temp.name=PQgetvalue(res,i,1);
		h_i=stoi(PQgetvalue(res,i,0));
		temp.bets=get_horse_bets(h_i,r_i);
		cerr << "h_i: " << h_i << " | speed: " << temp.speed << " | bets: " << temp.bets << " | state: " << temp.state << " | name: " << temp.name << endl;
		horses.insert(pair<int,horse_info>{h_i,temp});
	}
	PQclear(res);
	clog("New race started. Race_id=" + to_string(race_id) + " with " + to_string(laps) + " laps.");
	if(pthread_create(&race_t,NULL,race_routine_redirect,(void*)this))
	{
		cerr << "Error while starting race " << r_i << "'s thread!" << endl;
		clog( "FATAL ERROR: Race::Race() -> Couldn't start a race thread for race " << r_i << ". Program will now halt!");
		Network::server().broadcast(-1,"FATAL ERROR ON RACE!");
		exit(-1);
	}
}

Race::~Race()
{
	//cerr << "destructor" << endl;
}

void Race::broadcast(string s,int column,int line)
{
	for(auto &it:sockets_watching)
	{
		writeline(it,s,column,line);
	}
}

void Race::clear()
{
	broadcast("\u001B[2J\u001B[H");
}

double Race::mod (double a,double b)
{
	return (a/b-(int)(a/b))*b;
}

void Race::print_tracks()
{
	int i=FIRST_TRACK_LINE;
	for(auto &it : horses)
	{
		broadcast("|",FIRST_TRACK_COLUMN,i);
		for(int j=1;j<=LAP_SCREEN_LENGTH;j++) broadcast("_",FIRST_TRACK_COLUMN+j,i);

		int temp= mod( it.second.position , LAP_REAL_LENGTH+1 ) / LAP_REAL_LENGTH*LAP_SCREEN_LENGTH;
		broadcast("H",FIRST_TRACK_COLUMN+1+temp,i);

		broadcast("| - " + it.second.name,FIRST_TRACK_COLUMN+LAP_SCREEN_LENGTH+1,i);
		i++;
	}
}

bool comp(horse_info a, horse_info b)
{
	if(a.place>0)	//a finished
	{
		if(b.place>0)	//both a and b finished
			return a.place<b.place;
		return true;	//a finished but b didn't
	}
	else	//a didn't finish
	{
		if(b.place>0)	//a didn't finish but b did
			return false;
		return a.position>b.position;	//both didn't finish
	}
}

void Race::print_update_places()
{
	vector<horse_info> h_temp;
	for(auto& it : horses)
		h_temp.push_back(it.second);

	sort(h_temp.begin(),h_temp.end(),comp);

	broadcast(BOLD + "    LEADERBOARD" + RESET_ATTR,WINDOW_WIDTH - LEADERBOARD_WIDTH,FIRST_TRACK_LINE-1);
	for(int i=0;i<LEADERBOARD_WIDTH;i++) broadcast("-",WINDOW_WIDTH - LEADERBOARD_WIDTH + i,FIRST_TRACK_LINE);
	int i=1;
	string spaces;
	for(auto &it:h_temp)
	{
		spaces="";
		for(unsigned int j=0;j<15-it.name.length();j++) spaces+=" ";
		if(i<=3)
			broadcast(BOLD + to_string(i) + ".  " + RESET_ATTR + it.name + spaces,WINDOW_WIDTH - LEADERBOARD_WIDTH,FIRST_TRACK_LINE+i);
		else if(i<10)
			broadcast(RESET_ATTR + to_string(i) + ".  " + it.name + spaces,WINDOW_WIDTH - LEADERBOARD_WIDTH,FIRST_TRACK_LINE+i);
		else
			broadcast(RESET_ATTR + to_string(i) + ". " + it.name + spaces,WINDOW_WIDTH - LEADERBOARD_WIDTH,FIRST_TRACK_LINE+i);
		i++;
	}
}

void Race::print_title()
{
	if(laps==1)
		broadcast( BOLD + "Race n. " + to_string(race_id) + " (1 lap)" + RESET_ATTR,TITLE_POSITION);
	else
		broadcast( BOLD + "Race n. " + to_string(race_id) + " (" + to_string(laps) + " laps)" + RESET_ATTR,TITLE_POSITION);
}

bool Race::every_horse_stopped()
{
	for(auto &it:horses)
		if(it.second.running)
			return false;
	return true;
}

bool check_user(int u_i)
{
	if(u_i<=0)
		return false;
	stringstream query;
	PGresult* res;
	query << "SELECT user_id" << endl;
	query << "FROM users" << endl;
	query << "WHERE user_id=" << u_i;
	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::check_user\" using user_id=" << u_i << ". Query error:n" << e.what());
		//PQclear(res);
		//throw (int)3;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;	
}

bool check_race(int r_i)
{
	if(r_i<=0)
		return false;
	stringstream query;
	PGresult* res;
	query << "SELECT race_id" << endl;
	query << "FROM races" << endl;
	query << "WHERE race_id=" << r_i;
	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::check_race\" using race_id=" << r_i << ". Query error:n" << e.what());
		//PQclear(res);
		//throw (int)2;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;	
}

bool check_horse(int h_i)
{
	if(h_i<=0)
		return false;
	stringstream query;
	PGresult* res;
	query << "SELECT horse_id" << endl;
	query << "FROM horses" << endl;
	query << "WHERE horse_id=" << h_i;
	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"check_race\" using race_id=" << h_i << ". Query error:n" << e.what());
		//PQclear(res);
		//throw (int)2;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;	
}

double Race::get_horse_bets(int h_i,int r_i)
{
	if(!check_horse(h_i))
		return 0.0;
	if(!check_race(r_i))
		return 0.0;
	stringstream query;
	PGresult* res;
	query << "SELECT SUM(bet)" << endl;
	query << "FROM bets" << endl;
	query << "WHERE race_id=" << r_i << " AND horse_id=" << h_i << endl;
	query << "GROUP BY race_id;" << endl;
	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		broadcast(RESET_ATTR + RED_BG);
		clear();
		broadcast(RESET_ATTR + BOLD + "SQL ERROR OCCURRED. PLEASE CONTACT THE ADMIN." + RESET_ATTR,17,10);
		broadcast(RESET_ATTR);
		//throw(e);
	}
	if(PQntuples(res)==0)
		return 0.0;
	double ret=stod(PQgetvalue(res,0,0));
	PQclear(res);
	return ret;
}

double Race::get_race_bets(int r_i)
{
	if(!check_race(r_i))
		return 0.0;
	stringstream query;
	PGresult* res;
	query << "SELECT SUM(bet)" << endl;
	query << "FROM bets" << endl;
	query << "WHERE race_id=" << r_i << endl;
	query << "GROUP BY race_id;" << endl;
	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		broadcast(RESET_ATTR + RED_BG);
		clear();
		broadcast(RESET_ATTR + BOLD + "SQL ERROR OCCURRED. PLEASE CONTACT THE ADMIN." + RESET_ATTR,17,10);
		broadcast(RESET_ATTR);
		//throw(e);
	}
	if(PQntuples(res)==0)
		return 0.0;
	double ret=stod(PQgetvalue(res,0,0));
	PQclear(res);
	return ret;
}

typedef struct
{
	int user_id;
	double ammount;
}bet;

double get_user_credits(int u_i)
{
	if(!check_user(u_i))
		return 0.0;
	stringstream query;
	PGresult* res;
	query << "SELECT credits" << endl;
	query << "FROM users" << endl;
	query << "WHERE user_id=" << u_i;
	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"get_user_credits\" using user_id=" << u_i << ". Query error: " << e.what());
		//PQclear(res);
		throw (int)5;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return -1;
	}
	double cr=stod(PQgetvalue(res,0,0));
	PQclear(res);
	return cr;
}

void Race::add_credits(int u_i, double cr)
{
	if(cr==0)
		return;
	if(get_user_credits(u_i)+cr<0)
	{
		return;
	}
	stringstream query;
	PGresult* res;
	query << "BEGIN;";
	query << "UPDATE users" << endl;
	query << "SET credits = credits + " << cr << endl;
	query << "WHERE user_id=" << u_i << ";";
	query << "COMMIT;";
	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		broadcast("Error while running your command. If the problem persists, please contact the admin.");
		//throw(e);
	}
	PQclear(res);
}

void Race::add_balance(int r_i,int u_i, int h_i, double cr)
{
	if(cr==0)
		return;
	stringstream query;
	PGresult* res;
	query << "BEGIN;";
	query << "UPDATE bets" << endl;
	query << "SET balance = balance + (" << cr << ")" << endl;
	query << "WHERE user_id=" << u_i << " AND race_id=" << r_i << " AND horse_id=" << h_i << ";";
	query << "COMMIT;";
	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		broadcast("Error while running your command. If the problem persists, please contact the admin.");
		//throw(e);
	}
	PQclear(res);
}

void Race::race_routine()
{
	clear();
	print_title();
	print_update_places();
	print_tracks();
	count_down(RACE_DELAY);

	//------------------compute the race-----------------//
	while(!every_horse_stopped())
	{
		for(auto &it:horses)
		{
			if(!it.second.running)
				continue;
			double r=(double)rand()/(double)RAND_MAX*MAX_DEVIATION;
			r+=it.second.state;
			r=r<0?0:r;	//para não andar para trás
			r*=it.second.speed;	//velocidade média num tempo de amostragem TS
			r*=TS;	//integração do valor da velocidade;
			it.second.position+=r;
			if(it.second.position>=laps*LAP_REAL_LENGTH)
			{
				it.second.place=next_place++;
				it.second.position=laps*LAP_REAL_LENGTH;
				it.second.running=false;
			}
		}
		count_down(0);	//para mostrar o semáforo a quem chegou a meio da corrida
		print_title();
		print_update_places();
		print_tracks();
		usleep(TS*1000000);
	}

	finished=true;

	clog("Race " + to_string(race_id) + " has finished.");

	//------------------after race finished--------------------//

	stringstream query;
	PGresult* res;
		
	for(auto &it:horses)	//set places
	{
		if(!check_horse_in_race(it.first,race_id))
			continue;
		query.str("");query.clear();
		query << "BEGIN;";
		query << "UPDATE are_on" << endl;
		query << "SET place = " << it.second.place << endl;
		query << "WHERE race_id=" << race_id << " AND horse_id=" << it.first << ";";
		query << "COMMIT;";
		try
		{
			EXECUTE;
		}
		catch(DBMSError& e)
		{
			broadcast(RESET_ATTR + RED_BG);
			clear();
			broadcast(RESET_ATTR + BOLD + "SQL ERROR OCCURRED. PLEASE CONTACT THE ADMIN." + RESET_ATTR,17,10);
			broadcast(RESET_ATTR);
			//throw(e);
		}
		PQclear(res);
	}

	clog("All places set on the DB (Race " + to_string(race_id) + ").");

	//----------------------DISTRIBUTE BETS---------------------//
	map<int,vector<bet>> bets_temp;	//horse_id, bet

	query << "SELECT user_id,horse_id,bet" << endl;
	query << "FROM bets" << endl;
	query << "WHERE race_id=" << race_id << endl;
	query << "ORDER BY bet_id;" << endl;

	try
	{
		EXECUTE;
	}
	catch(DBMSError& e)
	{
		broadcast(RESET_ATTR + RED_BG);
		clear();
		broadcast(RESET_ATTR + BOLD + "SQL ERROR OCCURRED. PLEASE CONTACT THE ADMIN." + RESET_ATTR,17,10);
		broadcast(RESET_ATTR);
		//throw(e);
	}

	int n=PQntuples(res);
	int h_i;
	bet temp;

	//cerr << "aqui" << endl;

	auto it =bets_temp.begin();
	for(int i=0;i<n;i++)
	{
		h_i=stoi(PQgetvalue(res,i,1));
		//cerr << PQntuples(res) << endl;
		it=bets_temp.find(h_i);
		temp.user_id=stoi((PQgetvalue(res,i,0)));
		temp.ammount=stod(PQgetvalue(res,i,2));
		//temp.balance=-temp.ammount;
		//cerr << "user_id=" << temp.user_id << " | ammount=" << temp.ammount << " | "
		if(it==bets_temp.end())
		{
			bets_temp.insert(pair<int,vector<bet>>( h_i , vector<bet> {temp}));
			//cerr << h_i << " "  << bets_temp.find(h_i)->second[0].user_id << " "  << bets_temp.find(h_i)->second[0].ammount << endl;
		}
		else
		{
			it->second.push_back(temp);
			//cerr << "add element vec " << " " << h_i << " "  << bets_temp.find(h_i)->second[1].user_id << " " << bets_temp.find(h_i)->second[1].ammount << endl;
		}
	}

	int p;
	double total_h;
	double total_r=bets;
	double delta;
	for(auto &i:bets_temp)	//cavalo a cavalo
	{
		h_i=i.first;
		p=horses.find(h_i)->second.place;
		total_h=horses.find(h_i)->second.bets;

		if(total_h==0)
			continue;
		for(auto &j:i.second)	//aposta a aposta
		{
			delta=0;
			if(p==1)
			{
				delta=total_r*(1-FEE);		//dinheiro para distribuir depois de se tirar os "impostos"
				delta*=1.0/2.0*12.0/13.0;			//dinheiro para este cavalo
				delta*=j.ammount/total_h;	//regra de 3 simples para distribuir dinheiro entre apostadores no mesmo cavalo.
				add_credits(j.user_id,					delta);
				add_balance(race_id,j.user_id,h_i,		delta);
			}
			if(p==2)
			{
				delta=total_r*(1-FEE);		//dinheiro para distribuir depois de se tirar os "impostos"
				delta*=1.0/3.0*12.0/13.0;			//dinheiro para este cavalo
				delta*=j.ammount/total_h;	//regra de 3 simples para distribuir dinheiro entre apostadores no mesmo cavalo.
				add_credits(j.user_id,					delta);
				add_balance(race_id,j.user_id,h_i,		delta);
			}
			if(p==3)
			{
				delta=total_r*(1-FEE);		//dinheiro para distribuir depois de se tirar os "impostos"
				delta*=1.0/4.0*12.0/13.0;			//dinheiro para este cavalo
				delta*=j.ammount/total_h;	//regra de 3 simples para distribuir dinheiro entre apostadores no mesmo cavalo.
				add_credits(j.user_id,					delta);
				add_balance(race_id,j.user_id,h_i,		delta);
			}
			//cerr << "p: " << p << " | user_id:" << j.user_id << " | delta:" << delta << endl;
		}
	}

	clog("Bets winnings distributed successfuly (race " + to_string(race_id) + ").");

	//---------------------BLINK "RACE ENDED!!!"-------------------//

	string finished_race=RED_BG + BOLD + "RACE ENDED!!!" + RESET_ATTR;
	string clear_s;
	for(unsigned int j=0;j<finished_race.length();j++) clear_s+=" ";
	for(int i=0;i<20;i++)
	{
		count_down(0);	//para mostrar o semáforo a quem chegou a meio da corrida
		print_title();
		print_update_places();
		print_tracks();

		broadcast(finished_race,30,FIRST_TRACK_LINE+MAX_HORSES_ON_RACE);
		usleep(1*1000000);


		count_down(0);	//para mostrar o semáforo a quem chegou a meio da corrida
		print_title();
		print_update_places();
		print_tracks();

		broadcast(clear_s,30,FIRST_TRACK_LINE+MAX_HORSES_ON_RACE);
		usleep(1*1000000);
	}
	clog("Race " + to_string(race_id) + "'s thread terminaded.");
	races.erase(race_id);
	delete this;
}