#define CLIENT_CPP 1
#include "../headers/Client.hpp"

#include "../headers/Sockets.hpp"
#include "../headers/DBMS.hpp"
#include "../headers/Race.hpp"
#include "../headers/Log.hpp"
#include "../headers/Utils.hpp"
#include "../headers/Constants.hpp"
#include <cctype>
#include <string>

#define EXECUTE res=SQLServer::server().executeSQL(query.str());

using namespace std;
using namespace Constants;

#ifndef vcout //verbose cout.
#define vcout if(false) cout
#endif

void print_command(parsed_command temp)
{
 	cerr << endl << "command: \"" << temp.cmd << "\"" << endl << endl;
 	for(auto&& it : temp.args)
 	{
 		if(it.isOfType(typeid(int)))
 		{
 			cerr << endl << "int = " << it.get<int>() << endl;
 		}
 		if(it.isOfType(typeid(double)))
 		{
 			cerr << endl << "double = " << it.get<double>() << endl;
 		}
 		if(it.isOfType(typeid(bool)))
 		{
 			cerr << endl << "bool = " << Utils::b2s(it.get<bool>()) << endl;
 		}
 		if(it.isOfType(typeid(std::string)))
 		{
 			cerr << endl << "string = " << it.get<string>() << endl;
 		}
 		if(it.isOfType(typeid(vector<int>)))
 		{
 			vector<int> vi=it.get<vector<int>>();
 			cerr << endl << "v_int = ";
 			for(int i : vi)
 			{
 				cerr << i << " ";
 			}
 			cerr << endl; ;
 		}
 		cerr << endl;
 	}
}

/*vector<arg>& Commands::getCommands() {
	std::vector<arg> ret;
	ret.push_back
}*/

//map<string,vector<arg>> getCommands()

parsed_command Client::get_parsed_command(string input)
{
	const type_info& _string = typeid(std::string);
	const type_info& _int = typeid(int);
	const type_info& _double = typeid(double);
	const type_info& _bool = typeid(bool);
	const type_info& _v_int = typeid(vector<int>);
	const map<string,vector<arg>> commands =
	{
		{ "\\help" , {} },																								//ALTER
		{ "\\clear" , {} },																							//DONE
		{ "\\quit" , {} }/*,																							//DONE
		{ "\\login" , {(string)"" , (string)""} },	//username,pass													//DONE
		{ "\\logout" , {} },																						//DONE
		{ "\\register" , {(string)"" , (string)"" , (string)""} }, //username,pass,name (name may contain spaces)	//DONE
		{ "\\change_admin" , {(string)"" , false} },																//DONE
		{ "\\change_admin_id" , {-1 , false} },																		//DONE
		{ "\\passwd" , {(string)"" , (string)"" , (string)""} }, //old pass, new pass, new pass						//DONE
		{ "\\passwd_other" , {(string)"" , (string)"" , (string)""} }, //username, new pass, new pass				//DONE
		{ "\\passwd_other_id" , {-1 , (string)"" , (string)""} }, //user_id, new pass, new pass						//DONE
		{ "\\add_horse" , {0.0 , (string)""} },																		//DONE
		{ "\\start_race" , {-1} },																					//DONE
		{ "\\watch_race" , {-1} },																					//DONE
		{ "\\add_to_race" , {-1 , vector<int>{}} },																	//DONE
		{ "\\add_race" , {-1} },																					//DONE
		{ "\\add_credits" , {-1 , 0.0} },																			//DONE
		{ "\\bet" , {-1,-1,0.0} },	//horse_id,race_id,bet_ammount													//DONE
		{ "\\bet_other" , {-1,-1,-1,0.0} },	//user_id,horse_id,race_id,bet_ammount									//DONE
		{ "\\show_server_ip" , {} },																				//DONE
		{ "\\stop_server" , {} },																					//DONE
		{ "\\start_server" , {} },																					//DONE
		{ "\\show_users" , {(string)""} }, //"l" to show only logged users. "a" to show all users.					//DONE
		{ "\\show_all_horses" , {} },																				//DONE
		{ "\\show_some_horses" , {-1} },																			//DONE
		{ "\\show_horses_on_race" , {-1} }, //shows horses at a given race											//DONE
		{ "\\show_races" , {(string)""} },																			//DONE
		{ "\\show_race_info" , {-1} },																				//DONE
		{ "\\show_bets" , {-1} },																					//DONE
		{ "\\show_bets_other", {-1,-1} },																			//DONE -- ADD TO HELP
		{ "\\sql_query" , {(string)""} } //run a SQL query															//DONE*/
	};
	stringstream temp(input);
	size_t size=temp.str().size();
	string buffer;
	temp >> buffer;
	auto it=commands.find(buffer);
	if(it==commands.end())
	{
		writeline(client_socket,"Command \""+buffer+"\" is invalid.");
		return parsed_command{"",{}};
	}
	parsed_command command{"",(vector<arg>)it->second};

	command.cmd=buffer;
	size_t place=buffer.size()+1;
	int n=it->second.size();
	for(int i=0;i<n;i++)
	{
		if(place>=size)	//was still waiting for arguments but there are none left
		{
			writeline(client_socket,"Too few arguments inserted.");
			command.args.clear();
			command.cmd.clear();
			return command;
		}
		if(it->second[i].isOfType(typeid(std::string)))
		{
			if(i==n-1)	//quando o último argumento é uma string, põe tudo o resto no argumento string
			{
				command.args[i]=string(&temp.str()[place]);
				return command;
			}
			else
			{
				temp >> buffer;
				place+=(1+buffer.size());
				command.args[i]=buffer;
				continue;
			}
		}
		if(it->second[i].isOfType(typeid(vector<int>)))
		{
			vector<int> tt;
			for(int t;place<size;tt.push_back(t))
			{
				temp >> buffer;
				place+=(1+buffer.size());
				try
				{
					t=stoi(buffer);
				}
				catch(...)
				{
					writeline(client_socket,"Argument "+to_string(i+tt.size()+1)+" (\""+buffer+"\") is not an integer.");
					command.args.clear();
					command.cmd.clear();
					return command;
				}
			}
			command.args[i]=tt;
			return command;
		}
		temp >> buffer;
		place+=(1+buffer.size());
		if(it->second[i].isOfType(typeid(bool)))
		{
			if(!Utils::is_bool(buffer))
			{
				writeline(client_socket,"Argument "+to_string(i+1)+" (\""+buffer+"\") is not a bool.");
				command.args.clear();
				command.cmd.clear();
				return command;
			}
			command.args[i]=Utils::s2b(buffer);
			continue;
		}
		if(it->second[i].isOfType(typeid(int)))
		{
			int t;
			try
			{
				t=stoi(buffer);
			}
			catch(...)
			{
				writeline(client_socket,"Argument "+to_string(i+1)+" (\""+buffer+"\") is not an integer.");
				command.args.clear();
				command.cmd.clear();
				return command;
			}
			command.args[i]=t;
			continue;
		}
		if(it->second[i].isOfType(typeid(double)))
		{
			double db;
			try
			{
				db=stod(buffer);
			}
			catch(...)
			{
				writeline(client_socket,"Argument "+to_string(i+1)+" (\""+buffer+"\") is not a number.");
				command.args.clear();
				command.cmd.clear();
				return command;
			}
			command.args[i]=db;
			continue;
		}
	}
	//print_command(command);
	return command;
}

