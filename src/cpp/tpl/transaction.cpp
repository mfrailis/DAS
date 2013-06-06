#include "tpl/transaction.hpp"
#include "tpl/Database.hpp"
#include "../../../build/dbms/mysql/DasObject-odb.hxx"

using namespace das::tpl;

Transaction::Transaction(const DbBundle &bundle)
  : w_bundle_(bundle)
{
  transaction_.reset(new odb::transaction(bundle.db()->begin())); 
}

void
Transaction::commit()
{
    typedef odb::session::database_map database_map;
    typedef odb::session::type_map type_map;
    typedef odb::session::object_map<DasObject> object_map;
    
    if(w_bundle_.expired())
        throw not_in_managed_context();
    
    shared_ptr<odb::session> session_ = w_bundle_.session();
    shared_ptr<odb::database> db_ = w_bundle_.db(); 
    
  database_map::iterator db_it (session_->map().find (db_.get()));
         
  if (db_it == session_->map().end ())
    {
#ifdef VDBG
      std::cout << "DAS debug INFO: session empty" << std::endl;
 #endif     
      return;
    }
  for(type_map::iterator type_it = db_it->second.begin();
      type_it != db_it->second.end();
      type_it++)
    {
      object_map& obj_map (static_cast<object_map&> (*type_it->second));
      for(typename object_map::iterator obj_it = obj_map.begin();
	  obj_it != obj_map.end();
	  obj_it++)
	{
#ifdef VDBG
	  std::cout << "DAS debug INFO: UPD " << obj_it->second->name() <<std::endl; //DBG
#endif	  
	  obj_it->second->update();
#ifdef VDBG
	  std::cout << "DAS debug INFO: UPD " << obj_it->second->name() <<" DONE"<<std::endl; //DBG
#endif

	}
                 
    }
  transaction_->commit();
}
        
