#ifndef DAS_DB_BUNDLE_HPP
#define DAS_DB_BUNDLE_HPP
#include <odb/database.hxx>
#include <odb/session.hxx>
#include <odb/transaction.hxx>
#include <memory>
#include <odb/traits.hxx>
#include <odb/tr1/memory.hxx>
#include "../exceptions.hpp"

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
            valid() const {
                return db_ && session_;
            }

            template<typename T>
            void
            attach(const shared_ptr<T>& obj);

            template<typename T>
            long long
            persist(const shared_ptr<T> &obj, std::string path = "");

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
                return DbBundle(db_alias_, db, session);
            }

            void
            reset() {
                db_alias_ = "";
                db_.reset();
                session_.reset();
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



    }
}

#endif