void Client::help()
{
	clear();
	writeline(client_socket,"\r\n\u001B[30;41m----------HELP MENU---------\u001B[0m\r\n");
	writeline(client_socket,"\u001B[30;42m->\"\\clear\"\u001B[0m - Clears the window");
	if(!(client_socket<=0&&user_id<=0)) writeline(client_socket,"\u001B[30;42m->\"\\quit\"\u001B[0m - Quits the connection to the server.");
	if(user_id<=0)
	{
		writeline(client_socket,"\u001B[30;42m->\"\\login <username> <password>\"\u001B[0m - Logs the user in.");
		if(client_socket<=0) writeline(client_socket,"                                 - Note: Only admins can login in the server.");
		writeline(client_socket,"\u001B[30;42m->\"\\register <username> <password> <name>\"\u001B[0m - Registers a new user.");
		writeline(client_socket,"\u001B[30;42m->\"\\start_race <race_id>\"\u001B[0m - to start a race.");
		return;
	}
	writeline(client_socket,"\r\n\u001B[30;41m----------USER FUNCTIONS---------\u001B[0m\r\n");
	writeline(client_socket,"\u001B[30;42m->\"\\logout\"\u001B[0m - Logs out of the account.");
	writeline(client_socket,"\u001B[30;42m->\"\\passwd <old_pass> <new_pass> <new_pass>\"\u001B[0m - Change your password.");
	writeline(client_socket,"\u001B[30;42m->\"\\bet <horse_id> <race_id> <ammount>\"\u001B[0m - Use this command to bet on a race and horse.");
	writeline(client_socket,"                                        - Note1: You can't change or add a bet on a race that has already started.");
	writeline(client_socket,"                                        - Note2: If you have already bet on a race and horse but want to change the bet, just use the command like it is the first time you wer betting. It will change your previous bet and adjust your credits accordingly.");
	writeline(client_socket,"                                        - Note3: If you want to delete a bet you made previously, bet again and put 0 on the ammount. It will delete your bet and give you your credits back.");
	writeline(client_socket,"\u001B[30;42m->\"\\show_bets <lim>\"\u001B[0m - shows the bets of the logged user.");
	writeline(client_socket,"                     - Note1: Set lim to zero to see all bets or to a number greater than zero if you want to see only that number of bets.");
	writeline(client_socket,"                     - Note2: Bets are ordered in a way that the most recent ones are the ones at the top.");
	writeline(client_socket,"\u001B[30;42m->\"\\show_all_horses\"\u001B[0m - to see every horse and their  ranks.");
	writeline(client_socket,"\u001B[30;42m->\"\\show_some_horses <n>\"\u001B[0m - to see the best 'n' horses and their ranks.");
	writeline(client_socket,"\u001B[30;42m->\"\\show_horses_on_race <race_id>\"\u001B[0m - to see the horses that are in a race.");
	writeline(client_socket,"\u001B[30;42m->\"\\show_races <ops>\"\u001B[0m - to show races.");
	writeline(client_socket,"                      - Note: ops=\"all\" to show all races and \"active\" to show only races active at the moment.");
	writeline(client_socket,"\u001B[30;42m->\"\\show_race_info <race_id>\"\u001B[0m - to show information on a race.");


	if(is_admin(user_id))
	{
		writeline(client_socket,"\r\n\u001B[30;41m----------ADMIN FUNCTIONS (user related)---------\u001B[0m\r\n");
		writeline(client_socket,"\u001B[30;42m->\"\\change_admin <username> <bool>\"\u001B[0m - to change user rights.");
		writeline(client_socket,"                                    - Note: the originals cannot stop being admins.");
		writeline(client_socket,"\u001B[30;42m->\"\\change_admin_id <user_id> <bool>\"\u001B[0m - same as the last but using user_id.");
		writeline(client_socket,"\u001B[30;42m->\"\\passwd_other <username> <new_pass> <new_pass>\"\u001B[0m - change a user's password.");
		writeline(client_socket,"\u001B[30;42m->\"\\passwd_other_id <user_id> <new_pass> <new_pass>\"\u001B[0m - same as the last but using user_id.");
		writeline(client_socket,"\u001B[30;42m->\"\\show_users <\"a\"/\"l\">\"\u001B[0m - show all users if \"a\" or show only logged users if \"l\".");
		writeline(client_socket,"\u001B[30;42m->\"\\show_bets_other <user_id> <lim>\"\u001B[0m - same as \"show_bets\" but for any user_id.");

		writeline(client_socket,"\r\n\u001B[30;41m----------ADMIN FUNCTIONS (horses related)---------\u001B[0m\r\n");
		writeline(client_socket,"\u001B[30;42m->\"\\add_horse <speed> <horse name>\"\u001B[0m - to add a horse.");
		writeline(client_socket,"\u001B[30;42m->\"\\bet_other <user_id> <horse_id> <race_id> <ammount>\"\u001B[0m - same as \"bet\" but for a generic user_id.");

		writeline(client_socket,"\r\n\u001B[30;41m----------ADMIN FUNCTIONS (races related)---------\u001B[0m\r\n");
		writeline(client_socket,"\u001B[30;42m->\"\\start_race <race_id>\"\u001B[0m - starts a race.");
		writeline(client_socket,"\u001B[30;42m->\"\\add_race <laps>\"\u001B[0m - adds a race.");
		writeline(client_socket,"\u001B[30;42m->\"\\add_to_race <race_id> <[all the horses to add separated by spaces]>\"\u001B[0m - to add a list of horses to a race.");

		writeline(client_socket,"\r\n\u001B[30;41m----------ADMIN FUNCTIONS (administration related)---------\u001B[0m\r\n");
		writeline(client_socket,"\u001B[30;42m->\"\\stop_server\"\u001B[0m - to stop accepting clients and disconnect every one.");
		writeline(client_socket,"                 - Note: only possible in the server.");
		writeline(client_socket,"\u001B[30;42m->\"\\start_server\"\u001B[0m - to start taking clients again.");
		writeline(client_socket,"\u001B[30;42m->\"\\sql_query <query>\"\u001B[0m - to run a query.");
	}
}

