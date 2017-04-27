#ifndef DBMS_HPP
#define DBMS_HPP 1

#ifndef S
#define S SQLServer::server()
#endif

#include <postgresql/libpq-fe.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>

#define SHOW 0
#define HIDE 1

class SQLResultTable;

class Tuple {
	friend class SQLResultTable;

    Tuple(const std::vector<std::string *> &);    //only SQLResultTable can use this constructor
	const std::vector<std::string*> values;
public:
	Tuple(const Tuple&);

    ~Tuple();
	std::vector<std::string> getValues() const;
	std::string getString(unsigned int) const;
	int getInt(unsigned int) const;
	double getDouble(unsigned int) const;
	bool getBool(unsigned int) const;

    std::string operator[](unsigned int i) const {
        return std::string(*values[i]);
    }

};

class SQLResultTable {
	friend class SQLResult;	//only SQLResult can create objects of this class
	const std::vector<Tuple*> tuples;
    const std::vector<std::string> columnNames;
	const std::string printedTable;
	SQLResultTable(const PGresult *);
	void operator=(SQLResultTable const&);	// Don't Implement

    static std::vector<Tuple*> getTuplesFromPGresult(const PGresult *);
    static std::vector<std::string> getColumnNamesFromPGresult(const PGresult *);
	static std::string getPrintedTable(const PGresult *);
public:
    SQLResultTable(SQLResultTable const&);
    ~SQLResultTable();
	//Getters
	unsigned long getNumberOfTuples() const;
	unsigned long getNumberOfColumns() const;
	std::string getColumnName(unsigned int n) const;
	const std::vector<std::string>& getColumnNames() const;

    std::vector<Tuple> getTuples() const;
	bool isEmpty() const;

    void print(std::ostream &out) const;

    operator const char *() const {
        return printedTable.c_str();
    }

    const Tuple *const operator[](unsigned int i) const {
        return tuples.at(i);
    }
};

class SQLResult {
	//PGresult* const res;    //=PQcopyResult(origin,PG_COPYRES_ATTRS|PG_COPYRES_TUPLES|PG_COPYRES_NOTICEHOOKS|PG_COPYRES_EVENTS);
    const std::string cmdMessage;
    const ExecStatusType status;
    const std::string errorMessage;
    const SQLResultTable resultTable;

	friend class PreparedStatement;
    //Converts from PGresult to the wrapper class SQLResult
	SQLResult(PGresult*);

	void operator=(SQLResult const&);	// Don't Implement

public:
	SQLResult(const SQLResult&);
    //TODO: Implement copy constructor as public too after I'm done removing the PGresult pointer from the other "nested"/friend classes
    ~SQLResult();

    const SQLResultTable& getResultTable() const;
    std::string getCommandMessage() const;
    std::string getErrorMessage() const;
    bool hasError() const;
    bool hasTableResult() const;

    operator const char *() const {
        if (hasError())
            return getErrorMessage().c_str();
        else if (hasTableResult())
            return getResultTable();
        else
            return getCommandMessage().c_str();
    }
};

class PreparedStatement
{
	friend class SQLServer;

    //char ** argsConcat;   //if I delete this FOR SOME WITCHCRAFT the compilation fails. This is doing nothing
    //char** aasdfksgiasdhf;
    char **const _temp;    //used only in the "run" method. I'll save it in the object so I don't need to allocate memory every time I call the function. This pointer can definitely be reused by each object and doesn't need to be allocated every time the "run" method is run.

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
	PGconn* conn = NULL;
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

class DBMSError : public std::exception
{
private:
	std::string whatMessage;
public:
	DBMSError(const DBMSError&);
	DBMSError(const std::string&);
	virtual ~DBMSError();
	const char* what() const throw();
};

class DBMSErrorCreatingPreparedStatement : public DBMSError {
public:
	DBMSErrorCreatingPreparedStatement(const std::string&);
};

class DBMSErrorRunningPreparedStatement : public DBMSError {
public:
	DBMSErrorRunningPreparedStatement(const std::string&);
};

#endif