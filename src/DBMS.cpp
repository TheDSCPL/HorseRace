#include <sstream>
#include <boost/regex.hpp>
#include <regex>

#include "../headers/DBMS.hpp"
#include "../headers/Properties.hpp"
#include "../headers/Log.hpp"
#include "../headers/Utils.hpp"

#ifndef vcout //verbose cout.
#define vcout if(false) cout
#endif

using namespace std;

//------------------------------------------ Tuple ------------------------------------------//

Tuple::Tuple(const vector<string *> &v) : values(Utils::copyDynamicVector(v)) {}

Tuple::~Tuple() {
    for(string* s : values)
        if(s)
            delete s;
}

Tuple::Tuple(const Tuple &o) : values(Utils::copyDynamicVector(o.values)) {}

vector<string> Tuple::getValues() const {
    //create a copy of "values" and return it
    vector<string> ret;
    for(string* s : values)
        ret.push_back(string(*s));
    return ret;
}

//User must check if a value is NULL before trying to get the value
string Tuple::getString(unsigned int index) const {
    if(index < 0 || index >= values.size())
        throw out_of_range("tuple");
    return string(*values.at(index));
}

//User must check if a value is NULL before trying to get the value
int Tuple::getInt(unsigned int index) const {
    if(index < 0 || index >= values.size())
        throw out_of_range("tuple");
    const string* s = values.at(index);
    if(!Utils::isInt(*s))
        throw logic_error("Tried to get int but it's not int.");
    return atoi(s->c_str());
}

//User must check if a value is NULL before trying to get the value
double Tuple::getDouble(unsigned int index) const {
    if(index < 0 || index >= values.size())
        throw out_of_range("tuple");
    const string* s = values.at(index);
    return Utils::stod(*s);
}

//User must check if a value is NULL before trying to get the value
bool Tuple::getBool(unsigned int index) const {
    if(index < 0 || index >= values.size())
        throw out_of_range("tuple");
    return Utils::s2b(*values.at(index));
}

//------------------------------------------ SQLResultTable ------------------------------------------//

SQLResultTable::SQLResultTable(const PGresult *pGresult) : tuples(getTuplesFromPGresult(pGresult)),
                                                           columnNames(getColumnNamesFromPGresult(pGresult)),
                                                           printedTable(getPrintedTable(pGresult)) {}

vector<Tuple*> SQLResultTable::getTuplesFromPGresult(const PGresult *pgr) {
    unsigned long n = (unsigned long)Utils::max(PQntuples(pgr),0);
    unsigned long columns = (unsigned long)Utils::max(PQnfields(pgr),0);
    vector<Tuple *> ret;
    for (unsigned long i = 0; i < n; i++) {
        vector<string *> t;
        for (unsigned long j = 0; j < columns; j++)
        {
            if(PQgetisnull(pgr,i,j))
                t.push_back(NULL);
            else
                t.push_back(new string(PQgetvalue(pgr,i,j)));
        }
        ret.push_back(new Tuple(t));
    }
    return ret;
}

vector<std::string> SQLResultTable::getColumnNamesFromPGresult(const PGresult *pgr) {
    unsigned long columns = (unsigned long)Utils::max(PQnfields(pgr),0);
    vector<string> ret;
    for (unsigned long i = 0; i < columns; i++)
        ret.push_back(PQfname(pgr,i));
    return ret;
}

SQLResultTable::~SQLResultTable() {
    for(Tuple* t : tuples) {
        if(t)
            delete t;
    }
}

string SQLResultTable::getPrintedTable(const PGresult *pgr) {
    char *out;
    size_t size; //not important. contains the size of the out string

    FILE *temp = open_memstream(&out, &size);

    PQprintOpt opts = {0};
    opts.header = 1;    //don't show the number of rows
    opts.align = 1;
    opts.fieldSep = (char *) "|";

    PQprint(temp, pgr, &opts);

    fclose(temp);
    string ret(out);
    free(out);
    return ret;
}

SQLResultTable::SQLResultTable(SQLResultTable const &origin) : tuples(Utils::copyDynamicVector(origin.tuples)),
                                                               columnNames(origin.columnNames) {}