bool Client::parse( string ins ) //true always except when "\quit" received.
{
	if(ins=="")
	{
		return true;
	}
	else if(ins[0]!='\\')
	{
		writeline(client_socket,"Commands always have to start with a backslash. Try again.");
		return true;
	}
	parsed_command command=get_parsed_command(ins);
	//print_command(command);
	if(command.cmd=="")
	{
		//writeline(client_socket,"Invalid command.");
		return true;
	}
	try
	{
		if(command.cmd=="\\help") 						{help();}
		else if(command.cmd=="\\clear") 				{clear();}
		else if(command.cmd=="\\quit") 					{return !quit();}
		else if(command.cmd=="\\login") 				{login(command.args[0].get<string>(),command.args[1].get<string>());}
		else if(command.cmd=="\\logout") 				{logout();}
		else if(command.cmd=="\\register") 				{regist(command.args[0].get<string>(),command.args[1].get<string>(),command.args[2].get<string>());}
		else if(command.cmd=="\\change_admin") 			{change_admin(command.args[0].get<string>(),command.args[1].get<bool>());}
		else if(command.cmd=="\\change_admin_id") 		{change_admin(command.args[0].get<int>(),command.args[1].get<bool>());}
		else if(command.cmd=="\\passwd") 				{passwd(user_id,command.args[0].get<string>(),command.args[1].get<string>());}
		else if(command.cmd=="\\passwd_other") 			{passwd(command.args[0].get<string>(),command.args[1].get<string>(),command.args[2].get<string>());}
		else if(command.cmd=="\\passwd_other_id")		{passwd(command.args[0].get<int>(),command.args[1].get<string>(),command.args[2].get<string>());}
		else if(command.cmd=="\\add_horse")				{add_horse(command.args[0].get<double>(),command.args[1].get<string>());}
		else if(command.cmd=="\\start_race") 			{start_race(command.args[0].get<int>());}
		else if(command.cmd=="\\watch_race")			{watch_race(command.args[0].get<int>());}
		else if(command.cmd=="\\add_to_race")			{add_to_race(command.args[0].get<int>(),command.args[1].get<vector<int>>());}
		else if(command.cmd=="\\add_race")				{add_race(command.args[0].get<int>());}
		else if(command.cmd=="\\add_credits") 			{add_credits(command.args[0].get<int>(),command.args[1].get<double>());}
		else if(command.cmd=="\\bet") 					{bet(user_id,command.args[0].get<int>(),command.args[1].get<int>(),command.args[2].get<double>());}
		else if(command.cmd=="\\bet_other") 			{bet(command.args[0].get<int>(),command.args[1].get<int>(),command.args[2].get<int>(),command.args[3].get<double>());}
		else if(command.cmd=="\\stop_server") 			{if(client_socket==IN_SERVER&&is_admin(user_id)) {if(Network::server().srv_running()) Network::server().shutdown_server(); else writeline(client_socket,"Server is already shut down.");} else writeline(client_socket,"You cannot shutdown the server unless you are logged in as an admin in the server.");}
		else if(command.cmd=="\\start_server") 			{if(client_socket==IN_SERVER&&is_admin(user_id)) Network::server().start_server();else writeline(client_socket,"You cannot start the server unless you are logged in as an admin in the server.");}
		else if(command.cmd=="\\show_users") 			{show_users(command.args[0].get<string>());}
		else if(command.cmd=="\\show_all_horses")	 	{show_horses(0);}
		else if(command.cmd=="\\show_some_horses") 		{show_horses(command.args[0].get<int>());}
		else if(command.cmd=="\\show_horses_on_race") 	{show_horses_on_race(command.args[0].get<int>());}
		else if(command.cmd=="\\sql_query") 			{sql_query(command.args[0].get<string>());}
		else if(command.cmd=="\\show_races")			{show_races(command.args[0].get<string>());}
		else if(command.cmd=="\\show_race_info")		{show_race_info(command.args[0].get<int>());}
		else if(command.cmd=="\\show_bets")	 			{show_user_bets(user_id,command.args[0].get<int>());}
		else if(command.cmd=="\\show_bets_other")	 	{show_user_bets(command.args[0].get<int>(),command.args[1].get<int>());}
		else if(command.cmd=="\\show_server_ip")		{writeline(client_socket,"Server_IP: " + string(Network::server().get_ip(Network::server().sockfd)) + ":" + to_string(PORT));}
		//else if(command.cmd=="\\")	 					{}
	}
	catch(SQL_Error e)
	{
		stringstream err_msg;
		err_msg << "There was an error with the command from socket " << client_socket;
		if(client_socket<0)
		{
			err_msg << " which is not logged in.\r\n";
		}
		else
		{
			err_msg << " which is logged in as user_id=" << user_id << " username=\"" << username << "\" and name=\"" << name << "\". ";
		}
		if(!e.err)
		{
			clog(err_msg.str() << "SQL query is NULL.");
			return true;
		}
		//Network::server().writeline(client_socket,"An error that wasn't supposed to have happened has occurred. Please contact the admin");
		clog(err_msg.str() << "Command: \"" << ins << "\". SQL query error:" << endl << PQresultErrorMessage(e.err));
		//PQclear(e.err);
	}
	catch(int i)
	{
		Network::server().writeline(client_socket,"An error that wasn't supposed to have happened has occurred. Please contact the admin.");
	}
	/*catch(logic_error e)
	{
		Network::server().writeline(client_socket,"An error that wasn't supposed to have happened has occurred. Please contact the admin.");
		clog("Caught a logic error. What()="+string(e.what()));
	}*/
	catch(...)
	{
		clog("Caught an unknown error while parsing command=\"" << ins << "\" from socket "+to_string(client_socket)+". Exception type: \""+
																													 Utils::demangle(
																															 __cxxabiv1::__cxa_current_exception_type()->name())+"\"");
		Network::server().writeline(client_socket,"An error that wasn't supposed to have happened has occurred. Please contact the admin.");
	}
	return true;
}

bool check_valid(string pa)
{
	//char ic[]={' ','\'','\\','','"'};
	set<char> invalid_chars ({'\'','\\','/','\"',';','\b'});
	set<char>::iterator it;
	for(unsigned int i=0;i<pa.length();i++)
	{
		it=invalid_chars.find(pa[i]);
		if(*it==pa[i])
			return false;
	}
	if(strstr(pa.c_str(),"--"))
		return false;
	return true;
}

Client::Client(int so) : client_socket(so), user_id(LOGGED_OFF)
{
	vcout << "constructor " << client_socket << endl;
	Network::server().clients.insert(PAIR(client_socket,LOGGED_OFF));

	if(client_socket>0)
	{
		clear();
	}

	if(client_socket>0)
		writeline(client_socket, "Welcome to the client. Please login using your credentials.");
	else
		writeline(client_socket, "Welcome to the server. Please login using your admin credentials.");
	writeline(client_socket, "Type \"\\help\" to quit.");

	//cout << "Socket connected: " << client_socket << endl;

	//usleep(0.01*1000000); //espera 0.01s
}

