#include "../../../include/internal/storage_engine.ipp"
#include "../../../include/das_object.hpp"



namespace das {
    namespace tpl {

        class VoidStorageTransaction : public StorageTransaction {
        public:

            virtual void add(DasObject *ptr) {
            };

            virtual void add(DasObject *ptr, const Extension &e) {
            };

            virtual void save(const std::string &path) {
            };

            virtual void save() {
            };

            virtual void commit() {
            };

            virtual void rollback() {
            };

            VoidStorageTransaction(TransactionBundle &tb) {
            }
        };

        class VoidStorageAccess : public StorageAccess {
        public:
            virtual size_t read(ColumnFromFile* col, column_buffer_ptr buffer, size_t offset, size_t count) {
                return 0;
            }

            virtual size_t write(ColumnFromFile* col, column_buffer_ptr buffer, size_t offset, size_t count) {
                return 0;
            }

            virtual size_t read(ImageFromFile* col, void *buffer, size_t offset, size_t count) {
                return 0;
            }

            virtual size_t write(ImageFromFile* col, void *buffer, size_t offset, size_t count) {
                return 0;
            }

            virtual ~VoidStorageAccess() {
            }

            VoidStorageAccess(DasObject *obj) : StorageAccess(obj){
            }

        };

        shared_ptr<StorageTransaction>
        StorageTransaction::create(const std::string &db_alias, TransactionBundle &tb) {
            return shared_ptr<StorageTransaction>(new VoidStorageTransaction(tb));
        }

        StorageAccess*
        StorageAccess::create(const std::string &db_alias, DasObject *obj) {
            return new VoidStorageAccess(obj);
        }

    }

}

