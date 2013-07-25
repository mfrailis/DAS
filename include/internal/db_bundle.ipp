#ifndef DB_BUNDLE_IPP
#define	DB_BUNDLE_IPP
#include "db_bundle.hpp"
#include "aux_query.hpp"
#include "../ddl/types/mysql/aux_query-odb.hxx"
#include "../internal/log.hpp"
#include "../das_object.hpp"
namespace das {

    namespace tpl {

        template<typename T>
        inline
        void
        DbBundle::attach(const shared_ptr<T> &obj) {

            if (obj->is_new()) {
                DAS_LOG_DBG("DAS info: trying to attach an object without persisting firts");
                throw das::new_object();
            }
            if (!obj->bundle_.expired()) {
                if (obj->bundle_ != *this) {
                    DAS_LOG_DBG("DAS info: ERROR: trying to attach an object managed by other database");
                    throw das::wrong_database();
                } else
                    return;
            }
            // throws an exception if the session is expired
            shared_ptr<odb::session> s = lock_session(true); 
            odb::session::current(*s);
            shared_ptr<T> cache_hit = s->cache_find<T>(*db_, obj->das_id_);
            if (cache_hit) {
                if (cache_hit != obj) {
                    DAS_LOG_DBG("DAS info: ERROR: another copy of this object found in cache");
                    throw das::object_not_unique();
                } else {
                    DAS_LOG_DBG("DAS info: obj found cache but not attached to db: if you are loading from a query result ignore this message");
                    cache_hit->bundle_ = *this;
                }
            } else {
                s->cache_insert<T>(*db_, obj->das_id_, obj);
                obj->bundle_ = *this;
                obj->is_dirty_ = true;
                /* this will trigger a column files configuration reset on next 
                 * commit if the object will be dirty
                 */
                obj->set_dirty_columns();
            }
            
        }

        template<typename T>
        long long
        DbBundle::persist(const shared_ptr<T> &obj, std::string path) {

            if (!obj->is_new()) {
                if (operator !=(obj->bundle_)) // another db intance owns this obj
                {
                    throw wrong_database();
                }
                return obj->das_id_;
            } //FIXME should we do an update insted?

            if (!obj->bundle_.blank()) // should never happen
            {
                DAS_LOG_DBG("DAS debug INFO: ERROR: obj '" << obj->name_ << "' is new and his bundle isn't blank");
                throw wrong_database();
            }

            DAS_DBG
                    (
            if (obj->version_ != 0)
                    DAS_LOG_DBG("DAS debug INFO: WARNING: changing version number while persisting obj " << obj->name_);
                    );

            typedef odb::result<max_version> result;
            odb::session::reset_current();
            result r(db_->query<max_version> ("SELECT MAX(version) FROM " + obj->type_name_ + " WHERE name = '" + obj->name_ + "'"));
            result::iterator i(r.begin());

            if (i != r.end()) {
                obj->version_ = i->version + 1;
            } else {
                obj->version_ = 1;
            }
            shared_ptr<odb::session> s = lock_session(true);

            obj->save_data(path,*this);
                      
            odb::session::current(*s);
            obj->persist_associated_pre(*this); 
            
            DAS_LOG_DBG("DAS debug INFO: PRS " << obj->name_ << "... ");
            long long id = db_->persist(obj);
            DAS_LOG_DBG("done: id= " << id);

            obj->persist_associated_post(*this);
            obj->is_dirty_ = false;

            // bind new object with this database
            obj->bundle_ = *this;
            return id;
        }

        inline
        void
        DbBundle::reset_session(const shared_ptr<odb::session> &ptr) {
            session_ = ptr;
        }

        inline
        void
        DbBundle::transaction(const shared_ptr<odb::transaction> &ptr) {
            transaction_ = ptr;
        }
        
    }
}
#endif	/* DB_BUNDLE_IPP */

