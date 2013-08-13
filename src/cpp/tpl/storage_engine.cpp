#include "../../../include/internal/storage_engine.ipp"
#include "../../../include/das_object.hpp"

#include "../../../include/internal/storage_engine_raw.hpp"

namespace das {
    namespace tpl {

        shared_ptr<StorageTransaction>
        StorageTransaction::create(const std::string &db_alias, TransactionBundle &tb) {
            return shared_ptr<StorageTransaction>(new RawStorageTransaction(tb));
        }

        StorageAccess*
        StorageAccess::create(const std::string &db_alias, DasObject *obj) {
            return new RawStorageAccess(obj,DatabaseConfig::database(db_alias));
        }

    }

}

