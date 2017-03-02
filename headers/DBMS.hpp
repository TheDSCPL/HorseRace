#ifndef DBMS_HPP
#define DBMS_HPP 1

#ifndef S
#define S SQL_server_class::server()
#endif

#include "Error.hpp"
#include "Log.hpp"
#include "Client.hpp"

#include <postgresql/libpq-fe.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> //strdup
#include <algorithm>

#define SHOW 0
#define HIDE 1

using namespace std;

string b2s(bool b);

bool s2b(string s);

class SQL_server_class
{
private:	//singleton stuff
	SQL_server_class();
	~SQL_server_class();
	SQL_server_class(SQL_server_class const&);		// Don't Implement
	void operator=(SQL_server_class const&);	// Don't implement

public:
	static SQL_server_class& server()
	{
		static SQL_server_class s;
		return s;
	}
	void SQL_server_start();
	void SQL_server_stop();
	PGresult* executeSQL(string sql);
	void print_result(PGresult *result,int client_socket,int hide_num_rows=SHOW);

private:
	PGconn* conn;
	bool SQL_server_running;
};


#endif