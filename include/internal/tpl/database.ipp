#include "../log.hpp"
#include "../db_bundle.hpp"
namespace das {
    namespace tpl {

        inline
        shared_ptr<Database>
        Database::create(const std::string& alias) throw (das::wrong_database) {
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
        Transaction
        Database::begin() throw (das::already_in_transaction) {
            shared_ptr<odb::transaction> t = bundle_.transaction();
            if (t) {
                throw das::already_in_transaction();
            }
            shared_ptr<odb::session> s = bundle_.lock_session(false);
            if (!s) {
                s.reset(new odb::session());
                bundle_.reset_session(s);
            }
            t.reset(new odb::transaction(bundle_.db()->begin()));
            bundle_.transaction(t);

            shared_ptr<TransactionBundle> tb(new TransactionBundle(bundle_.alias(), bundle_.db(), s, t));
            tb_ = tb;
            return Transaction(tb);
        }

        template<typename T>
        shared_ptr<T>
        Database::load(const long long &id) throw (object_not_persistent) {
            shared_ptr<odb::session> s = bundle_.lock_session(true);
            odb::session::current(*s);
            shared_ptr<T> pobj;
            pobj = bundle_.db()->load<T>(id);
            // bind loaded object with this database
            pobj->bundle_ = bundle_;
            return pobj;
        }

        template<typename T>
        shared_ptr<T>
        Database::load(const std::string& name, int version) throw (object_not_persistent) {
            shared_ptr<odb::session> s = bundle_.lock_session(true);
            odb::session::current(*s);
            typedef odb::query<T> query;
            typedef odb::result<T> result;

            result *r;
            if (version != -1) {
                r = new result(bundle_.db()->query<T> (query::name == query::_val(name) &&
                        query::version == query::_val(version)));
            } else {
                r = new result(bundle_.db()->query<T> ((query::name == query::_val(name)) + "ORDER BY version DESC",
                        false)); // do not cache result, we just  need the first row.
            }

            shared_ptr<T> pobj;
            typename result::iterator i(r->begin());
            if (i != r->end()) {
                pobj = i.load();
            } else {
                DAS_LOG_DBG("DAS debug INFO: odb not persistent exception");
                throw object_not_persistent();
            }
            delete r;
            // bind loaded object with this database
            pobj->bundle_ = bundle_;
            return pobj;
        }

        template<typename T>
        inline
        long long
        Database::persist(const shared_ptr<T> &obj, std::string path) throw (das::not_in_transaction, das::wrong_database) {
            shared_ptr<TransactionBundle> tb = tb_.lock();
            if (!tb)
                throw das::not_in_transaction();

            return tb->persist<T>(obj, path);
        }

        template<typename T>
        inline
        Result<T>
        Database::query(const std::string& expression, const std::string& ordering, bool last_version_only) {
            shared_ptr<odb::session> s = bundle_.lock_session(true);
            shared_ptr<TransactionBundle> tb = tb_.lock();
            if (!tb)
                throw das::not_in_transaction();

            odb::session::current(*s);
            QLVisitor exp_visitor(das_traits<T>::name, info_);
            std::string clause = exp_visitor.parse_exp(expression, last_version_only);
            std::string order = exp_visitor.parse_ord(ordering);

            tb->flush_session(exp_visitor.get_involved_types());
            DAS_DBG(
                    typedef std::set<const std::type_info*> set;
                    const set& s = exp_visitor.get_involved_types();
            for (set::const_iterator it = s.begin(); it != s.end(); ++it) {
                DAS_LOG_DBG("DAS debug INFO: query type involved = " << (*it)->name());
            }
            )
            DAS_LOG_DBG("DAS debug INFO: WHERE" << std::endl << clause + order);

            odb::result<T> odb_r(bundle_.db()->query<T>(clause + order));
            //result<T> r = static_cast<result<T> > (bundle_.db()->query<T>(clause + order));
            //return static_cast<Result<T> > (odb_r);

            return Result<T>(odb_r, bundle_);
        }

        template<typename T>
        inline
        std::vector<long long>
        Database::query_id(const std::string& expression, const std::string& ordering, bool last_version_only) {

            std::vector<long long> ids;
            Result<T> r = query<T>(expression, ordering, last_version_only);
            for (typename Result<T>::const_iterator i(r.cbegin()); i != r.cend(); ++i) {
                ids.push_back(i.id());
            }
            return ids;
        }

        template<typename T>
        inline
        std::vector< std::pair<std::string, short> >
        Database::query_name(const std::string& expression, const std::string& ordering, bool last_version_only) {
            odb::session::reset_current(); //disable current session
            std::vector< std::pair<std::string, short> > pairs;

            Result<T> r = query<T>(expression, ordering, last_version_only);
            for (typename Result<T>::iterator i(r.begin()); i != r.end(); ++i) {
                std::pair<std::string, short> p((*i).name(), (*i).version());
                pairs.push_back(p);
            }
            return pairs;
        }

        template<typename T>
        inline
        bool
        Database::find(const std::string& name, int version) {
            typedef odb::result<find_count> result;
            //FIXME odb::id_common may be a problem
            std::string table(odb::access::object_traits_impl< T, odb::id_common >::table_name);
            bool found;

            result r(bundle_.db()->query<find_count> ("SELECT COUNT(*) FROM " + table + " WHERE name = '" + name + "'"));
            result::iterator i(r.begin());
            found = i->count != 0;

            return found;
        }

        template<typename T>
        inline
        void
        Database::attach(const shared_ptr<T> &obj) {
            //bundle_.attach<T>(obj);
            T::attach(obj, bundle_);
        }

        inline
        void
        Database::flush() {
            /*            if (odb::transaction::has_current()) {
                            DAS_LOG_DBG("DAS info: WARNING: calling db flush() while another transaction was open.");
                            return;
                        }
                        Transaction t = begin();
                        t.commit();
             */
            shared_ptr<TransactionBundle> tb = tb_.lock();
            if (!tb)
                throw das::not_in_transaction();
            tb->flush_session();
        }

        template<typename T>
        void
        Database::erase(const shared_ptr<T> &obj) {
            if (obj->is_new()) {
                DAS_LOG_DBG("DAS info: trying to delete a new object");
                return;
            }
            shared_ptr<odb::session> s = bundle_.lock_session(true);
            odb::session::current(*s);
            bundle_.db()->erase(obj);
            obj->das_id_ = 0;
            obj->bundle_.reset();
        }

        inline
        void
        Database::begin_session() {
            shared_ptr<odb::session> s = bundle_.lock_session(false);
            if (s.unique() || !s) {
                s.reset(new odb::session());
                bundle_.reset_session(s);
                extended_ = s;
            } else
                throw already_in_session();
        }

        inline
        void
        Database::end_session() {
            if (extended_) {
                extended_.reset();
            } else
                throw not_in_session();
        }
    }
}