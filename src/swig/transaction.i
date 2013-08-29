
%{
#include "tpl/transaction.hpp"
%}

%nodefaultctor das::tpl::Transaction;
%copyctor das::tpl::Transaction;
%feature("valuewrapper") das::tpl::Transaction;
namespace das
{
namespace tpl
{
  class Transaction
  {
  public:
    void commit();
    void rollback();

  };  

}
}
