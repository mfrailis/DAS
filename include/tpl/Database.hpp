#ifndef DAS_DATABASE_HPP
#define DAS_DATABASE_HPP
/* WARNING: this class is non the database-indipendent interface
 * but the mysql relate one
 */
#include <iostream>
#include <memory>
#include <odb/tr1/memory.hxx>
#include <odb/traits.hxx>
#include <odb/transaction.hxx>
#include <odb/database.hxx>
#include <odb/traits.hxx>
#include <odb/session.hxx>

#include "../build/dbms/mysql/aux_query-odb.hxx"

#include "ddl/info.hpp"
#include "exceptions.hpp"
#include "../src/cpp/aux_query.hpp"
#include "../src/cpp/ql/qlvisitor.hpp"
#include "DasObject.hpp"
#include "transaction.hpp"
#include "interal/db_bundle.hpp"

#include <odb/mysql/database.hxx>
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;
/*
  template<class T>
  struct object_traits
  {
  static const std::string table_name;
  };
*/

namespace das
{
  namespace tpl
  {
    
    namespace mysql{class Database;}

    class Database
    {
    public:

      static
      shared_ptr<Database>
      create(const std::string& alias)
      {
        //TODO look for configured databases
        shared_ptr<Database> db(new Database());
        //TODO select proper database
        //db->db_.reset(new odb::mysql::database(user,password,database,host,port));
        if(alias == "local")
	  db->bundle_->db_.reset(new odb::mysql::database("odb_test","","odb_test"));
        else if(alias == "benchmark")
	  db->bundle_->db_.reset(new odb::mysql::database("odb_test","","benchmark"));
        else
	  throw das::wrong_database();
	db->info_ = DdlInfo::get_instance(alias);
        return db;
      }

      Transaction begin()
      {
        Transaction t(bundle_->db_->begin(),bundle_);
        return t;
      }

      template<typename T>
      typename odb::object_traits<T>::pointer_type
      load(const typename odb::object_traits<const T>::id_type& id)
      {
        odb::session::current(*(bundle_->session_));
        typename odb::object_traits<T>::pointer_type pobj;
        bool local_trans = false;
        odb::transaction *transaction = auto_begin(local_trans);
        try
	  {
            pobj = bundle_->db_->load<T>(id);
	  }
        catch(const std::exception &e){auto_catch(local_trans,transaction);}

        auto_commit(local_trans,transaction);

        // bind loaded object with this database
        pobj->bundle_ = bundle_;
        return pobj;
      }

      template<typename T>
      typename odb::object_traits<T>::pointer_type
      load(const std::string& name, int version = -1)
      {
        odb::session(*(bundle_->session_));
        typedef odb::query<T> query;
        typedef odb::result<T> result;

        bool local_trans = false;
        odb::transaction *transaction = auto_begin(local_trans);

        result *r;
        if(version != -1)
	  {
            r = new result (bundle_->db_->query<T> (query::name == query::_val(name) &&
					   query::version == query::_val(version)));
	  }
        else
	  {
            r = new result (bundle_->db_->query<T> ((query::name == query::_val(name)) + "ORDER BY version DESC",
					   false)); // do not cache result, we just  need the first row.
	  }

        typename odb::object_traits<T>::pointer_type pobj;
        typename result::iterator i (r->begin());
        if (i != r->end())
	  {
            try
	      {
                pobj = i.load();
	      }
            catch(const std::exception &e){auto_catch(local_trans,transaction);}
	  }
        else
	  {
            if(local_trans)
	      {
                transaction->rollback();
                delete transaction;
	      }
#ifdef VDBG
            std::cout << "DAS debug INFO: odb not persistent exception" << std::endl;
#endif
            throw object_not_persistent();
	  }

        delete r;

        auto_commit(local_trans,transaction);
        // bind loaded object with this database
        pobj->bundle_ = bundle_;
        return pobj;
      }


