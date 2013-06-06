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

#include "../build/dbms/mysql/aux_query-odb.hxx"

#include "ddl/info.hpp"
#include "exceptions.hpp"
#include "../src/cpp/aux_query.hpp"
#include "../src/cpp/ql/qlvisitor.hpp"
#include "DasObject.hpp"

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
            db->db_.reset(new odb::mysql::database("odb_test","","odb_test"));
        else if(alias == "benchmark")
            db->db_.reset(new odb::mysql::database("odb_test","","benchmark"));
        else
            throw das::wrong_database();
        db->self_ = db;
	db->info_ = DdlInfo::get_instance(alias);
        return db;
    }

    odb::transaction_impl* begin()
    {
        return db_->begin();
    }

    template<typename T>
    typename odb::object_traits<T>::pointer_type
    load(const typename odb::object_traits<const T>::id_type& id)
    {
        typename odb::object_traits<T>::pointer_type pobj;
        bool local_trans = false;
        odb::transaction *transaction = auto_begin(local_trans);
        try
        {
            pobj = db_->load<T>(id);
        }
        catch(const std::exception &e){auto_catch(local_trans,transaction);}

        auto_commit(local_trans,transaction);

        // bind loaded object with this database
        pobj->db_ptr_ = self_.lock();
        return pobj;
    }

    template<typename T>
    typename odb::object_traits<T>::pointer_type
    load(const std::string& name, int version = -1)
    {
        typedef odb::query<T> query;
        typedef odb::result<T> result;

        bool local_trans = false;
        odb::transaction *transaction = auto_begin(local_trans);

        result *r;
        if(version != -1)
        {
            r = new result (db_->query<T> (query::name == query::_val(name) &&
                                          query::version == query::_val(version)));
        }
        else
        {
            r = new result (db_->query<T> ((query::name == query::_val(name)) + "ORDER BY version DESC",
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
        pobj->db_ptr_ = self_.lock();
        return pobj;
    }



    template<typename T>
    void update(typename odb::object_traits<T>::pointer_type& obj,bool recursive = true)
    {
        update<T>(*obj,recursive);
    }

    template<typename T>
    void update(T& obj,bool recursive = true)
    {
        shared_ptr<Database> self = self_.lock(); //always succesful

        if (!obj.is_dirty() && !recursive)
        {
#ifdef VDBG
            std::cout << "DAS debug INFO UPD: no ditry and no recursive: return" << std::endl;
#endif
            return;
        }
        if (obj.db_ptr_ != self)
        {
            throw wrong_database();
        }

        bool local_trans = false;
        odb::transaction *transaction = auto_begin(local_trans);
        try
        {
            if(recursive)
            {
                obj.update_associated();
            }
            obj.save_data(); //FIXME chek if is dirty?
            if(obj.is_dirty_)
            {
#ifdef VDBG
                std::cout << "DAS debug INFO UPD: updating   " <<obj.das_id_ <<" "<< obj.name_ << std::endl;
#endif
                db_->update<T>(obj);
            }
#ifdef VDBG
            if(!obj.is_dirty_)
            {
                std::cout << "DAS debug INFO UPD: up to date " <<obj.das_id_ <<" "<< obj.name_ << std::endl;
            }
#endif
	    obj.is_dirty_ = false;
        }
        catch(const std::exception &e){auto_catch(local_trans,transaction);}

        auto_commit(local_trans,transaction);
    }    
    
    template<typename T>
    typename odb::object_traits<T>::id_type
    persist(typename odb::object_traits<T>::pointer_type& obj, std::string path = "")
    {
        shared_ptr<Database> self = self_.lock(); //always succesful
        if (!obj->is_new())
        {
            if (obj->db_ptr_ != self)
            {
                throw wrong_database();
            }
            return obj->das_id_;
        } //FIXME should we do an update insted?

        if (obj->db_ptr_.get() != 0)
        {
            throw wrong_database();
        }
        bool local_trans = false;
        odb::transaction *transaction = auto_begin(local_trans);
        typename odb::object_traits<T>::id_type id;

        try
        {
#ifdef VDBG
            if(obj->version_ != 0 )std::cout << "DAS debug INFO: WARNING: changing version number while persisting obj " << obj->name_ <<std::endl; //DBG
#endif
            typedef odb::result<max_version> result;
            result r (db_->query<max_version> ("SELECT MAX(version) FROM " + obj->type_name_ + " WHERE name = '"+obj->name_+"'"));
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
            std::cout << "DAS debug INFO: PRS " << obj->name_ <<std::endl; //DBG
#endif
            id = db_->persist<T>(obj);
            obj->persist_associated_post(this);
	    obj->is_dirty_=false;
        }
        catch(const std::exception &e){auto_catch(local_trans,transaction);}

        auto_commit(local_trans,transaction);

        // bind new object with this database
        obj->db_ptr_ = self;
        obj->is_dirty_ = false;
        return id;
    }

    template<typename T>
    void erase(T obj);//TODO

    template<typename T>
    odb::result<T> query(const std::string& expression, const std::string& ordering = "")
    {
      QLVisitor exp_visitor(das_traits<T>::name,info_);
      std::string clause = exp_visitor.parse_exp(expression);
      std::string order = exp_visitor.parse_ord(ordering);

#ifdef VDBG
      std::cout << "DAS debug INFO: WHERE " << clause+order <<std::endl; //DBG
#endif

      return db_->query<T>(clause+order);
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
            result r(db_->query<find_count> ("SELECT COUNT(*) FROM " + table + " WHERE name = '"+name+"'"));
            result::iterator i(r.begin());
            found = i->count != 0;
        }
        catch(const std::exception &e){auto_catch(local_trans,transaction);}
        auto_commit(local_trans,transaction);
        return found;
    }

protected:
    Database(){}
private:
    inline odb::transaction* auto_begin(bool &is_auto)
    {
        odb::transaction *t;
        is_auto = !odb::transaction::has_current();
        if(is_auto)
        {
            t = new odb::transaction(db_->begin());
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

  shared_ptr<odb::mysql::database> db_;
    weak_ptr<Database> self_;
    DdlInfo *info_;
};


}//namespace tpl
}//namespace das


#endif
