#include "../headers/DBMS.hpp"
#include "../headers/Properties.hpp"
#include "../headers/Log.hpp"
#include "../headers/Client.hpp"
#include "../headers/Sockets.hpp"
#include "../headers/Utils.hpp"
#include <sstream>
#include <stdarg.h>

#ifndef vcout //verbose cout.
#define vcout if(false) cout
#endif

using namespace std;

//------------------------------------------ SQLResultTable::Tuple ------------------------------------------//

Tuple::Tuple(/*SQLResultTable* parent, */const vector<string> &v) : /*table(parent),*/ values(v) {
    //values.insert(values.begin(),v.begin(),v.end());
}

Tuple::Tuple(const Tuple & o) : /*table(o.table), */values(o.values) {}

const std::vector<std::string>& Tuple::getValues() const {
    return values;
}

string Tuple::getString(unsigned int index) const {
    if(index < 0 || index >= values.size())
        throw out_of_range("tuple");
    return values.at(index);
}

int Tuple::getInt(unsigned int index) const {
    if(index < 0 || index >= values.size())
        throw out_of_range("tuple");
    const string& s = values.at(index);
    if(!Utils::isInt(s))
        throw logic_error("Tried to get int but it's not int.");
    return atoi(s.c_str());
}

double Tuple::getDouble(unsigned int index) const {
    if(index < 0 || index >= values.size())
        throw out_of_range("tuple");
    const string& s = values.at(index);
    return Utils::atod(s);
}

bool Tuple::getBool(unsigned int index) const {
    if(index < 0 || index >= values.size())
        throw out_of_range("tuple");
    return Utils::s2b(values.at(index));
}

//------------------------------------------ SQLResultTable ------------------------------------------//

SQLResultTable::SQLResultTable(PGresult* pGresult) : pGresult(pGresult) {

}

SQLResultTable::~SQLResultTable() {
    for(Tuple* t : tuples) {
        if(t)
            delete t;
    }
    //PQclear(pGresult);    //SQLResultTable should NOT clear the PGresult because it's not its owner. The responsible party for clearing the pointer is SQLResult!
}

const std::vector<Tuple*> SQLResultTable::getTuples() const {
    return tuples;
}

int SQLResultTable::getNumberOfTuples() const {
    auto err = PQresultStatus(pGresult);
    return (err == PGRES_COMMAND_OK || err == PGRES_TUPLES_OK) ? PQntuples(pGresult) : 0;
}

int SQLResultTable::getNumberOfColumns() const {
    auto err = PQresultStatus(pGresult);
    return (err == PGRES_COMMAND_OK || err == PGRES_TUPLES_OK) ? PQnfields(pGresult) : 0;
}

const char* SQLResultTable::getColumnName(unsigned int n) const {
    return PQfname(pGresult,n);
}

std::vector<std::string> SQLResultTable::getColumnNames() const {
    vector<string> ret;
    for(unsigned int i = 0 ; i<getNumberOfColumns() ; i++) {
        ret.push_back(getColumnName(i));
    }
    return ret;
}

bool SQLResultTable::isEmpty() const {
    return !getNumberOfTuples();
}

void SQLResultTable::print(const int client_socket, const bool hide_num_rows = true) const {
    char *out;
    size_t size; //not important. contains the size of the out string
    FILE *temp = open_memstream(&out, &size);
    PQprintOpt opts = {0};

    opts.header = !hide_num_rows;
    opts.align = 1;
    opts.fieldSep = (char *) "|";


    PQprint(temp, pGresult, &opts);
    fclose(temp);
    Network::server().writeline(client_socket, "");
    Network::server().writeline(client_socket, out, false);
    free(out);
}

void SQLResultTable::print(std::ostream &outStream, const bool hide_num_rows) const {
    char *out;
    size_t size; //not important. contains the size of the out string
    FILE *temp = open_memstream(&out, &size);
    PQprintOpt opts = {0};

    opts.header = !hide_num_rows;
    opts.align = 1;
    opts.fieldSep = (char *) "|";


    PQprint(temp, pGresult, &opts);

    outStream << endl << out;

    fclose(temp);
    free(out);
}

//------------------------------------------ SQLResult ------------------------------------------//

SQLResult::SQLResult(PGresult * r) : res(r), isCopy(false) {}

SQLResult::~SQLResult() {
    PQclear(res);
}

SQLResult::SQLResult(SQLResult const& origin) : res(origin.res), isCopy(true) {

}

//------------------------------------------ PreparedStatement ------------------------------------------//

PreparedStatement::PreparedStatement(string const &name, string const &declaration) : name(name),
                                                                                      declaration(declaration),
                                 argsConcat((char**)malloc(Utils::getNumberOfArgs(declaration)*sizeof(char*))) {
    if (!S.getPreparedStatement(name)) {
        PQprepare(S.conn, name.c_str(), declaration.c_str(), Utils::getNumberOfArgs(declaration), NULL);
        S.preparedStatements.insert(pair<string, PreparedStatement *>(name, this));
    }
}

SQLResult PreparedStatement::run(const std::vector<std::string> &args) const {
    const int n = Utils::getNumberOfArgs(declaration);
    if(args.size() != n)
        throw SQL_Error("Prepared statement's arguments don't match the number of arguments that the prepared statement should receive.");
    for(int i = 0 ; i < n ; i++)
        argsConcat[i] = (char*)args[i].c_str();

    return SQLResult( PQexecPrepared(SQLServer::server().conn,name.c_str(),n,(const char* const*)argsConcat,NULL,NULL,0) );
}

PreparedStatement::~PreparedStatement() {
    free(argsConcat);
}

//------------------------------------------ SQLServer ------------------------------------------//

SQLServer::SQLServer() //construtor
{
    //start();
}

SQLServer::~SQLServer() //destrutor
{
    for (pair<const string, PreparedStatement const *> i : preparedStatements)
        if (i.second)
            delete i.second;
    stop();
}

void SQLServer::start() {
    if (PQstatus(conn) == CONNECTION_OK)
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
    if (PQstatus(conn) == CONNECTION_OK)
        PQfinish(conn);
    clog("SQL server stopped");
}

PGresult *SQLServer::executeSQL(string const &sql) {
    PGresult *res = PQexec(conn, sql.c_str());
    if (!(PQresultStatus(res) == PGRES_COMMAND_OK || PQresultStatus(res) == PGRES_TUPLES_OK))
        throw SQL_Error(res);

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
    cout << "printing: " << endl << PQerrorMessage(S.conn) << out << endl;
    free(out);
}

void SQLServer::requestNewPreparedStatement(std::string const &name, std::string const &declaration) {
    if(getPreparedStatement(name))
        return;
    new PreparedStatement(name,declaration);    //the new is VERY important because it makes the this pointer (saved in the map of PreparedStatement*'s) be permanent until the delete instruction. The constructor automatically adds the object to the map.
    cerr << PQerrorMessage(S.conn) << "\n";
}

const PreparedStatement *SQLServer::getPreparedStatement(std::string const &name) const {
    auto it = S.preparedStatements.find(name);
    if (it == S.preparedStatements.end())
        return NULL;
    return it->second;
}

SQL_Error::SQL_Error(PGresult *e) : err(e) {}

SQL_Error::SQL_Error(const SQL_Error *e) : err(e->err) {}

SQL_Error::SQL_Error(const SQL_Error &e) : err(e.err) {}

SQL_Error::SQL_Error(const std::string &) {}

SQL_Error::~SQL_Error() {
    //PQclear(err);
}

TupleConversionError::TupleConversionError(const std::string &err) : logic_error(err) {}