Client::~Client()
{
	vcout << "destructor " << client_socket << endl;
	vcout << "Client in socket " << client_socket;
	if(user_id<=0) 
	{
		vcout << " who wasn't logged in has disconnected." << endl;
		clog( "Client in socket " << client_socket << "(not logged in) has disconnected. Killing thread and closing socket." );
	}
	else
	{
		vcout << "who was logged in as \"" << user_id << "\" has disconnected.";
		clog( "Client in socket " << client_socket << "(user_id=" << user_id << ") has disconnected. Killing thread and closing socket." );
	}
	//Network::server().clients.erase(client_socket);

	// Fechar o socket
	//shutdown(client_socket,2);
	//close(client_socket);
}

void Client::writeline(int socketfd, string line,bool paragraph)
{
	Network::server().writeline(socketfd,line,paragraph);
}

void Client::regist(string login_name , string pass , string name , bool ad , int cr)	//needs to be erased after use with PQclear()!!!
{
	if(!check_valid(login_name))
	{
		writeline(client_socket,"The login_name you inserted contains invalid characters.");
		return;
	}
	if(!check_valid(pass))
	{
		writeline(client_socket,"The pass you inserted contains invalid characters.");
		return;
	}
	if(!check_valid(name))
	{
		writeline(client_socket,"The name you inserted contains invalid characters.");
		return;
	}
	if(user_id>0 && !is_admin(user_id))
	{
		writeline(client_socket,"Please logout before registering.");
		return;
	}
	transform(login_name.begin(),login_name.end(),login_name.begin(),::tolower); //tolower every char in the string
	PGresult* res;
	stringstream query;
	query << "BEGIN;" << endl;
	query << "INSERT INTO users VALUES (DEFAULT,'" << name << "','" << login_name << "','" << pass << "'," << Utils::b2s(ad) << "," << cr << ");" << endl;
	query << "COMMIT;" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Error in the query from the \"Client::regist\" function with login_name=\"" << login_name << "\", name=\"" << name << "\", pass=\"" << pass << "\", ad=" << b2s(ad) << " and cr=" << cr);
		writeline(client_socket,"Error while registering. If the problem persists, please contact the admin.");
		throw(e);
	}
	PQclear(res);
}

void Client::login(string login_name, string pass)
{
	if(user_id!=LOGGED_OFF)	//throw something
	{
		writeline(client_socket,"You are already logged in as \"" + username + "\".");
		return;
	}
	if(!check_valid(login_name))
	{
		writeline(client_socket,"The login_name you inserted contains invalid characters.");
		return;
	}
	if(!check_valid(pass))
	{
		writeline(client_socket,"The pass you inserted contains invalid characters.");
		return;
	}
	stringstream query;
	PGresult* res;
	query << "SELECT user_id, name" << endl;
	query << "FROM users" << endl;
	query << "WHERE username='" << login_name << "' AND pass='" << pass << "'" << endl;
	//cout << "query:" << endl << query.str();
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)	//implement later
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::login\" using login_name=\"" << login_name << "\" and pass=\"" << pass << "\"");
		writeline(client_socket,"There was an error during the login process. If the problem persists, please contact the admin.");
		//PQclear(res);
		throw (e);
	}
	if( PQntuples( res )==0 )	//(const PGresult*)
	{
		clog("Socket " << client_socket << " tried to login with wrong credentials. login: \"" << login_name << "\" and pass: \"" << pass << "\"." );
		writeline(client_socket,"ACCESS DENIED!!! Wrong credentials!");
		PQclear(res);
		return;
	}

	user_id=atoi(PQgetvalue(res,0,0));

	if(client_socket==IN_SERVER && !is_admin(user_id))
	{
		writeline(client_socket,"ACCESS DENIED!!! You cannot login directly at the server unless you are an admin.");
		clog("User " << user_id << " tried to login at the server. Access denied.");
		user_id=LOGGED_OFF;
		PQclear(res);
		return;
	}

	name=PQgetvalue(res,0,1);
	username=login_name;
	Network::server().clients[client_socket]=user_id;

	clog("Socket " << client_socket << " logged in as user_id " << user_id << ", username \"" << username << "\"");
	query.str("");query.clear();
	query << "ACCESS GRANTED!\r\nWelcome, " << name << " (id=" << user_id << ")!";
	writeline(client_socket,query.str());
	writeline(client_socket,"You have "+to_string(get_user_credits(user_id))+" credits.");
	PQclear(res);
}

bool Client::is_admin(int u)
{	//true se for admin e existir. false otherwise
	if(u<0)
		return false;
	stringstream query;
	query << "SELECT admin FROM users WHERE user_id=" << u;
	PGresult *res;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//cerr << "Unknown error occurred in the query from \"Client::is_admin\" using u=" << u << endl; ;
		clog("Error in internal function \"Client::is_admin\" using u=" << u << ". Query error: " << PQresultErrorMessage(e.err));
		//PQclear(e.err);
		throw (int)1;	//throw something to get caught by "catch(...)" in the parser function
	}
	if(PQntuples( res )==0)	//user doesn't exist
	{
		PQclear(res);
		return false;
	}
	if(!strcmp(PQgetvalue(res,0,0),"t"))
	{
		PQclear(res);
		return true;
	}
	PQclear(res);
	return false;
}

void Client::logout()
{
	if(user_id==LOGGED_OFF)
		return;
	writeline(client_socket,"You are now logged off!");
	clog("Socket " << client_socket << " which was logged with user_id=" << user_id << ", username \"" << username << "\" and name \"" << name << "\" has logged off");
	Network::server().clients[client_socket]=user_id=LOGGED_OFF;
	name="";
	username="";
}

void Client::clear()
{
	if(client_socket>0)
	{
		char clear_str[]="\u001B[2J\u001B[H";
		write(client_socket,clear_str,strlen(clear_str));
	}
	else
		system("clear");
}

bool Client::quit()	//returns true for quitting and false for not quitting (not enough permissions)	//no parse -> return !quit()
{
	if(client_socket>0)	//if not in server
		return true;
	else
	{
		if(!is_admin())	//if not admin or not logged in
		{
			writeline(client_socket,"You don't have permissions to close the server.");
			return false;
		}
		else
		{
			Network::server().shutdown_server();
			return true;
		}
	}
}

int Client::get_user_id(string login_name)
{	//returns LOGGED_OFF if user doesn't exist
	if(!check_valid(login_name))
	{
		//writeline(client_socket,"The login_name you inserted contains invalid characters.");
		return LOGGED_OFF;
	}
	stringstream query;
	PGresult* res;
	query << "SELECT user_id, name" << endl;
	query << "FROM users" << endl;
	query << "WHERE username='" << login_name << "'";
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::get_user_id\" using login_name=" << login_name << ".Query error:n" << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)2;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return LOGGED_OFF;
	}
	int u_i=atoi(PQgetvalue(res,0,0));
	PQclear(res);
	return u_i;
}

