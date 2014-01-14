#include "internal/storage_engine.ipp"
#include "das_object.hpp"
#include "ddl/info.hpp"
#include "internal/storage_engine_raw.hpp"
#include "internal/storage_engine_blob.hpp"

namespace das {

        shared_ptr<StorageTransaction>
        StorageTransaction::create(const std::string &db_alias, TransactionBundle &tb) {
            return shared_ptr<StorageTransaction>(new RawStorageTransaction(tb));
        }

        StorageAccess*
        StorageAccess::create(const std::string &db_alias, DasObject *obj) {
            const std::string& store_as = DdlInfo::get_instance(db_alias)->
            get_type_info(obj->type_name_).get_store_as();
            if(store_as == "blob")
                return new BlobStorageAccess(obj,DatabaseConfig::database(db_alias));
            else
                return new RawStorageAccess(obj,DatabaseConfig::database(db_alias));
        }
}

