#include "internal/db_bundle.ipp"
#include "ddl/types/mysql/das_object-odb.hxx"

namespace das {
    namespace tpl {

        void
        TransactionBundle::flush_session() {
            typedef odb::session::database_map database_map;
            typedef odb::session::type_map type_map;
            typedef odb::session::object_map<DasObject> object_map;

            database_map::iterator db_it(session_->map().find(db_.get()));

            if (db_it == session_->map().end()) {
                DAS_LOG_DBG("DAS debug INFO: session empty");
            } else {
                for (type_map::iterator type_it = db_it->second.begin();
                        type_it != db_it->second.end();
                        type_it++) {
                    object_map & obj_map(static_cast<object_map&> (*type_it->second));
                    for (object_map::iterator obj_it = obj_map.begin();
                            obj_it != obj_map.end();
                            obj_it++) {
                        obj_it->second->update(*this);
                    }
                }
            }
        }

        void
        TransactionBundle::flush_data() {
            typedef odb::session::database_map database_map;
            typedef odb::session::type_map type_map;
            typedef odb::session::object_map<DasObject> object_map;

            database_map::iterator db_it(session_->map().find(db_.get()));

            if (db_it == session_->map().end()) {
                DAS_LOG_DBG("DAS debug INFO: session empty");
            } else {
                for (type_map::iterator type_it = db_it->second.begin();
                        type_it != db_it->second.end();
                        type_it++) {
                    object_map & obj_map(static_cast<object_map&> (*type_it->second));
                    for (object_map::iterator obj_it = obj_map.begin();
                            obj_it != obj_map.end();
                            obj_it++) {
                        obj_it->second->save_data(*this);
                    }
                }
            }
        }
        /*  
              bool operator==(const DbBundle &lhs, const DbBundle &rhs){return lhs.equal(rhs);}
              bool operator!=(const DbBundle &lhs, const DbBundle &rhs){return !lhs.equal(rhs);}

              bool operator==(const WeakDbBundle &lhs, const DbBundle &rhs){return rhs.equal(lhs);}
              bool operator!=(const WeakDbBundle &lhs, const DbBundle &rhs){return !rhs.equal(lhs);}

              bool operator==(const DbBundle &lhs, const WeakDbBundle &rhs){return lhs.equal(rhs);}
              bool operator!=(const DbBundle &lhs, const WeakDbBundle &rhs){return !lhs.equal(rhs);}
         */
    }
}
