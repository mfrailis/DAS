#ifndef STORAGE_ENGINE_HPP
#define	STORAGE_ENGINE_HPP
#include <iostream>
#include <map>
#include <utility> // std::pair
#include <vector>
#include <algorithm>
#include <tr1/memory>
#include <boost/variant.hpp>
#include "ddl/column.hpp"
#include "ddl/image.hpp"
#include "ddl/info.hpp"
#include "internal/db_bundle.hpp"
#include "internal/array.hpp"
#include "internal/database_config.hpp"

using namespace std;
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;

class DasObject;

namespace das {
    namespace tpl {

        class StorageTransaction;

        typedef std::pair<std::string, int> Extension;

        class StorageAccess {
        public:
            typedef boost::variant<
            char*,
            short*,
            int*,
            long long*,
            float*,
            double*,
            bool*,
            unsigned char*,
            unsigned short*,
            unsigned int*,
            std::string*
            > column_buffer_ptr;

            typedef boost::variant<
            char*,
            short*,
            int*,
            float*,
            double*
            > image_buffer_ptr;




            virtual size_t read(
                    const std::string &col_name,
                    ColumnFromFile* col,
                    column_buffer_ptr buffer,
                    size_t offset,
                    size_t count
                    ) = 0;

            virtual void flush_buffer(
                    const std::string &col_name,
                    ColumnFromFile* col
                    ) = 0;


            virtual size_t read(
                    ImageFromFile* col,
                    image_buffer_ptr buffer,
                    const das::TinyVector<int,11> &offset,
                    const das::TinyVector<int,11> &count,
                    const das::TinyVector<int,11> &stride
                    ) = 0;

            virtual size_t flush_buffer(ImageFromFile* img) = 0;

            virtual bool buffered_only() {
                return true;
            }

            virtual ~StorageAccess() {
            };

            template <typename T>
            Array<T> get_column(const string &col_name, size_t start, size_t length);

            template <typename T>
            void append_column(const string &col_name, Array<T> &a);

            template <typename T, int Rank>
            Array<T,Rank> get_image();
            
            template <typename T, int Rank>
            Array<T,Rank> 
            get_image(
            const TinyVector<int,Rank> &offset,
            const TinyVector<int,Rank> &count,
            const TinyVector<int,Rank> &stride
            );

            template <typename T, int Rank>
            void set_image(Array<T,Rank> &i);
            
            template <typename T, int Rank>
            void append_tiles(Array<T,Rank> &i);
            
            static
            StorageAccess*
            create(const std::string &db_alias, DasObject *obj);

            const DatabaseInfo &info;
        protected:

            StorageAccess(DasObject *obj, const DatabaseInfo &i) : obj_(obj), info(i) {
            }

            static
            void
            get_keywords(DasObject *ptr,
                    std::map<std::string, keyword_type> &m);

            static
            void
            get_columns_from_file(DasObject *ptr,
                    std::map<std::string, ColumnFromFile*> &map);

            static
            void
            column_from_file(DasObject *ptr,
                    const std::string &col_name,
                    const ColumnFromFile &cf);

            static
            ColumnFromFile*
            column_from_file(DasObject *ptr,
                    const std::string &col_name);

            static
            const ImageFromFile *
            image_from_file(DasObject *ptr);

            static
            void
            image_from_file(DasObject *ptr, const ImageFromFile &iff);

            static
            const std::string&
            type_name(DasObject *ptr);

            static
            const std::string&
            db_alias(DasObject *ptr);

            DasObject *obj_;

        private:
            StorageAccess();
            friend class StorageTransaction;
        };

        class StorageTransaction {
        public:

            virtual void add(DasObject *ptr) = 0;
            virtual void add(DasObject *ptr, const Extension &e) = 0;
            virtual void save(const std::string &path) = 0;
            virtual void save() = 0;
            virtual void commit() = 0;
            virtual void rollback() = 0;

            virtual ~StorageTransaction() {
            }
            static
            shared_ptr<StorageTransaction>
            create(const std::string &db_alias, TransactionBundle &tb);

        protected:
            static
            das::tpl::StorageAccess*
            storage_access(DasObject *ptr);

            static
            void
            get_keywords(DasObject *ptr,
                    std::map<std::string, keyword_type> &m) {
                StorageAccess::get_keywords(ptr, m);
            }

            static
            void
            get_columns_from_file(DasObject *ptr,
                    std::map<std::string, ColumnFromFile*> &map) {
                StorageAccess::get_columns_from_file(ptr, map);
            }

            static
            void
            column_from_file(DasObject *ptr,
                    const std::string &col_name,
                    const ColumnFromFile &cf) {
                StorageAccess::column_from_file(ptr, col_name, cf);
            }

            static
            ColumnFromFile*
            column_from_file(DasObject *ptr,
                    const std::string &col_name) {
                return StorageAccess::column_from_file(ptr, col_name);
            }

            static
            const ImageFromFile *
            image_from_file(DasObject *ptr) {
                return StorageAccess::image_from_file(ptr);
            }

            static
            void
            image_from_file(DasObject *ptr, const ImageFromFile &iff) {
                StorageAccess::image_from_file(ptr, iff);
            }

            static
            const std::string&
            type_name(DasObject *ptr) {
                return StorageAccess::type_name(ptr);
            }

            static
            const std::string&
            db_alias(DasObject *ptr) {
                return StorageAccess::db_alias(ptr);
            }
        };
    }
}



#endif	/* STORAGE_ENGINE_HPP */