void Client::passwd(int id,string old_pass, string new_pass)
{
	if(user_id==LOGGED_OFF)	//throw something
	{
		writeline(client_socket,"You need to be logged in to be able to change your password.");
		return;
	}
	if(id==LOGGED_OFF)
	{
		writeline(client_socket,"Error running your command because of the user you inserted.");
	}
	const bool vo=check_valid(old_pass), vn=check_valid(new_pass);	//valid_old, valid_new
	if(!vo && !vn)
	{
		writeline(client_socket,"Both the passwords you inserted have invalid characters.");
		return;
	}
	else if(!vo)
	{
		writeline(client_socket,"The old password you inserted has invalid characters.");
		return;
	}
	else if(!vn)
	{
		writeline(client_socket,"The new password you inserted has invalid characters.");
		return;
	}
	if(new_pass.size()<6)
	{
		writeline(client_socket,"The new password must be at least 6 characters long.");
		return;
	}
	stringstream query;
	PGresult* res;
	query << "SELECT user_id, name" << endl;
	query << "FROM users" << endl;
	query << "WHERE user_id='" << user_id << "' AND pass='" << old_pass << "'" << endl;
	//cout << "query:" << endl << query.str();
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query to check old_pass in \"Client::passwd\" glitched. login_name: \"" << username << "\"");
		writeline(client_socket,"There was a problem with your request. Please contact an admin.");
		throw(e);
	}
	if(PQntuples( res )==0)
	{
		writeline(client_socket,"Wrong password!");
		PQclear(res);
		return;
	}
	query.str("");
	query.clear();
	query << "BEGIN;";
	query << "UPDATE users" << endl;
	query << "SET pass = '" << new_pass << "'" << endl;
	query << "WHERE user_id=" << id << " AND pass='" << old_pass << "';";
	query << "COMMIT;";
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query to change the pass in \"Client::passwd\" glitched. new_pass: \"" << new_pass << "\"");
		writeline(client_socket,"There was a problem with your request.\r\nAre you sure your new password doesn't have illegal characters and has the minimal length?\r\nIf the problem persists, please contact an admin.");
		throw(e);
	}
	writeline(client_socket,"Successfully changed user_id id's password.");
	PQclear(res);
}

void Client::change_admin(int id, bool ad)
{
	if(!is_admin(user_id))
	{
		writeline(client_socket,"You can't use this function for you don't have enough permissions.");
		return;
	}
	if(id==1||id==2||id==3)
	{
		if(ad)
			return;
		writeline(client_socket,"The originals are ALWAYS admins. Originals are forever! Long live the Originals!");
		return;
	}
	stringstream query;
	PGresult* res;
	query << "BEGIN;";
	query << "UPDATE users" << endl;
	query << "SET admin = " << Utils::b2s(ad) << endl;
	query << "WHERE user_id=" << id << ";";
	query << "COMMIT;";
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	query.str("");
	query.clear();
	query << "User " << id << " is now ";
	if(!ad)
		query << "a regular user.";
	else
		query << "an admin.";
	writeline(client_socket,query.str());
	PQclear(res);
}

int Client::get_horse_id(string h_name)
{
	if(!check_valid(h_name))
	{
		writeline(client_socket,"The horse name you inserted contains invalid characters.");
		return -1;
	}
	stringstream query;
	PGresult* res;
	query << "SELECT horse_id" << endl;
	query << "FROM horses" << endl;
	query << "WHERE name='" << h_name << "'";
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::get_horse_id\" using horse_name=" << h_name << ".Query error:n" << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)2;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return -1;
	}
	int h_i=atoi(PQgetvalue(res,0,0));
	PQclear(res);
	return h_i;	
}

void Client::add_horse(double speed , string h_name)
{
	if(!is_admin(user_id))
	{
		writeline(client_socket,"You can't use this function for you don't have enough permissions.");
		return;
	}
	if(!check_valid(h_name))
	{
		writeline(client_socket,"Invalid horse name.");
		return;
	}
	if(speed<=0)
	{
		writeline(client_socket,"Speed has to be greater than 0.");
		return;
	}
	if(h_name.length()<1 || h_name.length()>15)
	{
		writeline(client_socket,"Horse name must have between 1 and 15 characters.");
		return;
	}
	if(get_horse_id(h_name)>0)
	{
		writeline(client_socket,"Horse \""+h_name+"\" already exists.");
		return;
	}
	transform(h_name.begin(),h_name.end(),h_name.begin(),::tolower); //tolower every char in the string
	stringstream query;
	PGresult* res;
	query << "BEGIN;" << endl;
	query << "INSERT INTO horses VALUES (DEFAULT,'" << h_name << "'," << speed << ");" << endl;
	query << "COMMIT;" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	query.str("");
	query.clear();
	query << "SELECT horse_id FROM horses WHERE name='" << h_name << "'";
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		writeline(client_socket,"Error while running your command after adding horse. If the problem persists, please contact the admin.");
		throw(e);
	}
	int h_i=atoi(PQgetvalue(res,0,0));
	query.str("");
	query.clear();
	query << "Horse added with horse_id=" << h_i << endl;
	writeline(client_socket,query.str());
	query.str("");
	query.clear();
	query << "User " << user_id << " added horse \"" << h_name << "\" with speed=" << speed << " and horse_id=" << h_i << endl;
	clog(query.str());
	PQclear(res);
}

void Client::add_to_race(int race,vector<int> horses)
{
	stringstream query;
	if(!check_race(race))
	{
		query << "Race " << race << " does not exist.";
		writeline(client_socket,query.str());
		return;
	}
	if(!is_admin(user_id))
	{
		writeline(client_socket,"You need to be logged as an admin to do this.");
		return;
	}
	if(horses.size()==0)
	{
		return;
	}
	PGresult* res;
	double state;
	for(vector<int>::iterator it=horses.begin();it!=horses.end();it++)
	{
		if(get_num_horses_on_race(race)>=MAX_HORSES_ON_RACE)
		{
			writeline(client_socket,"Couldn't add all of the horses indicated because the race is full (max is " +to_string(MAX_HORSES_ON_RACE)+ " horses per race.");
			return;
		}
		query.str("");
		query.clear();
		if(!check_horse(*it))
		{
			query << "Horse with id=" << *it << " does not exist." << endl;
			writeline(client_socket,query.str());
			continue;
		}
		if(check_horse_in_race(*it,race))
		{
			query << "Horse " << *it << " already is in race " << race << ".";
			writeline(client_socket,query.str());
			continue;
		}
		state=(double)rand()/RAND_MAX/2+0.5;	//random number between [0.5;1[
		query << "BEGIN;" << endl;
		query << "INSERT INTO are_on VALUES (" << *it << "," << race << "," << state << ",NULL);" << endl;
		query << "COMMIT;" << endl;
		try
		{
			EXECUTE;
		}
		catch(SQL_Error e)
		{
			writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
			throw(e);
		}
		PQclear(res);
	}
}

