#ifndef DAS_PLF_DATABASE_HPP
#define DAS_PLF_DATABASE_HPP

#include "../tpl/database.hpp"
#include "../transaction.hpp"
#include "result.hpp"
#include <boost/unordered_map.hpp>

using std::tr1::shared_ptr;
using std::tr1::weak_ptr;

namespace das {
    namespace plf {

        typedef das::Transaction Transaction;

        class Functor;

        /** 
         * @brief Polymorphic version of the Persistence Manager class.
         *
         * It provides an interface for storing, retrieving and deleting objects from
         * a Persistence Unit instance. 
         */
        class Database : private tpl::Database {
        public:
            typedef tpl::Database super;
            /**
             * @param alias pu-instance identifier
             * @throw das::wrong_database if alias isn't a valid identifier.
             */
            static
            shared_ptr<Database>
            create(const std::string& alias) throw (das::wrong_database);

            /**
             * Starts a new database transaction, if you are not using an
             * extended session, a new session that spans only the transaction 
             * life is also started.
             * @tparam das::isolation_level the isolation level for the transaction
             * @return Transaction needed for commit or rollback the
             * transaction. Note that if the returned object goes out of scope
             * before any of Transaction::commit(), Transaction::rollback()
             * is called (this happen if you don't store the return object in a
             * variable for example); the database transaction is automatically
             * rolled back.
             * @throw already_in_transaction if a previously transaction
             * started from this pm-manager isn't finalized yet, i.e. neither
             * committed or rolled-back.
             */
            Transaction
            begin(isolation_level isolation = databaseDefault) throw (das::already_in_transaction) {
                return super::begin(isolation);
            }

            shared_ptr<DasObject>
            load(const std::string &type_name, const long long &id)
            throw (object_not_persistent);

            /**
             * Loads an object given a name and optionally a version number.
             * @param type_name the ddl-type to load
             * @param name name of the object
             * @param version if not provided, the last version of the object is
             * returned
             * @throw object_not_persistent if no name,version of type T resides
             * on database
             */
            shared_ptr<DasObject>
            load(const std::string &type_name, const std::string& name, int version = -1);

            /**
             * Persists an object in the database. From now until the session end,
             * obj is an attached object: changes on data or metadata will be eventually
             * flushed in the pu-instance.
             * @param obj object to persist
             * @param path optional sub-URI to the persistent data location
             * @return the id of the new persistent object
             */
            long long
            persist(const shared_ptr<DasObject> &obj, std::string path = "")
            throw (das::not_in_transaction, das::wrong_database);

            /**
             * Deletes the argument object from the database. The in memory
             * object and its metadata is still accessible. On the other hand
             * data access behaviour is undefined.
             */
            void
            erase(const shared_ptr<DasObject> &obj);

            /**
             * Database query interface. The Result container returned is valid only 
             * inside the current transaction. Trying to access a Result container
             * outside a transaction leads to an undefined behavior.
             * For this reason, this method shows its best when used for fast 
             * result iteration such as printing keywords or load result objects
             * in in-memory containers.
             * @param type_name the ddl-type to query for
             * @param expression query expression as defined [here](index.html#ql).
             * @param ordering optional list of comma separated keyword/order as defined 
             * [here](index.html#order_clause).
             * @param last_version_only specifies whether to return each version of
             * the objects that meet the expression requirements, or just the 
             * highest one.
             * @return iterable Result container or the ddl-objects retrieved from
             * the database.
             */
            Result
            query(const std::string& type_name, const std::string& expression, const std::string& ordering = "", bool only_last_version = false);


            /**
             * If you plan to perform a possibly long computation iterating through
             * the result you should use this, or the query_name() method.
             * Once this method returns you can commit the current transaction 
             * and then load each object using result vector in a new one.
             * This method has also the smallest memory footprint of the three 
             * query facilities because it doesn't internally load any ddl-object.
             * @param expression query expression as defined [here](index.html#ql).
             * @param ordering optional list of comma separated keyword/order as defined 
             * [here](index.html#order_clause).
             * @param last_version_only specifies whether to return each version of
             * the objects that meet the expression requirements, or just the 
             * highest one.
             * @return A vector containing the ids of the objects returned from
             * the database query.
             */
            std::vector<long long>
            query_id(const std::string& type_name, const std::string& expression, const std::string& ordering = "", bool only_last_version = false);


            /**
             * Little variant of the previous method. Remain true the consideration
             * for the previous method query_id().
             * @param expression query expression as defined [here](index.html#ql).
             * @param ordering optional list of comma separated keyword/order as defined 
             * [here](index.html#order_clause).
             * @param last_version_only specifies whether to return each version of
             * the objects that meet the expression requirements, or just the 
             * highest one.
             * @return A vector of pairs name,version of the objects returned from
             * the database query.
             */
            std::vector< std::pair<std::string, short> >
            query_name(const std::string& type_name, const std::string& expression, const std::string& ordering, bool only_last_version = false);


            bool
            find(const std::string& type_name, const std::string& name, int version = -1);


            /**
             * Attaches the object to the current session, either an extended one
             * or a transaction bound. See the [this](index.html#ddl_object_state)
             * section for further about attached objects.
             */
            void
            attach(const shared_ptr<DasObject> &obj);

            /**
             * Forces to update the data and metadata of all the managed objects.
             * This method is automatically called on transaction commit. 
             */
            void
            flush() {
                super::flush();
            }

            /**
             * begins an extended session. See [this](index.html#extended_session) for
             * further about extended session.
             */
            void
            begin_session() {
                super::begin_session();
            }

            /**
             * ends an extended session. See [this](index.html#extended_session) for
             * further about extended session.
             */
            void
            end_session() {
                super::end_session();
            }

        private:
            static boost::unordered_map< std::string, shared_ptr<Functor> > f_;

            Database();
            void init();

            Database(const std::string &db_alias,
                    const shared_ptr<odb::database> &db)
            : super(db_alias, db) {
                init();
            }
        };

    } //namespace plf
}//namespace das
#include "../internal/plf/database.ipp"
#endif
