#ifndef DAS_DB_BUNDLE_HPP
#define DAS_DB_BUNDLE_HPP
#include <odb/database.hxx>
#include <odb/session.hxx>
#include <odb/transaction.hxx>
#include <memory>
#include <odb/tr1/memory.hxx>
#include "exceptions.hpp"
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;

namespace das {
    namespace tpl {
        class WeakDbBundle;

        class DbBundle {
        public:

            DbBundle() {
            }

            DbBundle(const std::string &alias,
                    const shared_ptr<odb::database> &db,
                    const shared_ptr<odb::session> &session) :
            db_alias_(alias),
            db_(db),
            session_(session) {
            }

            bool operator ==(const DbBundle&rhs) const {
                return db_ == rhs.db_ && session_ == rhs.session_;
            }

            bool operator !=(const DbBundle&rhs) const {
                return !operator ==(rhs);
            }

            bool operator ==(const WeakDbBundle&rhs) const;

            bool operator !=(const WeakDbBundle&rhs) const {
                return !operator ==(rhs);
            }

            const std::string&
            alias() const {
                return db_alias_;
            }

            const shared_ptr<odb::database>&
            db() const {
                return db_;
            }

            const shared_ptr<odb::session>&
            session() const {
                return session_;
            }
            
            bool
            valid() const
            {
                return db_ && session_;
            }

            template<typename T>
            void
            attach(typename odb::object_traits<T>::pointer_type& obj);

        private:
            friend class WeakDbBundle;
            std::string db_alias_;
            shared_ptr<odb::database> db_;
            shared_ptr<odb::session> session_;
        };

        class WeakDbBundle {
        public:

            WeakDbBundle() {
            }

            WeakDbBundle(const DbBundle &rhs) :
            db_alias_(rhs.db_alias_),
            db_(rhs.db_),
            session_(rhs.session_) {
            }

            WeakDbBundle& operator=(const DbBundle &rhs) {
                db_alias_ = rhs.db_alias_;
                db_ = rhs.db_;
                session_ = rhs.session_;
                return *this;
            }

            bool operator ==(const DbBundle &rhs) const {
                return db() == rhs.db() && session() == rhs.session();
            }

            bool operator !=(const DbBundle &rhs) const {
                return !operator ==(rhs);
            }

            const std::string&
            alias() const {
                return db_alias_;
            }

            const shared_ptr<odb::database>
            db() const {
                return db_.lock();
            }

            const shared_ptr<odb::session>
            session() const {
                return session_.lock();
            }

            bool
            expired() const {
                return db_.expired() || session_.expired();
            }

            bool
            blank() const {
                return expired() && db_alias_ == "";
            }

            DbBundle
            lock() {

                shared_ptr<odb::database> db = db_.lock();
                shared_ptr<odb::session> session = session_.lock();
                return DbBundle(db_alias_,db,session);
            }

        private:
            std::string db_alias_;
            weak_ptr<odb::database> db_;
            weak_ptr<odb::session> session_;
        };

        inline bool
        DbBundle::operator==(const WeakDbBundle &rhs) const {
            return db_ == rhs.db() && session_ == rhs.session();
        }

        //TODO move this method in he bundle!!!

        template<typename T>
        inline
        void
        DbBundle::attach(typename odb::object_traits<T>::pointer_type& obj) {
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



    }
}

#endif
