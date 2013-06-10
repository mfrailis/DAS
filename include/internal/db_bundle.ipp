#ifndef DB_BUNDLE_IPP
#define	DB_BUNDLE_IPP
#include "db_bundle.hpp"
#include "../../src/cpp/aux_query.hpp"
#include "../../build/dbms/mysql/aux_query-odb.hxx"

namespace das {

    namespace tpl {

        template<typename T>
        inline
        void
        DbBundle::attach(const shared_ptr<T> &obj) {
            if (!valid()) {
#ifdef VDBG
                std::cout << "DAS info: pointers to db or session not valid" << std::endl;
#endif               
                throw das::not_in_managed_context();
            }
            if (obj->is_new()) {
#ifdef VDBG
                std::cout << "DAS info: trying to attach an object without persisting firts" << std::endl;
#endif
                throw das::new_object();
            }
            if (!obj->bundle_.expired()) {
                if (obj->bundle_ != *this) {
#ifdef VDBG
                    std::cout << "DAS info: ERROR: trying to attach an object managed by other database" << std::endl;
#endif
                    throw das::wrong_database();
                } else
                    return;
            }

            odb::session::current(*session_);
            shared_ptr<T> cache_hit = session_->cache_find<T>(*db_, obj->das_id_);
            if (cache_hit) {
                if (cache_hit != obj) {
#ifdef VDBG
                    std::cout << "DAS info: ERROR: another copy of this object found in cache" << std::endl;
#endif              
                    throw das::object_not_unique();
                } else {
#ifdef VDBG
                    std::cout << "DAS info: object found in the cache but not bound to the database" << std::endl;
#endif              
                    cache_hit->bundle_ = *this;
                }
            } else {
                session_->cache_insert<T>(*db_, obj->das_id_, obj);
                obj->bundle_ = *this;
            }

        }

        template<typename T>
        long long
        DbBundle::persist(const shared_ptr<T> &obj, std::string path) {
            odb::session::current(*session_);
            //shared_ptr<Database> self = self_.lock(); //always succesful
            if (!obj->is_new()) {
                if (operator !=(obj->bundle_)) // another db intance owns this obj
                {
                    throw wrong_database();
                }
                return obj->das_id_;
            } //FIXME should we do an update insted?

            if (!obj->bundle_.blank()) // shold never happen
            {
#ifdef VDBG
                std::cout << "DAS debug INFO: ERROR: obj '" << obj->name_ << "' is new and his bundle isn't blank" << std::endl;
#endif            
                throw wrong_database();
            }
#ifdef VDBG
            if (obj->version_ != 0)std::cout << "DAS debug INFO: WARNING: changing version number while persisting obj " << obj->name_ << std::endl; //DBG
#endif
            typedef odb::result<max_version> result;
            result r(db_->query<max_version> ("SELECT MAX(version) FROM " + obj->type_name_ + " WHERE name = '" + obj->name_ + "'"));
            result::iterator i(r.begin());

            if (i != r.end()) {
                obj->version_ = i->version + 1;
            } else {
                obj->version_ = 1;
            }

            obj->save_data(path);
            obj->persist_associated_pre(*this);
#ifdef VDBG
            std::cout << "DAS debug INFO: PRS " << obj->name_ << "... "; //DBG
#endif
            long long id = db_->persist(obj);
#ifdef VDBG
            std::cout << "done: id= " << id << std::endl;
#endif
            obj->persist_associated_post(*this);
            obj->is_dirty_ = false;

            // bind new object with this database
            obj->bundle_ = *this;
            return id;
        }

    }
}
#endif	/* DB_BUNDLE_IPP */

