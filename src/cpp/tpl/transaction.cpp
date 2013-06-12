#include "tpl/transaction.hpp"
#include "tpl/database.hpp"
#include "ddl/types/mysql/das_object-odb.hxx"

using namespace das::tpl;

Transaction::Transaction(const DbBundle &bundle)
: w_bundle_(bundle) {
    transaction_.reset(new odb::transaction(bundle.db()->begin()));
}

void
Transaction::commit() {
    typedef odb::session::database_map database_map;
    typedef odb::session::type_map type_map;
    typedef odb::session::object_map<DasObject> object_map;

    DbBundle bundle = w_bundle_.lock();
    if (!bundle.valid())
        throw not_in_managed_context();

    const shared_ptr<odb::session> &session_ = bundle.session();
    const shared_ptr<odb::database> &db_ = bundle.db();

    database_map::iterator db_it(session_->map().find(db_.get()));

    if (db_it == session_->map().end()) {
#ifdef VDBG
        std::cout << "DAS debug INFO: session empty" << std::endl;
#endif     
    }
    else {
        for (type_map::iterator type_it = db_it->second.begin();
                type_it != db_it->second.end();
                type_it++) {
            object_map & obj_map(static_cast<object_map&> (*type_it->second));
            for (typename object_map::iterator obj_it = obj_map.begin();
                    obj_it != obj_map.end();
                    obj_it++) {
                obj_it->second->update();
            }
        }
    }
    transaction_->commit();
}

void
inline
Transaction::rollback(){
    transaction_->rollback();
}

