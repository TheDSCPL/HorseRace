#include "../headers/Commands.hpp"

using namespace std;

int arg::arg_int()
{
	if(arg_type==_int_)
		return i;
	return -100;
}
string arg::arg_string()
{
	if(arg_type==_string_)
		return s;
	return "";
}
bool arg::arg_bool()
{
	if(arg_type==_bool_)
		return b;
	return false;
}
double arg::arg_double()
{
	if(arg_type==_double_)
		return d;
	return d=-100.001;
}
vector<int> arg::arg_v_int()
{
	if(arg_type==_v_int_)
		return v;
	return vector<int>{};
}

bool arg::set_int(int in)
{
	if(arg_type!=_int_)
		return false;
	i=in;
	return true;
}
bool arg::set_bool(bool bo)
{
	if(arg_type!=_bool_)
		return false;
	b=bo;
	return true;
}
bool arg::set_string(string st)
{
	if(arg_type!=_string_)
		return false;
	s=st;
	return true;
}
bool arg::set_double(double db)
{
	if(arg_type!=_double_)
		return false;
	d=db;
	return true;
}
bool arg::set_v_int(vector<int> v_i)
{
	if(arg_type!=_v_int_)
		return false;
	v=v_i;
	return true;
}

//https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_codes	//http://www.termsys.demon.co.uk/vtansi.htm