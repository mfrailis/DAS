#ifndef DAS_DATABASE_HPP
#define DAS_DATABASE_HPP

#include <iostream>
#include <memory> 
#include <odb/traits.hxx>
#include <odb/transaction.hxx>
#include <odb/database.hxx>
#include <odb/exceptions.hxx>

namespace das { namespace tpl
{

  class Database
  {
  public:

    odb::transaction_impl* begin();

    template<typename T>
    T* 
    load(const typename odb::object_traits<const T>::id_type& id)
    {
      T *pobj;
      odb::transaction *transaction;
      bool local_trans = !odb::transaction::has_current();
      if(local_trans)
      {
	  transaction = new odb::transaction(db->begin());
      }
      try{
	pobj = db->load<T>(id);
      }catch(std::exception& e){
	if(local_trans){
	  transaction->commit();
	  delete transaction;
	}
	throw e;
      }
      if(local_trans){
	transaction->commit();
	delete transaction;
      }
      pobj->database_ = db_id_;
      return pobj;
    }

    template<typename T>
    T* //TODO should we use odb::object_traits<T>::pointer_type?
    load(const std::string& name, int version = -1){
      typedef odb::query<T> query;
      typedef odb::result<T> result;
      result *r;
      T *pobj;
      odb::transaction *transaction;
      bool local_trans = !odb::transaction::has_current();
      if(local_trans)
	{
	  transaction = new odb::transaction(db->begin());
	}
      if(version != -1)
	{
	  r = new result (db->query<T> (query::name == query::_val(name) && 
					query::version == query::_val(version)));
	}
      else
	{
	  r = new result (db->query<T> ((query::name == query::_val(name)) + "ORDER BY version DESC",
					false)); // do not cache result, we need just the first row.
	}
      typename result::iterator i (r->begin());
      if (i != r->end())
	{
	  pobj = i.load();    
	}
      else
	{
	  if(local_trans)
	    {
	      transaction->commit();
	      delete transaction;
	    }
	  odb::object_not_persistent e;
	  throw e;
	}
      
      delete r;
      if(local_trans)
	{
	  transaction->commit();
	  delete transaction;
	}
      pobj->database_ = db_id_;
      return pobj;
    }
  


    template<typename T>
    void update(const T& obj);
    
    template<typename T>
    typename odb::object_traits<T>::id_type 
    persist(T obj, std::string path = "");
    
    template<typename T>
    void erase(T obj);
    /*
      template<typename T>
      result query(const std::string& expression);
    */
    template<typename T>   
    bool find(const std::string&, int version = -1);

  protected:
    std::auto_ptr<odb::database> db; // to be initialized by child class
    std::string db_id_; // to be initialized by child class
  };

  
  }}
#endif
