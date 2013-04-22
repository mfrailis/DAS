#ifndef DAS_DATABASE_HPP
#define DAS_DATABASE_HPP

#include <iostream>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <odb/traits.hxx>
#include <odb/transaction.hxx>
#include <odb/database.hxx>
#include <odb/traits.hxx>
#include "../exceptions.hpp"
#include "../aux_query.hpp"
#include "aux_query-odb.hxx"

#include <odb/mysql/database.hxx>

namespace das
{
namespace tpl
{
class Database
{
public:
    static
    boost::shared_ptr<Database>
    create(const std::string& alias)
    {
        // TODO look for configured databases
        boost::shared_ptr<Database> db(new Database());
        //TODO select proper database
        //db->db_.reset(new odb::mysql::database(user,password,database,host,port));
        db->db_.reset(new odb::mysql::database("odb_test","","prova"));
        db->self_ = db;
        return db;
    }
    odb::transaction_impl* begin()
    {
        return db_->begin();
    }

    template<typename T>
    T*
    load(const typename odb::object_traits<const T>::id_type& id)
    {
        T *pobj;
        odb::transaction *transaction;
        bool local_trans = !odb::transaction::has_current();
        if(local_trans)
        {
            transaction = new odb::transaction(db_->begin());
        }
        else
        {
            transaction = &odb::transaction::current();
        }
        try
        {
            pobj = db_->load<T>(id);
        }
        catch(std::exception& e)
        {
            if(local_trans)
            {
                transaction->commit();
                delete transaction;
            }
            throw e;
        }
        if(local_trans)
        {
            transaction->commit();
            delete transaction;
        }
        // bind loaded object with this database
        pobj->database_ = db_id_;
        return pobj;
    }

    template<typename T>
    T* //TODO should we use odb::object_traits<T>::pointer_type?
    load(const std::string& name, int version = -1)
    {
        typedef odb::query<T> query;
        typedef odb::result<T> result;

        odb::transaction *transaction;
        bool local_trans = !odb::transaction::has_current();
        if(local_trans)
        {
            transaction = new odb::transaction(db_->begin());
        }
        else
        {
            transaction = &odb::transaction::current();
        }

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

        T *pobj;
        typename result::iterator i (r->begin());
        if (i != r->end())
        {
            try
            {
            pobj = i.load();
            }
            catch(std::exception &e)
            {
                if(local_trans)
                {
                    transaction->rollback();
                    delete transaction;
                }
                throw e;
            }
        }
        else
        {
            if(local_trans)
            {
                transaction->rollback();
                delete transaction;
            }
            throw object_not_persistent();
        }

        delete r;
        if(local_trans)
        {
            transaction->commit();
            delete transaction;
        }
        // bind loaded object with this database
        pobj->database_ = db_id_;
        return pobj;
    }



    template<typename T>
    void update(T& obj,bool recursive = true)
    {
        boost::shared_ptr<Database> self = self_.lock(); //always succesful
#ifdef VDBG
        std::cout << " update "<< obj.name_<< std::endl;
#endif
        if (!obj.is_dirty() && !recursive)
        {
#ifdef VDBG
            std::cout << "      UPD: no ditry and no recursive: return" << std::endl;
#endif
            return;
        }
        if (obj.db_ptr_ != self)
        {
#ifdef VDBG
            std::cout << "DBG obj.database: " << /*obj.database_ <<"db: "<<db_id_<<*/ std::endl;
#endif
            throw wrong_database();
        }
        odb::transaction *transaction;
        bool local_trans = !odb::transaction::has_current();
        if(local_trans)
        {
            transaction = new odb::transaction(db_->begin());
        }
        else
        {
            transaction = &odb::transaction::current();
        }
        try
        {

            if(recursive)
            {
                obj.update_associated(this);
            }
            obj.save_data(); //FIXME chek if is dirty?
            if(obj.is_dirty_)
            {
#ifdef VDBG
                std::cout << "UPD: updating metadata " << obj.name_ << std::endl;
#endif
                db_->update(obj);
            }
#ifdef VDBG
            if(!obj.is_dirty_)
            {
                std::cout << "UPD: obj "<< obj.name_ <<" up to date" << std::endl;
            }
#endif
        }
        catch(std::exception &e)
        {
            if(local_trans)
            {
                transaction->rollback();
                delete transaction;
            }
            throw e;
        }
        if(local_trans)
        {
            transaction->commit();
            delete transaction;
        }
    }

    template<typename T>
    typename odb::object_traits<T>::id_type
    persist(T& obj, std::string path = "")
    {
        boost::shared_ptr<Database> self = self_.lock(); //always succesful
#ifdef VDBG
        std::cout << " persist "<< obj.name_<< std::endl;
#endif
        if (!obj.is_new())
        {
            if (obj.db_ptr_ != self)
            {
#ifdef VDBG
                std::cout << "DBG obj.database: " /*<< obj.database_ <<"db: "<<db_id_*/<< std::endl;
#endif
                throw wrong_database();
            }
            return obj.das_id_;
        } //FIXME should we do an update insted?

        if (obj.db_ptr_.get() != 0)
        {
#ifdef VDBG
            std::cout << "DBG obj.database: " /*<< obj.database_ <<" expected: none"*/<< std::endl;
#endif
            throw wrong_database();
        }
        odb::transaction *transaction;
        bool local_trans = !odb::transaction::has_current();
        if(local_trans)
        {
#ifdef VDBG
            std::cout << "BEGIN local transaction" << std::endl; //DBG
#endif
            transaction = new odb::transaction(db_->begin());
        }
        else
        {
            transaction = &odb::transaction::current();
        }
        typename odb::object_traits<T>::id_type id;

        try
        {
#ifdef VDBG
            if(obj.version_ != 0 )std::cout << "WARNING: changing version number while persisting obj " << obj.name_ <<std::endl; //DBG
#endif
            typedef odb::result<max_version> result;
            result r (db_->query<max_version> ("SELECT MAX(version) FROM " + obj.type_name_ + " WHERE name = '"+obj.name_+"'"));
            result::iterator i(r.begin());

            if(i != r.end())
            {
                obj.version_ = i->version+1;
#ifdef VDBG
                std::cout << "FOUND VERISON " << i->version <<std::endl; //DBG
#endif
            }
            else
            {
                obj.version_ = 1; //FIXME should we start from 0?
            }

            obj.save_data(path);
            obj.persist_associated(this);
#ifdef VDBG
            std::cout << "PRS " << obj.name_ <<std::endl; //DBG
#endif
            id = db_->persist<T>(obj);
        }
        catch(std::exception &e)
        {
            if(local_trans)
            {
#ifdef VDBG
                std::cout << "ROLLBACK local transaction" << std::endl; //DBG
#endif
                transaction->rollback();
                delete transaction;
            }
            throw e;
        }
        if(local_trans)
        {
#ifdef VDBG
            std::cout << "COMMIT local transaction" << std::endl; //DBG
#endif
            transaction->commit();
            delete transaction;
        }
        // bind new object with this database
        obj.db_ptr_ = self;
        obj.is_dirty_ = false;
        return id;
    }

    template<typename T>
    void erase(T obj);
    /*
      template<typename T>
      result query(const std::string& expression);
    */
    template<typename T>
    bool find(const std::string&, int version = -1);

private:
    Database(){}
    boost::shared_ptr<odb::database> db_; // to be initialized by child class
    boost::weak_ptr<Database> self_;
/*COMMENTAMI!!!*/    std::string db_id_; // to be initialized by child class
};


}
}


#endif
