#include "../../plf/result.hpp"

namespace das {
    namespace plf {

        class Functor {
        public:

            virtual
            shared_ptr<DasObject>
            load(tpl::Database *db, const long long &id) = 0;

            virtual
            shared_ptr<DasObject>
            load(tpl::Database *db, const std::string& name, int version = -1) = 0;

            virtual
            long long
            persist(tpl::Database *db, const shared_ptr<DasObject> &obj, std::string path = "") = 0;

            virtual
            void
            erase(tpl::Database *db, const shared_ptr<DasObject> &obj) = 0;

            virtual
            Result
            query(tpl::Database *db, const std::string& expression, const std::string& ordering = "", bool only_last_version = false) = 0;

            virtual
            std::vector<long long>
            query_id(tpl::Database *db, const std::string& expression, const std::string& ordering = "", bool only_last_version = false) = 0;

            virtual
            std::vector< std::pair<std::string, short> >
            query_name(tpl::Database *db, const std::string& expression, const std::string& ordering = "", bool only_last_version = false) = 0;

            virtual
            bool
            find(tpl::Database *db, const std::string& name, int version = -1) = 0;

            virtual
            void
            attach(tpl::Database *db, const shared_ptr<DasObject> &obj) = 0;

        };

        template<class Das_type>
        class FunctorImp : public Functor {
        public:

            virtual
            shared_ptr<DasObject>
            load(tpl::Database *db, const long long &id) {
                return db->load<Das_type>(id);
            }

            virtual
            shared_ptr<DasObject>
            load(tpl::Database *db, const std::string& name, int version = -1) {
                return db->load<Das_type>(name, version);
            }

            virtual
            long long
            persist(tpl::Database *db, const shared_ptr<DasObject> &obj, std::string path = "") {
                shared_ptr<Das_type> obj_ = std::tr1::dynamic_pointer_cast<Das_type> (obj);
                return db->persist(obj_, path);
            }

            virtual
            void
            erase(tpl::Database *db, const shared_ptr<DasObject> &obj) {
                shared_ptr<Das_type> obj_ = std::tr1::dynamic_pointer_cast<Das_type> (obj);
                db->erase(obj_);
            }

            virtual
            Result
            query(tpl::Database *db, const std::string& expression, const std::string& ordering = "", bool only_last_version = false) {
                ResultWrapper *rw = new ResultWrapperImp<Das_type>(db->query<Das_type>(expression,ordering,only_last_version));
                return Result(rw);
            }

            virtual
            std::vector<long long>
            query_id(tpl::Database *db, const std::string& expression, const std::string& ordering = "", bool only_last_version = false) {
                return db->query_id<Das_type> (expression, ordering,only_last_version);
            }

            virtual
            std::vector< std::pair<std::string, short> >
            query_name(tpl::Database *db, const std::string& expression, const std::string& ordering = "", bool only_last_version = false) {
                return db->query_name<Das_type> (expression, ordering,only_last_version);
            }

            virtual
            bool
            find(tpl::Database *db, const std::string& name, int version = -1) {
                return db->find<Das_type> (name, version);
            }

            virtual
            void
            attach(tpl::Database *db, const shared_ptr<DasObject> &obj) {
                shared_ptr<Das_type> obj_ = std::tr1::dynamic_pointer_cast<Das_type> (obj);
                db->attach(obj_);
            }

        };

        inline
        shared_ptr<Database>
        Database::create(const std::string& alias) {
            shared_ptr<odb::database> db;
            const das::DatabaseInfo &info = das::DatabaseConfig::database(alias);
            if (info.db_type != "mysql") {
                DAS_LOG_DBG("DAS debug INFO: non mysql dbms aren't supported yet");
                throw das::wrong_database();
            }
            db.reset(new odb::mysql::database(info.user, info.password, info.db_name, info.host, info.port));
            shared_ptr<Database> das_db(new Database(alias, db));
            das_db->info_ = DdlInfo::get_instance(alias);
            return das_db;
        }

        inline
        shared_ptr<DasObject>
        Database::load(const std::string &type_name, const long long &id) {
            return f_.at(type_name)->load(this, id);
        }

        inline
        shared_ptr<DasObject>
        Database::load(const std::string &type_name, const std::string& name, int version) {
            return f_.at(type_name)->load(this, name, version);
        }

        inline
        long long
        Database::persist(const shared_ptr<DasObject> &obj, std::string path) {
            return f_.at(obj->type_name())->persist(this, obj, path);
        }

        inline
        void
        Database::erase(const shared_ptr<DasObject> &obj) {
            f_.at(obj->type_name())->erase(this, obj);
        }

        inline
        Result
        Database::query(const std::string& type_name, const std::string& expression, const std::string& ordering, bool only_last_version) {
            return f_.at(type_name)->query(this, expression, ordering, only_last_version);
        }

        inline
        std::vector<long long>
        Database::query_id(const std::string& type_name, const std::string& expression, const std::string& ordering, bool only_last_version) {
            return f_.at(type_name)->query_id(this, expression, ordering, only_last_version);
        }

        inline
        std::vector< std::pair<std::string, short> >
        Database::query_name(const std::string& type_name, const std::string& expression, const std::string& ordering, bool only_last_version) {
            return f_.at(type_name)->query_name(this, expression, ordering, only_last_version);
        }

        inline
        bool
        Database::find(const std::string& type_name, const std::string& name, int version) {
            return f_.at(type_name)->find(this, name, version);
        }

        inline
        void
        Database::attach(const shared_ptr<DasObject> &obj) {
            f_.at(obj->type_name())->attach(this, obj);
        }

    }
}