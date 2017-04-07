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
#include <map>

#define SHOW 0
#define HIDE 1

class SQLResultTable;

class Tuple {
	friend class SQLResultTable;
	Tuple(/*SQLResultTable*, */const std::vector<std::string>&);	//only SQLResultTable can use this constructor
	const std::vector<std::string> values;
	//SQLResultTable* table;
public:
	Tuple(const Tuple&);
	const std::vector<std::string>& getValues() const;
	std::string getString(unsigned int) const;
	int getInt(unsigned int) const;
	double getDouble(unsigned int) const;
	bool getBool(unsigned int) const;
};

class SQLResultTable {
	friend class SQLResult;	//only SQLResult can create objects of this class
	std::vector<Tuple*> tuples;
	PGresult* pGresult;
	SQLResultTable(PGresult*);
	~SQLResultTable();
	SQLResultTable(SQLResultTable const&);	// Don't Implement
	void operator=(SQLResultTable const&);	// Don't Implement
public:
	//Getters
	const std::vector<Tuple*> getTuples() const;
	int getNumberOfTuples() const;
	int getNumberOfColumns() const;
	const char* getColumnName(unsigned int n) const;
	std::vector<std::string> getColumnNames() const;
	bool isEmpty() const;

	void print(const int client_socket, const bool hide_num_rows) const;
	void print(std::ostream& out, const bool hide_num_rows) const;
};

//TODO: SQLResult CANNOT contain the PGresult pointer. It must be fully independent meaning that it must take everything that it needs from the PGresult and then be autonomous from it.
class SQLResult {
public:
	const bool isCopy;
	PGresult* const res;

	friend class PreparedStatement;
	SQLResult(PGresult*);
	~SQLResult();

	void operator=(SQLResult const&);	// Don't Implement

public:
	SQLResult(const SQLResult&);
};

class PreparedStatement
{
	friend class SQLServer;
	char ** const argsConcat;	//used only in the run function. I'll save it in the object so I don't need to allocate memory every time I call the function. This pointer can definitely be reused and doesn't need to be allocated every time.

	PreparedStatement (std::string const& name, std::string const& declaration);
	~PreparedStatement();
	PreparedStatement(PreparedStatement const&);	// Don't Implement
	void operator=(PreparedStatement const&);	// Don't Implement

public:
	const std::string name;
	const std::string declaration;
	SQLResult run(const std::vector<std::string> &args) const;
};

class SQLServer
{
public:
	PGconn* conn;
	std::map<std::string const,PreparedStatement const*> preparedStatements;

	friend PreparedStatement::PreparedStatement(std::string const &name, std::string const &declaration);
	friend SQLResult PreparedStatement::run(const std::vector<std::string> &args) const;

	SQLServer();
	~SQLServer();
	SQLServer(SQLServer const&);	// Don't Implement
	void operator=(SQLServer const&);	// Don't implement
public:
	static SQLServer& server()
	{
		static SQLServer s;
		return s;
	}
	void start();
	void stop();
	//DEPRECATED
	PGresult* executeSQL(std::string const& sql);
	//DEPRECATED
	static void printResult(PGresult *result,int client_socket,int hide_num_rows=SHOW);

	void requestNewPreparedStatement(std::string const& name, std::string const& declaration);
	const PreparedStatement* getPreparedStatement(std::string const& name) const;
};

//DEPRECATED
class SQL_Error
{
public:
	SQL_Error(PGresult*);
	SQL_Error(const SQL_Error *);
	SQL_Error(const SQL_Error&);
	SQL_Error(const std::string&);
	~SQL_Error();
	PGresult *err;
};

class TupleConversionError : std::logic_error {
public:
	TupleConversionError(const std::string& err);
};

#endif