
%{
#include "transaction.hpp"
%}

%nodefaultctor das::Transaction;
%copyctor das::Transaction;
%feature("valuewrapper") das::Transaction;
namespace das
{

  class Transaction
  {
  public:
    void commit();
    void rollback();

  };  

}
