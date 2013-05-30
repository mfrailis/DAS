inline
shared_ptr<Database>
Database::create(const std::string& alias) {
    shared_ptr<odb::database> db;
    shared_ptr<odb::session> session;
    //TODO look for configured databases
    //TODO select proper database
    //db->db_.reset(new odb::mysql::database(user,password,database,host,port));
    if (alias == "local")
        db.reset(new odb::mysql::database("odb_test", "", "odb_test"));
    else if (alias == "benchmark")
        db.reset(new odb::mysql::database("odb_test", "", "benchmark"));
    else
        throw das::wrong_database();

    session.reset(new odb::session(false));
    shared_ptr<Database> das_db(new Database(alias, db, session));
    das_db->info_ = DdlInfo::get_instance(alias);
    return das_db;
}

inline
Transaction
Database::begin() {
    Transaction t(bundle_);
    return t;
}

template<typename T>
typename odb::object_traits<T>::pointer_type
Database::load(const typename odb::object_traits<const T>::id_type& id) {
    odb::session::current(*(bundle_.session()));
    typename odb::object_traits<T>::pointer_type pobj;
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
typename odb::object_traits<T>::pointer_type
Database::load(const std::string& name, int version) {
    odb::session(*(bundle_.session()));
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

    typename odb::object_traits<T>::pointer_type pobj;
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
typename odb::object_traits<T>::id_type
Database::persist(typename odb::object_traits<T>::pointer_type& obj, std::string path) {
    odb::session::current(*(bundle_.session()));
    //shared_ptr<Database> self = self_.lock(); //always succesful
    if (!obj->is_new()) {
        if (obj->bundle_ != bundle_) // another db intance owns this obj
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
    result r(bundle_.db()->query<max_version> ("SELECT MAX(version) FROM " + obj->type_name_ + " WHERE name = '" + obj->name_ + "'"));
    result::iterator i(r.begin());

    if (i != r.end()) {
        obj->version_ = i->version + 1;
    } else {
        obj->version_ = 1;
    }

    obj->save_data(path);
    obj->persist_associated_pre(this);
#ifdef VDBG
    std::cout << "DAS debug INFO: PRS " << obj->name_ << "... "; //DBG
#endif
    typename odb::object_traits<T>::id_type id = bundle_.db()->persist<T>(obj);
#ifdef VDBG
    std::cout << "done: id= " << id << std::endl;
#endif
    obj->persist_associated_post(this);
    obj->is_dirty_ = false;

    // bind new object with this database
    obj->bundle_ = bundle_;
    obj->is_dirty_ = false;
    return id;
}

template<typename T>
inline
odb::result<T>
Database::query(const std::string& expression, const std::string& ordering) {
    odb::session::current(*(bundle_.session()));
    QLVisitor exp_visitor(das_traits<T>::name, info_);
    std::string clause = exp_visitor.parse_exp(expression);
    std::string order = exp_visitor.parse_ord(ordering);

#ifdef VDBG
    std::cout << "DAS debug INFO: WHERE\n" << clause + order << std::endl; //DBG
#endif

    return bundle_.db()->query<T>(clause + order);
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

//TODO move this method in he bundle!!!
template<typename T>
inline
void
Database::attach(typename odb::object_traits<T>::pointer_type& obj) {
    if (obj->is_new()) {
#ifdef VDBG
        std::cout << "DAS info: trying to attach an object without persisting firts" << std::endl;
#endif
        throw das::new_object();
    }
    if (!obj->bundle_.expired()) {
        if (obj->bundle_ != bundle_) {
#ifdef VDBG
            std::cout << "DAS info: ERROR: trying to attach an object managed by other database" << std::endl;
#endif
            throw das::wrong_database();
        } else
            return;
    }

    odb::session::current(*(bundle_.session()));
    shared_ptr<T> cache_hit = bundle_.session()->cache_find<T>(*(bundle_.db()), obj->das_id_);
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
            cache_hit->bundle_ = bundle_;
        }
    } else {
        bundle_.session()->cache_insert<T>(*(bundle_.db()), obj->das_id_, obj);
        obj->bundle_ = bundle_;
    }

}

inline
void
Database::flush() {
    if (odb::transaction::has_current()) {
#ifdef VDBG
        std::cout << "DAS info: calling db flush() while another transaction was open." << std::endl;
#endif                   
        return;
    }
    Transaction t = begin();
    t.commit();
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
