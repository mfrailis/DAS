#ifndef STORAGE_ENGINE_RAW_HPP
#define	STORAGE_ENGINE_RAW_HPP
#include <vector>
#include "../../../include/storage_engine.hpp"


namespace das {
    namespace tpl {
        typedef std::pair<std::string, int> Extension;

        class RawStorageTransaction : public  StorageTransaction{
        public:

            virtual void add(DasObject *ptr);
            virtual void add(DasObject *ptr, const Extension &e);
            virtual void save(const std::string &path){save();}
            virtual void save();
            virtual void commit(){}
            virtual void rollback(){}
            virtual ~RawStorageTransaction(){}

            RawStorageTransaction (TransactionBundle &tb);
        private:
            std::vector<DasObject*> objs_;
            TransactionBundle& tb_;
        };

        class RawStorageAccess : public StorageAccess{
        public:
            virtual size_t read(ColumnFromFile* col, column_buffer_ptr buffer, size_t offset, size_t count);
            virtual size_t write(ColumnFromFile* col, column_buffer_ptr buffer, size_t offset, size_t count);

            virtual size_t read(ImageFromFile* col, void *buffer, size_t offset, size_t count){return 0;}
            virtual size_t write(ImageFromFile* col, void *buffer, size_t offset, size_t count){return 0;}
            
        };
        
        
    }
}


#endif	/* STORAGE_ENGINE_RAW_HPP */