unsigned long SQLResultTable::getNumberOfTuples() const {
    return tuples.size();
    //auto err = PQresultStatus(pGresult);
    //return (err == PGRES_COMMAND_OK || err == PGRES_TUPLES_OK) ? PQntuples(pGresult) : 0;
}

unsigned long SQLResultTable::getNumberOfColumns() const {
    return columnNames.size();
    //auto err = PQresultStatus(pGresult);
    //return (err == PGRES_COMMAND_OK || err == PGRES_TUPLES_OK) ? PQnfields(pGresult) : 0;
}

string SQLResultTable::getColumnName(unsigned int n) const {
    return n >= getNumberOfColumns() ? NULL : columnNames.at(n);
}

const vector<string>& SQLResultTable::getColumnNames() const {
    return columnNames;
}

std::vector<Tuple> SQLResultTable::getTuples() const {
    vector<Tuple> ret;
    for (unsigned long i = 0; i < getNumberOfTuples(); i++) {
        ret.push_back(Tuple(*tuples[i]));
    }
    return ret;
}

bool SQLResultTable::isEmpty() const {
    return !getNumberOfTuples();
}

void SQLResultTable::print(ostream &outStream) const {
    outStream << printedTable;
}

//------------------------------------------ SQLResult ------------------------------------------//

SQLResult::SQLResult(PGresult * r) : cmdMessage(PQcmdStatus(r)),
                                     status(PQresultStatus(r)),
                                     errorMessage(PQresultErrorMessage(r)),
                                     resultTable(SQLResultTable(r)) {
    PQclear(r);
}

SQLResult::~SQLResult() {
    //PQclear(res);
}

SQLResult::SQLResult(SQLResult const& origin) : cmdMessage(origin.cmdMessage),
                                                status(origin.status),
                                                errorMessage(origin.errorMessage),
                                                resultTable(origin.resultTable) {}

std::string SQLResult::getCommandMessage() const {
    return cmdMessage;
}

std::string SQLResult::getErrorMessage() const {
    return errorMessage;
}

bool SQLResult::hasError() const {
    return errorMessage.size() > 0 || status == PGRES_BAD_RESPONSE || status == PGRES_NONFATAL_ERROR ||
           status == PGRES_FATAL_ERROR;
}

bool SQLResult::hasTableResult() const {
    return status == PGRES_TUPLES_OK;
}

const SQLResultTable& SQLResult::getResultTable() const {
    return resultTable;
}

//------------------------------------------ PreparedStatement ------------------------------------------//

PreparedStatement::PreparedStatement(string const &name, string const &declaration) : _temp((char **) malloc(
        Utils::getNumberOfArgs(
                declaration) *
        sizeof(char *))),
                                                                                      name(name),
                                                                                      declaration(declaration) {
    if (!S.getPreparedStatement(name)) {
        PGresult *res = PQprepare(S.conn, name.c_str(), declaration.c_str(), Utils::getNumberOfArgs(declaration), NULL);
        ExecStatusType r = PQresultStatus(res);
        if(r != PGRES_COMMAND_OK && r != PGRES_TUPLES_OK) {
            string err(PQresultErrorMessage(res));
            if(err.empty())
                err = string(PQerrorMessage(SQLServer::server().conn));
            PQclear(res);
            throw DBMSErrorCreatingPreparedStatement(err);
        }
        S.preparedStatements.insert(make_pair(name,this));
    }
}

SQLResult PreparedStatement::run(const std::vector<std::string> &args) const {
    const unsigned int n = Utils::getNumberOfArgs(declaration);
    cerr << args.size() << " " << n << endl;
    if(args.size() != n)
        throw DBMSErrorRunningPreparedStatement("Prepared statement's arguments don't match the number of arguments that the prepared statement should receive.");
    for (unsigned int i = 0; i < n; i++)
        _temp[i] = (char *) args[i].c_str();

    return SQLResult(
            PQexecPrepared(SQLServer::server().conn, name.c_str(), n, (const char *const *) _temp, NULL, NULL, 0));
}

PreparedStatement::~PreparedStatement() {
    if (_temp)
        free(_temp);
}

//------------------------------------------ SQLServer ------------------------------------------//

SQLServer::SQLServer() //construtor
{
    //start();
}

