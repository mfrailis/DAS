#ifndef DAS_DASOBJECT_HPP
#define DAS_DASOBJECT_HPP

#include <odb/core.hxx>
#include <string>
#include "ddl/info.hpp"
#include "ddl/column.hpp"
//#include "tpl/Database.hpp"
//template <typename T>
//class DasVector;
#include <odb/database.hxx>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>
#include "internal/db_bundle.hpp"
using std::tr1::shared_ptr;

class QLVisitor;
namespace das{namespace tpl{
    class Database;
    class Transaction;
    template<typename T>
    class result_iterator;
}}

#pragma db object abstract
class DasObject
{
public:
  const KeywordInfo&
  get_keyword_info (std::string keyword_name) throw(std::out_of_range)
  {
    return DdlInfo::get_instance()->
      get_keyword_info(type_name_, keyword_name);
  }

  const ColumnInfo&
  get_column_info (std::string column_name) throw(std::out_of_range)
  {
    return DdlInfo::get_instance()->
      get_column_info(type_name_, column_name);
  }

  bool is_dirty() const{return is_dirty_;}
  bool is_new()   const{return das_id_ == 0;  }

  const std::string&
  dbUserId () const
  {
    return dbUserId_;
  }

  const long long&
  creationDate () const
  {
    return creationDate_;
  }

  void
  creationDate (long long &creationDate)
  {
    creationDate_ = creationDate;
    is_dirty_ = true;
  }

  const short&
  version () const
  {
    return version_;
  }

  const std::string&
  name () const
  {
    return name_;
  }


protected:
  DasObject()
  {
    type_name_ = "DasObject";
    version_ = 0;
    das_id_ = 0;
    is_dirty_ = false;
  }

#pragma db transient
  std::string type_name_;

#pragma db transient
  das::tpl::WeakDbBundle bundle_;

#pragma db transient
  bool is_dirty_;                                   // does it need an update?

// polimorphic internal persistent interface
  
  virtual void save_data(){}                                // update external data.
  virtual void save_data(const std::string &path,das::tpl::DbBundle &db){}                // save external data, check if the path is empty.
  virtual void update(){}  // update self and associated if necessary
  // we need a database pointer because this object is not bound to any db yet
  virtual void persist_associated_pre (das::tpl::DbBundle &db){} // call persist on shared many associated objects
  virtual void persist_associated_post(das::tpl::DbBundle &db){} // call persist on exclusive and oneassociated objects
  virtual void set_dirty_columns(){} //set all entries on odb::vector columns as dirty in order to force an update

// external data interface
  virtual ColumnFromFile* column_from_file(const std::string &col_name){throw das::no_external_data();}
  virtual void column_from_file(const std::string &col_name, const ColumnFromFile &cf){throw das::no_external_data();}
 
//  virtual ImageFromFile* image_from_file(){throw das::no_external_data();}
//  virtual void image_from_file(const ImageFromFile &cf){throw das::no_external_data();}
  
private:
  friend class odb::access;
  friend class das::tpl::Database;
  friend class das::tpl::Transaction;
  friend class das::tpl::DbBundle;
  template <typename T> friend class das::tpl::result_iterator;
  friend class QLVisitor;
//  template <typename T> friend class DasVector;
//  template <typename T> friend void ::swap(DasVector<T> &x, DasVector<T> &y);
#pragma db id auto
  long long das_id_;

#pragma db type("VARCHAR(256)")
  std::string dbUserId_;
  long long creationDate_;

protected:
  short version_;
#pragma db type("VARCHAR(256)")
#pragma db index
  std::string name_;
  
};

template<class T>
struct das_traits{};

#endif
