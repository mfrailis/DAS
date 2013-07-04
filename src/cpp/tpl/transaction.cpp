#include "tpl/transaction.hpp"
#include "tpl/database.hpp"
#include "ddl/types/mysql/das_object-odb.hxx"

using namespace das::tpl;

Transaction::Transaction(const shared_ptr<odb::database> &db,const shared_ptr<odb::session> &s)
: db_(db), session_(s) {
    transaction_.reset(new odb::transaction(db_->begin()));
}

void
Transaction::commit() {
    typedef odb::session::database_map database_map;
    typedef odb::session::type_map type_map;
    typedef odb::session::object_map<DasObject> object_map;

    database_map::iterator db_it(session_->map().find(db_.get()));

    if (db_it == session_->map().end()) {
        DAS_LOG_DBG("DAS debug INFO: session empty");
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

