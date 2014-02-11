#ifndef STORAGE_ENGINE_BLOB_HPP
#define	STORAGE_ENGINE_BLOB_HPP
#include <vector>
#include <fstream>
#include <sstream>
#include <exception>
#include "../storage_engine.hpp"


namespace das {

    class BlobStorageTransaction : public StorageTransaction {
    public:

        virtual void add(DasObject *ptr);

        virtual void add(DasObject *ptr, const Extension &e);
        
        virtual void save(const std::string &path) {
            save();
        }

        virtual void save();

        virtual void commit() {
        }

        virtual void rollback() {
        }

        virtual ~BlobStorageTransaction() {
        }

        BlobStorageTransaction(TransactionBundle &tb) {
        }
    private:
        std::list<DasObject*> objs_;

    };

    class BlobStorageAccess : public StorageAccess {
        //continua da qui //
    public:

        BlobStorageAccess(DasObject *obj, const DatabaseInfo &i) : StorageAccess(obj, i) {
        }

        virtual size_t read_column(
                const std::string &col_name,
                Column* col,
                column_buffer_ptr buffer,
                size_t offset,
                size_t count
                );

        virtual size_t read_column_array(
                const std::string &col_name,
                Column* col,
                column_array_buffer_ptr &buffer,
                size_t offset,
                size_t count
                );

        virtual void flush_buffer(
                const std::string &col_name,
                Column* col
                );


        virtual size_t read_image(
                Image* img,
                image_buffer_ptr buffer,
                const das::TinyVector<int, 11> &offset,
                const das::TinyVector<int, 11> &count,
                const das::TinyVector<int, 11> &stride
                );

        virtual Column* create_column(
                const std::string &type,
                const std::string &array_size) {
            return new ColumnFromBlob(type, array_size);
        }
        
        virtual Image* create_image(
                const std::string &pixel_type){
            return new ImageBlob(pixel_type);
        }
        
        virtual void flush_buffer(Image* img);

        virtual bool release(Column *cff) {
            return true;
        }

        virtual bool release(Image *iff) {
            return true;
        }

        virtual bool buffered_only() {
            return true;
        }

        virtual ~BlobStorageAccess() {
        }


    private:
        BlobStorageAccess();
        friend class StorageTransaction;
    };


}



#endif	/* STORAGE_ENGINE_BLOB_HPP */