void Client::add_race(int laps)
{
	if(!is_admin(user_id))
	{
		writeline(client_socket,"Only admins can do this.");
		return;
	}
	if(laps<=0)
	{
		writeline(client_socket,"Number of laps must be greater than zero.");
		return;
	}
	PGresult* res;
	stringstream query;
	query << "BEGIN;" << endl;
	query << "INSERT INTO races VALUES (" << laps <<",DEFAULT,DEFAULT,DEFAULT);" << endl;
	query << "COMMIT;" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	PQclear(res);
	query.str("");query.clear();
	query << "SELECT race_id" << endl;
	query << "FROM races" << endl;
	query << "ORDER BY race_id DESC" << endl;
	query << "LIMIT 1;" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	int r_i=stoi(PQgetvalue(res,0,0));
	PQclear(res);
	writeline(client_socket,"Added race " + to_string(r_i) + ".");
}

bool Client::check_race(int r_i)
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
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::check_race\" using race_id=" << r_i << ". Query error:n" << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)2;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;	
}

bool Client::check_horse(int h_i)
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
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::check_race\" using race_id=" << h_i << ". Query error:n" << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)2;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;	
}

bool Client::check_user(int u_i)
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
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::check_user\" using user_id=" << u_i << ". Query error:n" << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)3;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;	
}

double Client::get_user_credits(int u_i)
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
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::get_user_credits\" using user_id=" << u_i << ". Query error: " << PQresultErrorMessage(e.err));
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

string Client::get_race_date(int r_i)
{
	if(!check_race(r_i))
		return "";
	stringstream query;
	PGresult* res;
	query << "SELECT time_created" << endl;
	query << "FROM races" << endl;
	query << "WHERE race_id=" << r_i;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::get_race_date\" using race_id=" << r_i << ". Query error: " << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)8;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return "";
	}
	string ret=PQgetvalue(res,0,0);
	PQclear(res);
	return ret;
}

int Client::get_num_horses_on_race(int r_i)
{
	if(r_i<=0)
		return -1;
	stringstream query;
	PGresult* res;
	query << "SELECT num_horses_race(" << r_i << ")" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::get_num_horses_on_race\" using race_id=" << r_i << ". Query error: " << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)7;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return -1;
	}
	int num=stoi(PQgetvalue(res,0,0));
	PQclear(res);
	return num;
}

bool Client::check_horse_available(int h_i)
{
	if(!check_horse(h_i))
		return false;
	for(auto &it:races)
	{
		if(it.second->finished)
			continue;	//if it is already finished, then the horse can be available even if it was in this race
		for(auto &i:it.second->horses)
		{
			if(i.first==h_i)
				return false;
		}
	}
	return true;
}

int Client::check_all_horses_available(int r_i)
{
	if(!check_race(r_i))
		return 0;
	stringstream query;
	PGresult* res;
	query << "SELECT horse_id" << endl;
	query << "FROM are_on" << endl;
	query << "WHERE race_id=" << r_i << endl;
	query << "ORDER BY horse_id;" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		clog("Error in internal function \"Client::check_all_horses_available\" using race_id=" << r_i <<  ". Query error: " << PQresultErrorMessage(e.err));
		throw (int)11;
	}
	int n=PQntuples(res);
	for(int i=0;i<n;i++)
		if( !check_horse_available(stoi(PQgetvalue(res,i,0))) )
		{
			n=stoi(PQgetvalue(res,i,0));
			PQclear(res);
			return n;
		}

	PQclear(res);
	return 0;
}

double Client::check_bet(int u_i,int h_i,int r_i)
{
	if(u_i<=0||h_i<=0||r_i<=0)
		return -1;
	stringstream query;
	PGresult* res;
	query << "SELECT bet" << endl;
	query << "FROM bets" << endl;
	query << "WHERE user_id=" << u_i << " AND horse_id=" << h_i << " AND race_id=" << r_i;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::check_bet\" using user_id=" << u_i << ", horse_id=" << h_i << " and race_id=" << r_i <<  ". Query error: " << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)6;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return -1;
	}
	double cr=stoi(PQgetvalue(res,0,0));
	PQclear(res);
	return cr;
}

void Client::watch_race(int r_i)
{
	if(!check_race(r_i))
	{
		writeline(client_socket,"Invalid race.");
		return;
	}
	auto temp=races.find(r_i);
	if(temp==races.end())
	{
		writeline(client_socket,"Race not active.");
		return;
	}
	if(temp->second->sockets_watching.find(client_socket)!=temp->second->sockets_watching.end())
	{
		writeline(client_socket,"Client already watching the race.");
	}
	clear();
	string dummy;
	temp->second->sockets_watching.insert(client_socket);
	Network::server().readline(client_socket,dummy);
	clear();
	temp=races.find(r_i);
	if(temp==races.end())
		return;
	temp->second->sockets_watching.erase(client_socket);
}

void Client::add_credits(int u_i, double cr)
{
	if(!is_admin(user_id))
	{
		writeline(client_socket,"You need to be logged as an admin to do this.");
		return;
	}
	if(get_user_credits(u_i)+cr<0)
	{
		writeline(client_socket,"This transaction would make the credits negative. Transaction denied.");
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
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	PQclear(res);
}

bool Client::check_race_started(int r_i)
{	//true se for admin e existir. false otherwise
	if(r_i<0)
		return false;
	stringstream query;
	query << "SELECT started FROM races WHERE race_id=" << r_i;
	PGresult *res;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//cerr << "Unknown error occurred in the query from \"Client::is_admin\" using u=" << u << endl; ;
		clog("Error in internal function \"Client::get_race_started\" using race_id=" << r_i << ". Query error: " << PQresultErrorMessage(e.err));
		//PQclear(e.err);
		throw (int)1;	//throw something to get caught by "catch(...)" in the parser function
	}
	if(PQntuples( res )==0)	//user doesn't exist
	{
		PQclear(res);
		return false;
	}
	if(!strcmp(PQgetvalue(res,0,0),"t"))
	{
		PQclear(res);
		return true;
	}
	PQclear(res);
	return false;
}