SQLServer::~SQLServer() //destrutor
{
    stop();
}

void SQLServer::start() {
    if (conn && PQstatus(conn) != CONNECTION_BAD)
        return;

    clog("Starting SQL server");
    stringstream temp;
    const Properties &defPro = Properties::getDefault();
    string host = defPro.getProperty("DB_host");
    string port = defPro.getProperty("DB_port");
    string username = defPro.getProperty("DB_user");
    string password = defPro.getProperty("DB_password");
    string dbname = defPro.getProperty("DB_name");
    temp << "host='" << host << "' port='" << port << "' user='" << username;
    temp << "' password='" << password << "' dbname='" << dbname << "'";
    conn = PQconnectdb(temp.str().c_str());

    if (!conn) {
        cerr << "Couldn't connect do the DB." << endl;
        clog("Couldn't connect do the DB");
        exit(-1);
    }

    if (PQstatus(conn) != CONNECTION_OK) {
        cerr << "Couldn't connect do the DB. Error: " << endl;
        cerr << PQerrorMessage(conn) << endl;
        clog("Couldn't connect do the DB");
        exit(-1);
    }
    // res=NULL;
    vcout << "DBMS successfully started." << endl;
    clog("DBMS successfully started");
}

void SQLServer::stop() {
    if (PQstatus(conn) != CONNECTION_BAD)
        PQfinish(conn);
    conn = NULL;
    for(auto ps : preparedStatements) {
        if(ps.second)
            delete(ps.second);
    }
    preparedStatements.clear();
    //clog("SQL server stopped");
}

PGresult *SQLServer::executeSQL(string const &sql) {
    PGresult *res = PQexec(conn, sql.c_str());
    if (!(PQresultStatus(res) == PGRES_COMMAND_OK || PQresultStatus(res) == PGRES_TUPLES_OK)) {
        string err(PQresultErrorMessage(res));
        if(err.empty())
            err = PQerrorMessage(S.conn);
        PQclear(res);
        throw DBMSError(err);
    }

    return res;
}

void SQLServer::printResult(PGresult *result, int client_socket, int hide_num_rows) {
    /*for (int row = 0; row < PQntuples(res); row++)
    cout << PQgetvalue(res, row, 0) << ' ' << PQgetvalue(res, row, 1) << endl;*/
    //PQdisplayTuples(result,stdout,1,"|",1,hide_num_rows);
    char *out;
    size_t size; //not important. contains the size of the out string
    FILE *temp = open_memstream(&out, &size);
    PQprintOpt opts = {0};

    opts.header = !hide_num_rows;
    opts.align = 1;
    opts.fieldSep = (char *) "|";

    PQprint(temp, result, &opts);
    fclose(temp);
    //Network::server().writeline(client_socket, "");
    //Network::server().writeline(client_socket, out, false);
    cout << PQerrorMessage(S.conn) << endl << out << endl;
    free(out);
}

void SQLServer::requestNewPreparedStatement(std::string const &name, std::string const &declaration) {
    if(getPreparedStatement(name))
        return;
    new PreparedStatement(name,declaration);    //the new is VERY important because it makes the this pointer (saved in the map of PreparedStatement*'s) be permanent until the delete instruction. The constructor automatically adds the object to the map.
    //cerr << PQerrorMessage(S.conn) << "\n";
}

const PreparedStatement *SQLServer::getPreparedStatement(std::string const &name) const {
    auto it = S.preparedStatements.find(name);
    if (it == S.preparedStatements.end())
        return NULL;
    return it->second;
}

//------------------------------------------ DBMS Errors ------------------------------------------//

DBMSError::DBMSError(const DBMSError &e) : whatMessage(e.whatMessage) {}

DBMSError::DBMSError(const std::string &s) : whatMessage(s) {}

DBMSError::~DBMSError() {}

DBMSErrorCreatingPreparedStatement::DBMSErrorCreatingPreparedStatement(const std::string & s) : DBMSError(s) {}

DBMSErrorRunningPreparedStatement::DBMSErrorRunningPreparedStatement(const std::string & s) : DBMSError(s) {}

const char* DBMSError::what() const throw() {
    return whatMessage.c_str();
}