#ifndef ERROR_HPP
#define ERROR_HPP 1

#include <string>
#include <iostream>

#include "DBMS.hpp"
#include "Log.hpp"
#include "Client.hpp"
#include <postgresql/libpq-fe.h>

using namespace std;

//--------------------------EXCEPTION HANDLING----------------------//

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