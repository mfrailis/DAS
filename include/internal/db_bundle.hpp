#ifndef DAS_DB_BUNDLE_HPP
#define DAS_DB_BUNDLE_HPP
#include <odb/database.hxx>
#include <odb/session.hxx>
#include <odb/transaction.hxx>
#include <memory>
#include <vector>
#include <typeinfo>
#include <set>
#include <odb/traits.hxx>
#include <odb/tr1/memory.hxx>
#include "../exceptions.hpp"

using std::tr1::shared_ptr;
using std::tr1::weak_ptr;

namespace das {

    class Transaction;
    class StorageTransaction;
    class WeakDbBundle;
    class DbBundle;

    class TransactionBundle {
    public:
        typedef std::vector<shared_ptr<StorageTransaction> > data_list_type;

        TransactionBundle(const std::string &alias,
                const shared_ptr<odb::database> &db,
                const shared_ptr<odb::session> &s,
                const shared_ptr<odb::transaction> &t)
        : db_alias_(alias), db_(db), session_(s), transaction_(t) {
        }

        template<typename T>
        long long
        persist(const shared_ptr<T> &obj, std::string path = "");


        void
        flush_session();
        
        void
        flush_session(const std::set<const std::type_info*> &types);       

        void
        flush_data();

        void
        add(const shared_ptr<StorageTransaction> &e) {
            data_list_.push_back(e);
        }

        const shared_ptr<odb::database>&
        db() {
            return db_;
        }

        bool equal(const DbBundle &rhs) const;

        bool equal(const WeakDbBundle &rhs) const;

    private:
        friend class WeakDbBundle;
        friend class Transaction;
        data_list_type data_list_;
        std::string db_alias_;
        shared_ptr<odb::database> db_;
        shared_ptr<odb::session> session_;
        shared_ptr<odb::transaction> transaction_;
    };

    class DbBundle {
    public:

        DbBundle() {
        }

        DbBundle(const std::string &alias,
                const shared_ptr<odb::database> &db) :
        db_alias_(alias),
        db_(db) {
        }

        DbBundle(const std::string &alias,
                const shared_ptr<odb::database> &db,
                const shared_ptr<odb::session> &session) :
        db_alias_(alias),
        db_(db),
        session_(session) {
        }

        bool equal(const DbBundle &rhs) const {
            return db_ == rhs.db_ && session_.lock() == rhs.session_.lock();
        }

        bool equal(const WeakDbBundle&rhs) const;

        const std::string&
        alias() const {
            return db_alias_;
        }

        const shared_ptr<odb::database>&
        db() const {
            return db_;
        }

        shared_ptr<odb::session>
        lock_session(bool throw_on_expired = true) const {
            shared_ptr<odb::session> session = session_.lock(); //may be null
            if (throw_on_expired && !session)
                throw das::not_in_session();
            return session;
        }

        bool
        valid() const {
            return db_ && !session_.expired();
        }

        shared_ptr<odb::transaction>
        transaction() {
            return transaction_.lock();
        }

        void
        transaction(const shared_ptr<odb::transaction> &ptr);

        void
        reset_session(const shared_ptr<odb::session> &ptr);

        void
        reset_session();

        template<typename T>
        void
        attach(const shared_ptr<T>& obj, bool log_bad_entries = true);




    private:
        friend class WeakDbBundle;
        std::string db_alias_;
        shared_ptr<odb::database> db_;
        weak_ptr<odb::session> session_;
        weak_ptr<odb::transaction> transaction_;
    };

    class WeakDbBundle {
    public:

        WeakDbBundle() {
        }

        WeakDbBundle(const std::string &alias) : db_alias_(alias) {
        }

        WeakDbBundle(const TransactionBundle &rhs) :
        db_alias_(rhs.db_alias_),
        db_(rhs.db_),
        session_(rhs.session_),
        transaction_(rhs.transaction_) {
        }

        WeakDbBundle(const DbBundle &rhs) :
        db_alias_(rhs.db_alias_),
        db_(rhs.db_),
        session_(rhs.session_),
        transaction_(rhs.transaction_) {
        }

        WeakDbBundle& operator=(const TransactionBundle &rhs) {
            db_alias_ = rhs.db_alias_;
            db_ = rhs.db_;
            session_ = rhs.session_;
            transaction_ = rhs.transaction_;
            return *this;
        }

        WeakDbBundle& operator=(const DbBundle &rhs) {
            db_alias_ = rhs.db_alias_;
            db_ = rhs.db_;
            session_ = rhs.session_;
            transaction_ = rhs.transaction_;
            return *this;
        }

        bool operator ==(const DbBundle &rhs) const {
            return lock_db(false) == rhs.db() && lock_session(false) == rhs.lock_session(false);
        }

        bool operator !=(const DbBundle &rhs) const {
            return !operator ==(rhs);
        }

        const std::string&
        alias() const {
            return db_alias_;
        }

        shared_ptr<odb::transaction>
        lock_transaction() {
            return transaction_.lock();
        }

        bool
        transaction_expired() {
            return transaction_.expired();
        }

        shared_ptr<odb::database>
        lock_db(bool throw_on_expired = true) const {
            shared_ptr<odb::database> database = db_.lock();
            if (throw_on_expired && !database)
                throw das::no_database();
            return database;
        }

        shared_ptr<odb::session>
        lock_session(bool throw_on_expired = true) const {
            shared_ptr<odb::session> session = session_.lock(); //may be null
            if (throw_on_expired && !session)
                throw das::not_in_session();
            return session;
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
        lock(bool throw_on_expired = true) const {

            shared_ptr<odb::database> db = db_.lock();
            shared_ptr<odb::session> session = session_.lock(); //may be null
            if (throw_on_expired && expired())
                throw das::not_in_managed_context();

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
        weak_ptr<odb::transaction> transaction_;
    };

    inline bool
    DbBundle::equal(const WeakDbBundle &rhs) const {
        return db_ == rhs.lock_db(false) && session_.lock() == rhs.lock_session(false);
    }

    inline bool
    TransactionBundle::equal(const WeakDbBundle &rhs) const {
        return db_ == rhs.lock_db(false) && session_ == rhs.lock_session(false);
    }

    inline bool
    TransactionBundle::equal(const DbBundle &rhs) const {
        return db_ == rhs.db() && session_ == rhs.lock_session(false);
    }

    template<typename T>
    bool operator==(const T &lhs, const WeakDbBundle &rhs) {
        return lhs.equal(rhs);
    }

    template<typename T>
    bool operator!=(const T &lhs, const WeakDbBundle &rhs) {
        return !lhs.equal(rhs);
    }


}

#endif
