#include "../headers/DBMS.hpp"
#include "../headers/Properties.hpp"
#include <sstream>

#ifndef vcout //verbose cout.
#define vcout if(false) cout
#endif

string b2s(bool b)
{
  string s(b? "true" : "false");
  return s;
}

bool s2b(string s)
{ //returns true if s is "true" or "t". this function IS NOT case sensitive
  transform(s.begin(),s.end(),s.begin(),::tolower); //tolower every char in the string
  if(s=="true"||s=="t")
    return true;
  return false;
}

SQL_server_class::SQL_server_class() //construtor
{
  SQL_server_start();
}

SQL_server_class::~SQL_server_class() //destrutor
{
  SQL_server_stop();
}

void SQL_server_class::SQL_server_start()
{
  if(PQstatus(conn)==CONNECTION_OK)
    return;

  clog( "Starting SQL server" );
  stringstream temp;
  //conn = PQconnectdb("host='dbm.fe.up.pt' user='sinf16g61' password='qwerty07feup' dbname='sinf16g61'");
  const Properties& defPro = Properties::getDefault();
  string host = defPro.getProperty("DB_host");
  string port = defPro.getProperty("DB_port");
  string username = defPro.getProperty("DB_user");
  string password = defPro.getProperty("DB_password");
  string dbname = defPro.getProperty("DB_name");
  temp << "host='" << host << "' port='" << port << "' user='" << username;
  temp << "' password='" << password << "' dbname='" << dbname << "'";
  conn = PQconnectdb(temp.str().c_str());

  if (!conn)
  {
    cerr << "Couldn't connect do the DB." << endl;
    clog( "Couldn't connect do the DB" );
    exit(-1);
  }
 
  if (PQstatus(conn) != CONNECTION_OK)
  {
    cerr << "Couldn't connect do the DB. Error: " << endl;
    cerr << PQerrorMessage(conn) << endl;
    clog( "Couldn't connect do the DB" );
    exit(-1);
  }
  // res=NULL;
  vcout << "DB successfully created." << endl;
  clog( "DB successfully created" );
}

void SQL_server_class::SQL_server_stop()
{
  //cout << "SQL before: " << PQstatus(conn);
  if(PQstatus(conn)==CONNECTION_OK)
    PQfinish(conn);
  clog("SQL server stopped");
  //cout << "\t\tSQL after: " << PQstatus(conn) << endl;
}

PGresult* SQL_server_class::executeSQL(string sql)
{
  PGresult* res = PQexec(conn, sql.c_str());
  if(!(PQresultStatus(res) == PGRES_COMMAND_OK || PQresultStatus(res) == PGRES_TUPLES_OK))
    throw SQL_Error(res);
 
  return res;
}

void SQL_server_class::print_result(PGresult *result,int client_socket,int hide_num_rows)
{
	/*for (int row = 0; row < PQntuples(res); row++)
	cout << PQgetvalue(res, row, 0) << ' ' << PQgetvalue(res, row, 1) << endl;*/
	//PQdisplayTuples(result,stdout,1,"|",1,hide_num_rows);
  char* out;
  size_t size; //not important. contains the size of the out string
  FILE* temp=open_memstream(&out,&size);
  PQprintOpt opts={0};

  opts.header=!hide_num_rows;
  opts.align=1;
  opts.fieldSep=(char*)"|";


  PQprint(temp,result,&opts);
  fclose(temp);
  N.writeline(client_socket,"");
  N.writeline(client_socket,out,false);
  free(out);
}