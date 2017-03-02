#include "../headers/Error.hpp"

SQL_Error::SQL_Error(PGresult* e) : err(e) {}

SQL_Error::SQL_Error(SQL_Error* e) : err(e->err) {}

SQL_Error::~SQL_Error()
{
	//PQclear(err);
}
