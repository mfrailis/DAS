namespace das {
    namespace tpl {

        inline
        shared_ptr<Database>
        Database::create(const std::string& alias) {
            shared_ptr<odb::database> db;
            shared_ptr<odb::session> session;
            const das::DatabaseInfo &info = das::DatabaseConfig::database(alias);
            if (info.db_type != "mysql") {
#ifdef VDBG
                std::cout << "DAS debug INFO: non mysql dbms aren't supported yet" << std::endl;
#endif    
                throw das::wrong_database();
            }
            db.reset(new odb::mysql::database(info.user, info.password, info.db_name, info.host, info.port));
            session.reset(new odb::session(false));
            shared_ptr<Database> das_db(new Database(alias, db, session));
            das_db->info_ = DdlInfo::get_instance(alias);
            return das_db;
        }

        inline
        Transaction
        Database::begin() {
            return Transaction(bundle_);
        }

        template<typename T>
        shared_ptr<T>
        Database::load(const long long &id) {
            odb::session::current(*(bundle_.session()));
            shared_ptr<T> pobj;
            bool local_trans = false;
            odb::transaction *transaction = auto_begin(local_trans);
            try {
                pobj = bundle_.db()->load<T>(id);
            } catch (const std::exception &e) {
                auto_catch(local_trans, transaction);
            }

            auto_commit(local_trans, transaction);

            // bind loaded object with this database
            pobj->bundle_ = bundle_;            
            return pobj;
        }

        template<typename T>
        shared_ptr<T>
        Database::load(const std::string& name, int version) {
            odb::session::current(*(bundle_.session()));
            typedef odb::query<T> query;
            typedef odb::result<T> result;

            bool local_trans = false;
            odb::transaction *transaction = auto_begin(local_trans);

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
                try {
                    pobj = i.load();
                } catch (const std::exception &e) {
                    auto_catch(local_trans, transaction);
                }
            } else {
                if (local_trans) {
                    transaction->rollback();
                    delete transaction;
                }
#ifdef VDBG
                std::cout << "DAS debug INFO: odb not persistent exception" << std::endl;
#endif
                throw object_not_persistent();
            }

            delete r;

            auto_commit(local_trans, transaction);
            // bind loaded object with this database
            pobj->bundle_ = bundle_;
            return pobj;
        }

        template<typename T>
        inline
        long long
        Database::persist(const shared_ptr<T> &obj, std::string path) {
            return bundle_.persist<T>(obj, path);
        }

        template<typename T>
        inline
        Result<T>
        Database::query(const std::string& expression, const std::string& ordering) {
            odb::session::current(*(bundle_.session()));
            QLVisitor exp_visitor(das_traits<T>::name, info_);
            std::string clause = exp_visitor.parse_exp(expression);
            std::string order = exp_visitor.parse_ord(ordering);

#ifdef VDBG
            std::cout << "DAS debug INFO: WHERE\n" << clause + order << std::endl; //DBG
#endif
            odb::result<T> odb_r(bundle_.db()->query<T>(clause + order));
            //result<T> r = static_cast<result<T> > (bundle_.db()->query<T>(clause + order));
            //return static_cast<Result<T> > (odb_r);
            return Result<T>(odb_r,bundle_);
        }

        template<typename T>
        inline
        std::vector<long long>
        Database::query_id(const std::string& expression, const std::string& ordering) {
            std::vector<long long> ids;
            bool local_trans = false;
            odb::transaction *transaction = auto_begin(local_trans);
            try {
                Result<T> r = query<T>(expression, ordering);
                for (typename Result<T>::iterator i(r.begin()); i != r.end(); ++i) {
                    ids.push_back(i.id());
                }
            } catch (const std::exception &e) {
                auto_catch(local_trans, transaction);
            }
            auto_commit(local_trans, transaction);

            return ids;
        }
        
        
        template<typename T>
        inline
        std::vector< std::pair<std::string, short> >
        Database::query_name(const std::string& expression, const std::string& ordering) {
            odb::session::reset_current(); //disable current session
            std::vector< std::pair<std::string, short> > pairs;
            bool local_trans = false;
            odb::transaction *transaction = auto_begin(local_trans);
            try {
                Result<T> r = query<T>(expression, ordering);
                for (typename Result<T>::iterator i(r.begin()); i != r.end(); ++i) {
                    std::pair<std::string, short> p((*i).name(), (*i).version());
                    pairs.push_back(p);
                }
            } catch (const std::exception &e) {
                auto_catch(local_trans, transaction);
            }
            auto_commit(local_trans, transaction);
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
            bool local_trans = false;
            odb::transaction *transaction = auto_begin(local_trans);
            try {
                result r(bundle_.db()->query<find_count> ("SELECT COUNT(*) FROM " + table + " WHERE name = '" + name + "'"));
                result::iterator i(r.begin());
                found = i->count != 0;
            } catch (const std::exception &e) {
                auto_catch(local_trans, transaction);
            }
            auto_commit(local_trans, transaction);
            return found;
        }

        template<typename T>
        inline
        void
        Database::attach(const shared_ptr<T> &obj) {
            bundle_.attach<T>(obj);
        }

        inline
        void
        Database::flush() {
            if (odb::transaction::has_current()) {
#ifdef VDBG
                std::cout << "DAS info: WARNING: calling db flush() while another transaction was open." << std::endl;
#endif                   
                return;
            }
            Transaction t = begin();
            t.commit();
        }

        template<typename T>
        void
        Database::erase(const shared_ptr<T> &obj) {
            if (obj->is_new()) {
#ifdef VDBG
                std::cout << "DAS info: trying to delete a new object" << std::endl;
#endif
                return;
            }
            odb::session::current(*bundle_.session());
            bundle_.db()->erase(obj);
            obj->das_id_ = 0;
            obj->bundle_.reset();
        }

        inline
        odb::transaction*
        Database::auto_begin(bool &is_auto) {
            odb::transaction *t;
            is_auto = !odb::transaction::has_current();
            if (is_auto) {
                t = new odb::transaction(bundle_.db()->begin());
            } else {
                t = &odb::transaction::current();
            }
            return t;
        }

        inline
        void
        Database::auto_catch(bool is_auto, odb::transaction *t) {
            if (is_auto) {
                t->rollback();
                delete t;
            }
            throw;
        }

        inline
        void
        Database::auto_commit(bool is_auto, odb::transaction *t) {
            if (is_auto) {
                t->commit();
                delete t;
            }
        }
    }
}