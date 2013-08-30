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

        class Database : private tpl::Database {
        public:
            typedef tpl::Database super;

            static
            shared_ptr<Database>
            create(const std::string& alias);

            Transaction
            begin() {
                return super::begin();
            }

            shared_ptr<DasObject>
            load(const std::string &type_name, const long long &id);


            shared_ptr<DasObject>
            load(const std::string &type_name, const std::string& name, int version = -1);

            long long
            persist(const shared_ptr<DasObject> &obj, std::string path = "");

            void
            erase(const shared_ptr<DasObject> &obj);


            Result
            query(const std::string& type_name, const std::string& expression, const std::string& ordering = "", bool only_last_version = false);


            std::vector<long long>
            query_id(const std::string& type_name, const std::string& expression, const std::string& ordering = "", bool only_last_version = false);


            std::vector< std::pair<std::string, short> >
            query_name(const std::string& type_name, const std::string& expression, const std::string& ordering, bool only_last_version = false);


            bool
            find(const std::string& type_name, const std::string& name, int version = -1);


            void
            attach(const shared_ptr<DasObject> &obj);

            void
            flush() {
                super::flush();
            }

            void
            begin_session() {
                super::begin_session();
            }

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
