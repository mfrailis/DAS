
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

