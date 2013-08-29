#include "internal/storage_engine.ipp"
#include "das_object.hpp"

#include "internal/storage_engine_raw.hpp"

namespace das {

        shared_ptr<StorageTransaction>
        StorageTransaction::create(const std::string &db_alias, TransactionBundle &tb) {
            return shared_ptr<StorageTransaction>(new RawStorageTransaction(tb));
        }

        StorageAccess*
        StorageAccess::create(const std::string &db_alias, DasObject *obj) {
            return new RawStorageAccess(obj,DatabaseConfig::database(db_alias));
        }
}