      template<typename T>
      typename odb::object_traits<T>::id_type
      persist(typename odb::object_traits<T>::pointer_type& obj, std::string path = "")
      {
        odb::session::current(*(bundle_->session_));
        //shared_ptr<Database> self = self_.lock(); //always succesful
        if (!obj->is_new())
	  {
            if (obj->bundle_ != bundle_)
	      {
                throw wrong_database();
	      }
            return obj->das_id_;
	  } //FIXME should we do an update insted?

        if (obj->bundle_)
	  {
            throw wrong_database();
	  }
#ifdef VDBG
	if(obj->version_ != 0 )std::cout << "DAS debug INFO: WARNING: changing version number while persisting obj " << obj->name_ <<std::endl; //DBG
#endif
	typedef odb::result<max_version> result;
	result r (bundle_->db_->query<max_version> ("SELECT MAX(version) FROM " + obj->type_name_ + " WHERE name = '"+obj->name_+"'"));
	result::iterator i(r.begin());

	if(i != r.end())
	  {
	    obj->version_ = i->version+1;
	  }
	else
	  {
	    obj->version_ = 1;
	  }

	obj->save_data(path);
	obj->persist_associated_pre(this);
#ifdef VDBG
	std::cout << "DAS debug INFO: PRS " << obj->name_ <<"... "; //DBG
#endif
	typename odb::object_traits<T>::id_type id = bundle_->db_->persist<T>(obj);
#ifdef VDBG
	std::cout << "done: id= " << id << std::endl;
#endif
	obj->persist_associated_post(this);
	obj->is_dirty_=false;

	// bind new object with this database
	obj->bundle_ = bundle_;
	obj->is_dirty_ = false;
	return id;
      }

      template<typename T>
      void erase(T obj);//TODO

      template<typename T>
      odb::result<T> query(const std::string& expression, const std::string& ordering = "")
      {
	odb::session::current(*(bundle_->session_));
	QLVisitor exp_visitor(das_traits<T>::name,info_);
	std::string clause = exp_visitor.parse_exp(expression);
	std::string order = exp_visitor.parse_ord(ordering);

#ifdef VDBG
	std::cout << "DAS debug INFO: WHERE\n" << clause+order <<std::endl; //DBG
#endif

	return bundle_->db_->query<T>(clause+order);
      }

      template<typename T>
      bool find(const std::string& name, int version = -1)
      {
	typedef odb::result<find_count> result;
	//FIXME odb::id_common may be a problem
	std::string table(odb::access::object_traits_impl< T, odb::id_common >::table_name);
	bool found;
	bool local_trans = false;
	odb::transaction *transaction = auto_begin(local_trans);
	try
	  {
	    result r(bundle_->db_->query<find_count> ("SELECT COUNT(*) FROM " + table + " WHERE name = '"+name+"'"));
	    result::iterator i(r.begin());
	    found = i->count != 0;
	  }
	catch(const std::exception &e){auto_catch(local_trans,transaction);}
	auto_commit(local_trans,transaction);
	return found;
      }

    protected:
      Database() :bundle_(new DbBundle())
      {
          bundle_->session_.reset(new odb::session(false)); 
      }
    private:
      inline odb::transaction* auto_begin(bool &is_auto)
      {
	odb::transaction *t;
	is_auto = !odb::transaction::has_current();
	if(is_auto)
	  {
	    t = new odb::transaction(bundle_->db_->begin());
	  }
	else
	  {
	    t = &odb::transaction::current();
	  }
	return t;
      }

      inline void auto_catch(bool is_auto, odb::transaction *t)
      {
	if(is_auto)
	  {
	    t->rollback();
	    delete t;
	  }
	throw;
      }

      inline void auto_commit(bool is_auto, odb::transaction *t)
      {
	if(is_auto)
	  {
	    t->commit();
	    delete t;
	  }
      }
      friend class Transaction;
//      shared_ptr<odb::mysql::database> db_;
//      weak_ptr<Database> self_;
      shared_ptr<DbBundle> bundle_;
      DdlInfo *info_;
//      odb::session session_;
    };


  }//namespace tpl
}//namespace das


#endif
