#include "internal/db_bundle.ipp"
#include "ddl/types/mysql/das_object-odb.hxx"

namespace das {
    namespace tpl {

        void
        DbBundle::flush_session() {
            typedef odb::session::database_map database_map;
            typedef odb::session::type_map type_map;
            typedef odb::session::object_map<DasObject> object_map;

            shared_ptr<odb::session> s = lock_session(true);

            database_map::iterator db_it(s->map().find(db_.get()));

            if (db_it == s->map().end()) {
                DAS_LOG_DBG("DAS debug INFO: session empty");
            } else {
                for (type_map::iterator type_it = db_it->second.begin();
                        type_it != db_it->second.end();
                        type_it++) {
                    object_map & obj_map(static_cast<object_map&> (*type_it->second));
                    for (object_map::iterator obj_it = obj_map.begin();
                            obj_it != obj_map.end();
                            obj_it++) {
                        obj_it->second->update();
                    }
                }
            }
        }
    }
}
