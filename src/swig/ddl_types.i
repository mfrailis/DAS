// First call swig with
// swig -Wall -python -c++ -I../../include/ -I/opt/boost_1.54/include pydas.i
// Then compile with
// g++ -c -g -fPIC -I../../include/ -I/opt/boost_1.54/include -I/usr/include/python2.7/ -I/opt/blitz/include/ pydas_wrap.cxx
// Then create the pydas dynamic library with
// g++ -shared pydas_wrap.o -o _pydas.so -L../../build/ -L/opt/odb-2.2.0/lib -ldas -lodb-mysql -lodb

%{
#define DAS_SWIG_BINDING
#include "ddl/types/ddl_measure.hpp"
#include "ddl/types/ddl_types.hpp"
#include "tpl/database.hpp"
%}

%import "das_object.i"
%import "database.i"
%import "result.i"

%include <std_string.i>

#define SWIG_SHARED_PTR_SUBNAMESPACE tr1
%include <std_shared_ptr.i>
%shared_ptr(measure)

class measure: public DasObject
{
 public:

  static  std::tr1::shared_ptr<measure> create(const std::string &name, const std::string &db_alias);

  const int&
  run_id () const;

  void
  run_id (const int &run_id);

  const std::string&
  obs_id () const;

  void
  obs_id (const std::string &obs_id);

  const long long&
  startdate () const;

  void
  startdate (const long long &startdate);

  const long long&
  enddate () const;

  void
  enddate (const long long &enddate);

 protected:

  measure ();

 private:

  measure (const std::string &name, const std::string &db_alias);

};

%template(Result_measure) das::tpl::Result<measure>;
%template(Result_measure_iterator) das::result_iterator_wrapper<measure>;

%extend measure {

  static 
    std::tr1::shared_ptr<measure>
    load(std::tr1::shared_ptr<das::tpl::Database> db, const long long &id) {
    return db->load<measure>(id);
  }

  static 
    std::tr1::shared_ptr<measure>
    load(std::tr1::shared_ptr<das::tpl::Database> db, std::string& name, int version = -1) {
    return db->load<measure>(name, version);
  }

  long long
    persist(std::tr1::shared_ptr<das::tpl::Database> db, std::string path = "") {
    return db->persist($self->get_shared_ptr(), path);
  }

  static 
    das::tpl::Result<measure>
    query(std::tr1::shared_ptr<das::tpl::Database> db, const std::string& expression, 
          const std::string& ordering = "") {
    return db->query<measure>(expression, ordering);
  }

  void
    erase(std::tr1::shared_ptr<das::tpl::Database> db) {
    db->erase($self->get_shared_ptr());
  }    

  void
    attach(std::tr1::shared_ptr<das::tpl::Database> db) {
    db->attach($self->get_shared_ptr());
  }

};



      
