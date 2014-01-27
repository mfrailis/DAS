
%{
#include "tpl/database.hpp"
%}

%import "transaction.i"

%include <std_string.i>

#define SWIG_SHARED_PTR_SUBNAMESPACE tr1
%include <std_shared_ptr.i>
%shared_ptr(das::tpl::Database)


%nodefaultctor das::tpl::Database;
%copyctor das::tpl::Database;
namespace das {
  namespace tpl {

    class Database {
    public:
      static
        std::tr1::shared_ptr<Database>
        create(const std::string& alias);
      
      Transaction
        begin();

      void
        flush();
      
      void
        begin_session();
      
      void
        end_session();

    };

  }
 }

%extend das::tpl::Database {
%pythoncode %{

    def _find_ddl_type(self, type_name):
      pydas_module = __import__('pydas')
      return getattr(pydas_module, type_name)

    def load(self, type_name, id):
      ddl_class = self._find_ddl_type(type_name)
      return ddl_class.load(self, id)

    def load(self, type_name, obj_name, obj_version = -1):
      ddl_class = self._find_ddl_type(type_name)
      return ddl_class.load(self, obj_name, obj_version)

    def persist(self, obj, path = ""):
      obj.persist(self, path)

    def query(self, type_name, query_expr, ordering = ""):
      ddl_class = self._find_ddl_type(type_name)
      return ddl_class.query(self, query_expr, ordering)

    def erase(self, obj):
      obj.erase(self)

    def attach(self, obj):
      obj.attach(self)

%}



};


