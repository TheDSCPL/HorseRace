#ifndef DBMS_HPP
#define DBMS_HPP 1

#ifndef S
#define S SQLServer::server()
#endif

#include <postgresql/libpq-fe.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> //strdup
#include <algorithm>
#include <vector>

#define SHOW 0
#define HIDE 1

using namespace std;

class PreparedStatement
{
public:
	const string name;
	const string declaration;
};

class SQLServer
{
	SQLServer();
	~SQLServer();
	SQLServer(SQLServer const&);	// Don't Implement
	void operator=(SQLServer const&);	// Don't implement
	vector<PreparedStatement*> preparedStatements;
public:
	static SQLServer& server()
	{
		static SQLServer s;
		return s;
	}
	void start();
	void stop();
	PGresult* executeSQL(string sql);
	static void printResult(PGresult *result,int client_socket,int hide_num_rows=SHOW);
private:
	PGconn* conn;
	bool SQL_server_running;
};

class SQL_Error
{
public:
	SQL_Error(PGresult* e);
	SQL_Error(SQL_Error *e);
	~SQL_Error();
	PGresult *err;
private:
};

#endif