void Client::bet(int u_i,int h_i,int r_i,double cr)
{
	if(user_id<=0)
	{
		writeline(client_socket,"You need to be logged in to be able to bet.");
		return;
	}
	if(!check_user(u_i))
	{
		writeline(client_socket,"Invalid user_id.");
		return;
	}
	if(!check_horse(h_i))
	{
		writeline(client_socket,"Invalid horse_id.");
		return;
	}
	if(!check_race(r_i))
	{
		writeline(client_socket,"Invalid race_id.");
		return;
	}
	if(check_race_started(r_i))
	{
		writeline(client_socket,"You can't change or add bets on a race that has already started.");
		return;
	}
	if(cr<0)
	{
		writeline(client_socket,"The bet ammount has to be >0 or =0 to delete the bet.");
		return;
	}
	if(user_id!=u_i&&!is_admin(user_id))	//betting for other people is only available to admins
	{
		writeline(client_socket,"You can't bet for other people unless you are an admin.");
		return;
	}
	stringstream query;
	PGresult* res;
	int previous_bet=check_bet(u_i,h_i,r_i);
	if(previous_bet>0)	//bet already existed
	{
		if(cr==0)
		{
			query << "BEGIN;" << endl;

			query << "DELETE FROM bets" << endl;
			query << "WHERE user_id=" << u_i << " AND race_id=" << r_i << " AND horse_id=" << h_i << ";" << endl;

			query << "UPDATE users" << endl;
			query << "SET credits=credits+" << previous_bet << endl;
			query << "WHERE user_id=" << u_i << ";";

			query << "COMMIT;";
			try
			{
				EXECUTE;
			}
			catch(SQL_Error e)
			{
				//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
				writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
				throw(e);
			}
			if(u_i==user_id)
				writeline(client_socket,"Your bet on race_id=" + to_string(r_i) + " and on horse_id=" + to_string(h_i) + " was deleted.");
			else
				writeline(client_socket,"User " + to_string(u_i) + "'s bet on race_id=" + to_string(r_i) + " and on horse_id=" + to_string(h_i) + " was deleted.");
			return;
		}
		if(cr>get_user_credits(user_id)+previous_bet)
		{
			writeline(client_socket,"You can't bet credits that you don't have. Get to an admin if you want more credits.");
			return;
		}
		query << "BEGIN;";

		query << "UPDATE bets" << endl;
		query << "SET bet = " << cr << endl;
		query << "WHERE user_id=" << u_i << " AND horse_id=" << h_i << " AND race_id=" << r_i << ";";

		query << "UPDATE bets" << endl;
		query << "SET balance = " << -cr << endl;
		query << "WHERE user_id=" << u_i << " AND horse_id=" << h_i << " AND race_id=" << r_i << ";";		

		query << "UPDATE users" << endl;
		query << "SET credits=credits-(" << cr-previous_bet << ")" << endl;
		query << "WHERE user_id=" << u_i << ";";

		query << "COMMIT;";
		try
		{
			EXECUTE;
		}
		catch(SQL_Error e)
		{
			//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
			writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
			throw(e);
		}
		if(u_i==user_id)
			writeline(client_socket,"You had already bet in this race and horse so your previous bet was successfuly replaced by this new one.");
		else
			writeline(client_socket,"User " + to_string(u_i) + " had already bet in this race and horse so your previous bet was successfuly replaced by this new one.");
		PQclear(res);
		return;
	}
	else	//bet didn't yet exist so it will be created
	{
		if(cr==0)
		{
			writeline(client_socket,"You can't bet 0 credits.");
			return;
		}
		if(cr>get_user_credits(user_id))
		{
			writeline(client_socket,"You can't bet credits that you don't have. Get to an admin if you want more credits.");
			return;
		}
		query << "BEGIN;" << endl;

		query << "INSERT INTO bets VALUES (DEFAULT," << u_i << "," << h_i << "," << r_i << "," << cr << "," << -cr << ");" << endl;

		query << "UPDATE users" << endl;
		query << "SET credits=credits-(" << cr << ")" << endl;
		query << "WHERE user_id=" << u_i << ";";

		query << "COMMIT;" << endl;
		try
		{
			EXECUTE;
		}
		catch(SQL_Error e)
		{
			writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
			throw(e);
		}
		writeline(client_socket,"Bet successfuly added to the system.");
		PQclear(res);
		return;
	}
}

void Client::start_race(int r_i)
{
	if(!is_admin(user_id))
	{
		writeline(client_socket,"Command only available for admins.");
		return;
	}
	if(!check_race(r_i))
	{
		writeline(client_socket,"Invalid race_id.");
		return;
	}
	if(races.find(r_i)!=races.end())
	{
		writeline(client_socket,"Race already active.");
		return;
	}
	if(get_race_started(r_i))
	{
		writeline(client_socket,"Race already started once.");
		return;
	}
	if(get_num_horses_on_race(r_i)<MIN_HORSES_ON_RACE)
	{
		writeline(client_socket,"Race doesn't have enough participants yet.");
		return;
	}
	int h=check_all_horses_available(r_i);
	if(h>0)
	{
		writeline(client_socket,"At least the horse "+to_string(h)+" is already in another active race.");
		return;
	}
	races.insert( pair<int,class Race*>(r_i,new Race(r_i,get_race_laps(r_i))) );
	writeline(client_socket,"Race started.");
}

void Client::show_races(string flag)
{
	if(user_id<=0)
	{
		writeline(client_socket,"You need to login to be able to use this command.");
		return;
	}
	if(flag!="all"&&flag!="active")
	{
		writeline(client_socket,"Invalid flag. Use \"all\" for seeing all of the races and \"active\" to see only the races active at the moment.");
		return;
	}
	if(flag=="all")
	{
		stringstream query;
		PGresult* res;
		query << "SELECT *" << endl;
		query << "FROM races" << endl;
		try
		{
			EXECUTE;
		}
		catch(SQL_Error e)
		{
			//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
			writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
			throw(e);
		}
		S.printResult(res,client_socket);
		PQclear(res);
		return;
	}
	if(races.size()==0)
	{
		writeline(client_socket,"There are no active races at the moment.");
		return;
	}
	writeline(client_socket,"These are the active races:");
	for(auto& it:races)
	{
		writeline(client_socket,"race_id="+to_string(it.first)+" | #horses="+to_string(it.second->horses.size()));
	}
	writeline(client_socket,"To go watch any of these races, use the command \"watch_race <race_id>\".");
}

void Client::show_race_info(int r_i)
{
	if(user_id<=0)
	{
		writeline(client_socket,"You need to login to be able to use this command.");
		return;
	}
	if(!check_race(r_i))
	{
		writeline(client_socket,"Invalid race_id.");
		return;
	}
	writeline(client_socket,"#horses="+to_string(get_num_horses_on_race(r_i)));
	writeline(client_socket,"Laps="+to_string(get_race_laps(r_i)));
	writeline(client_socket,"Time created="+get_race_date(r_i));
	writeline(client_socket,"Started="+Utils::b2s(get_race_started(r_i)));
	auto temp=races.find(r_i);
	bool temp_b = temp==races.end()?false:true;
	writeline(client_socket,"Active now="+Utils::b2s(temp_b));
	if(temp_b)
		writeline(client_socket,"Finished="+Utils::b2s(temp->second->finished));
	writeline(client_socket,"\r\nHorses on race:\r\n");
	show_horses_on_race(r_i);
}

bool Client::get_race_started(int r_i)
{
	if(!check_race(r_i))
		return "";
	stringstream query;
	PGresult* res;
	query << "SELECT started" << endl;
	query << "FROM races" << endl;
	query << "WHERE race_id=" << r_i;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::get_race_started\" using race_id=" << r_i << ". Query error: " << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)9;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return false;
	}
	bool ret=Utils::s2b(PQgetvalue(res,0,0));
	PQclear(res);
	return ret;
}

