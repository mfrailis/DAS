#ifndef DAS_EXCEPTIONS_HPP
#define DAS_EXCEPTIONS_HPP
#include <odb/exceptions.hxx>

namespace das
{
  typedef odb::object_not_persistent object_not_persistent;
  
  class wrong_database : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "wrong database";
    }
    
  }; //TODO

  class keyword_not_present : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "the keyword requested is not present";
    }
    
  }; //TODO
  class association_not_present : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "the association requested is not present";
    }
    
  }; //TODO
 class unknown_kewyword_type : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "unknown type";
    }
    
  }; //TODO

 class non_compatible_types : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "types non compatibles";
    }
    
  }; //TODO

 class bad_type : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "type non compatible with the expression";
    }
    
  }; //TODO

 class incomplete_statement : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "incomplete query expression";
    }
    
  }; //TODO

 class bad_ordering_clause : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "bad ordering clause";
    }
    
  }; //TODO
}

#endif
