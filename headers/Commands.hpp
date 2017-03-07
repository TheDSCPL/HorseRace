#include <vector>
#include <string>

typedef enum {_int_,_bool_,_string_,_double_,_v_int_} arg_types;

class arg  //funciona
{
  public:
    //public variables
    const arg_types arg_type; //int=0 bool=1 std::string=2 double=3
  
  private:
    //private variables
    int i;
    bool b;
    std::string s;
    double d;
    std::vector<int> v;

  public:
    //constructors
    arg(int in)          : arg_type(_int_)     , i(in) {}
    arg(bool bo)         : arg_type(_bool_)    , b(bo) {}
    arg(std::string st)       : arg_type(_string_)  , s(st) {}
    arg(double db)       : arg_type(_double_)  , d(db) {}
    arg(std::vector<int> vi)  : arg_type(_v_int_)   , v(vi) {}

    bool operator=(std::string st)
    {
      return this->set_string(st);
    }
    bool operator=(bool bo)
    {
      return this->set_bool(bo);
    }
    bool operator=(int in)
    {
      return this->set_int(in);
    }
    bool operator=(double db)
    {
      return this->set_double(db);
    }
    bool operator=(std::vector<int> vi)
    {
      return this->set_v_int(vi);
    }

    bool set_int(int in);
    bool set_bool(bool bo);
    bool set_string(std::string st);
    bool set_double(double db);
    bool set_v_int(std::vector<int> vi);
    //public functions
    int arg_int();
    std::string arg_string();
    bool arg_bool();
    double arg_double();
    std::vector<int> arg_v_int();
};