int Client::get_race_laps(int r_i)
{
	if(!check_race(r_i))
		return 0;
	stringstream query;
	PGresult* res;
	query << "SELECT laps" << endl;
	query << "FROM races" << endl;
	query << "WHERE race_id=" << r_i;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::get_race_laps\" using race_id=" << r_i << ". Query error: " << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)10;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return 0;
	}
	int ret=stoi(PQgetvalue(res,0,0));
	PQclear(res);
	return ret;
}

void Client::show_horses(int n)
{
	if(user_id<=0)
	{
		writeline(client_socket,"You need to login to use this command.");
		return;
	}
	stringstream query;
	PGresult* res;
	query << "SELECT *" << endl;
	query << "FROM horse_ranks" << endl;
	if(n>0)
		query << "LIMIT " << n << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	S.printResult(res,client_socket);
	PQclear(res);
}

void Client::show_user_bets(int u_i,int lim)
{
	if(user_id<=0)
	{
		writeline(client_socket,"You need to login to use this command.");
		return;
	}
	if(!is_admin(user_id))
	{
		writeline(client_socket,"You can't see other people's bets");
		return;
	}
	if(!check_user(u_i))
	{
		writeline(client_socket,"User " + to_string(u_i) + " doesn't exist.");
		return;
	}
	stringstream query;
	PGresult* res;
	query << "SELECT *" << endl;
	query << "FROM bets" << endl;
	query << "WHERE user_id=" << u_i << endl;
	query << "ORDER BY race_id DESC , horse_id" << endl;
	if(lim>0)
		query << "LIMIT " << lim << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	S.printResult(res,client_socket);
	PQclear(res);
}

void Client::show_users(string opt)
{
	if(!is_admin(user_id))
	{
		writeline(client_socket,"You need to login as an admin to use this command.");
		return;
	}
	if(opt!="l"&&opt!="a")
	{
		writeline(client_socket,"Invalid option. Option \"l\" to show only logged in users and \"a\" to show every user on the system.");
		return;
	}
	stringstream query;
	PGresult* res;

	if(opt=="a")
	{
		query << "SELECT user_id,name,username,admin,credits" << endl;
		query << "FROM users" << endl;
		query << "ORDER BY user_id;" << endl;
		try
		{
			EXECUTE;
		}
		catch(SQL_Error e)
		{
			//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
			writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
			throw(e);
		}
		S.printResult(res,client_socket);
		PQclear(res);
		return;
	}

	map<int,int> clients_photo=Network::server().clients;
	query << "CREATE TEMP TABLE foo (" << endl;
	query << "user_id integer NOT NULL," << endl;
	query << "client_socket char varying NOT NULL" << endl;
	query << ");" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	query.str("");query.clear();
	PQclear(res);

	string temp;

	for(auto &it : clients_photo)
	{
		if(it.second>0)	//if logged_in
		{
			temp= it.first<=0 ? "SERVER" : to_string(it.first);
			query << "INSERT INTO foo VALUES(" << it.second << ",'" << temp << "');" ;
			try
			{
				EXECUTE;
			}
			catch(SQL_Error e)
			{
				//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
				writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
				throw(e);
			}
			PQclear(res);
			query.str("");query.clear();
		}
	}

	query << "SELECT user_id,name,username,admin,credits,client_socket" << endl;
	query << "FROM users JOIN foo USING (user_id)" << endl;
	query << "ORDER BY user_id;" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	query.str("");query.clear();
	S.printResult(res,client_socket);
	PQclear(res);

	query << "DISCARD TEMP;" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	PQclear(res);
}

void Client::show_horses_on_race(int r_i)
{
	if(user_id<=0)
	{
		writeline(client_socket,"Log in to use this command.");
		return;
	}
	if(!check_race(r_i))
	{
		writeline(client_socket,"Invalid race_id.");
		return;
	}
	stringstream query;
	PGresult* res;
	query << "SELECT horse_id,name,speed" << endl;
	query << "FROM are_on JOIN horses USING (horse_id)" << endl;
	query << "WHERE race_id=" << r_i << endl;
	query << "ORDER BY horse_id;" << endl;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"Error while running your command. If the problem persists, please contact the admin.");
		throw(e);
	}
	S.printResult(res,client_socket);
	PQclear(res);
}

void Client::sql_query(string q)
{
	if(!is_admin(user_id))
	{
		writeline(client_socket,"Only admins can issue this command.");
		return;
	}
	if(q=="")
		return;
	PGresult* res=NULL;
	try
	{
		res=SQLServer::server().executeSQL(q);
	}
	catch(SQL_Error e)
	{
		//clog("Query in \"Client::change_admin\" glitched. user_id=" << id << ".");
		writeline(client_socket,"The query entered originated the following error message:.\r\n"+string(PQresultErrorMessage(e.err)));
		clog("User " << user_id << " used the \"sql_query\" command and originated an error. Query inserted:" << endl << q << endl << endl << "Error message:" << endl << PQresultErrorMessage(res));
		return;
	}
	clog("User " << user_id << " used the \"sql_query\" command. Query inserted:" << endl << q);
	S.printResult(res,client_socket);
	PQclear(res);
}

using namespace std;
using namespace boost;

arg::arg() {}
arg::arg(boost::any d) : data(d) {}
arg::arg(const arg& d) : data(d.data) {}

bool arg::set(const boost::any& st) {
	if(!this->isEmpty() && !this->isOfType(st.type()))
		return false;
	this->data = st;
	return true;
}

template <typename T> T arg::get() const {
	return boost::any::any_cast<T>(&data);
}

bool arg::isEmpty() const {
	return this->data.empty();
}

bool arg::isOfType(const type_info& t) const {
	return t == this->data.type();
}

//template arg::get<int>();
//template arg::get<bool>();
//template arg::get<double>();
//template arg::get<string>();
//template arg::get<vector<int>>();

//https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_codes	//http://www.termsys.demon.co.uk/vtansi.htm

bool check_horse_in_race(int h_i, int r_i)
{
	stringstream query;
	PGresult* res;
	query << "SELECT *" << endl;
	query << "FROM are_on" << endl;
	query << "WHERE horse_id=" << h_i << " AND race_id=" << r_i;
	try
	{
		EXECUTE;
	}
	catch(SQL_Error e)
	{
		//clog(cout << "Unknown error occurred in the query from \"Client::get_user_id\" using login_name=\"" << login_name << "\"");
		clog("Error in internal function \"Client::check_horse_in_race\" using race_id=" << r_i << " and horse_id=" << h_i << ". Query error:n" << PQresultErrorMessage(e.err));
		//PQclear(res);
		throw (int)4;
	}
	if( PQntuples( res )==0 )
	{
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;
}