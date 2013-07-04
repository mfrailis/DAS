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
  
   class no_credentials : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "wrong database";
    }
    
  }; //TODO
  
  class no_database : public std::exception
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
  
    class column_not_present : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "the column requested is not present";
    }
    
  }; //TODO
  
  class no_data_column : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "the column requested is not present";
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
  
  class not_in_managed_context : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "bad ordering clause";
    }
    
  }; //TODO
  
  class new_object : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "bad ordering clause";
    }
    
  }; //TODO  
  class object_is_corrupt : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "bad ordering clause";
    }
    
  }; //TODO  
  class object_not_unique : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "bad ordering clause";
    }
    
  }; //TODO  
  class bad_path : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "cannot access file";
    }
    
  }; //TODO
   class bad_param : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "wrong parameter";
    }
    
  }; //TODO
  
//IO exceptions
  class das_io_exception : public std::exception
  {
  public :
      das_io_exception(char *str):errstr_(str){}
    virtual const char*
    what() const throw()
    {
      return errstr_;
    }
  private:
      char * errstr_;
    
  }; //TODO 
  
  class already_in_session : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "another session is active onto this database";
    }
  private:
      char * errstr_;
    
  }; //TODO 
    class not_in_session : public std::exception
  {
  public :
    virtual const char*
    what() const throw()
    {
      return "no session or transaction active onto this database";
    }
  private:
      char * errstr_;
    
  }; //TODO 
  
}

#endif
