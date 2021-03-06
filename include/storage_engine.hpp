#ifndef STORAGE_ENGINE_HPP
#define	STORAGE_ENGINE_HPP
#include <iostream>
#include <map>
#include <utility> // std::pair
#include <vector>
#include <algorithm>
#include <tr1/memory>

#include "ddl/column.hpp"
#include "ddl/image.hpp"
#include "ddl/info.hpp"
#include "internal/db_bundle.hpp"
#include "internal/array.hpp"
#include "internal/utility.hpp"
#include "internal/database_config.hpp"

#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
//#include "das_object.hpp"

typedef unsigned long long chunk_t;

using namespace std;
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;

class DasObject;

namespace das {

    class StorageTransaction;

    typedef std::pair<std::string, int> Extension;

    class StorageAccess {
    public:

        typedef boost::variant<
        signed char,
        char,
        short,
        int,
        long long,
        float,
        double,
        bool,
        std::string,
        boost::posix_time::ptime
        > keyword_type;

        typedef boost::variant<
        long long&, // das_id
        std::string&, // name, dbUserId
        short&, // version
        boost::posix_time::ptime&, // creationDate
        boost::optional<signed char>&,
        boost::optional<char>&,
        boost::optional<short>&,
        boost::optional<int>&,
        boost::optional<long long>&,
        boost::optional<float>&,
        boost::optional<double>&,
        boost::optional<bool>&,
        boost::optional<std::string>&
        > keyword_type_ref;

        typedef boost::unordered_map<std::string, keyword_type_ref> keyword_map;

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
        ColumnArrayBuffer<char>,
        ColumnArrayBuffer<short>,
        ColumnArrayBuffer<int>,
        ColumnArrayBuffer<long long>,
        ColumnArrayBuffer<float>,
        ColumnArrayBuffer<double>,
        ColumnArrayBuffer<bool>,
        ColumnArrayBuffer<unsigned char>,
        ColumnArrayBuffer<unsigned short>,
        ColumnArrayBuffer<unsigned int>,
        ColumnArrayBuffer<std::string>
        > column_array_buffer_ptr;

        typedef boost::variant<
        char*,
        short*,
        int*,
        float*,
        double*
        > image_buffer_ptr;




        virtual size_t read_column(
                const std::string &col_name,
                Column* col,
                column_buffer_ptr buffer,
                size_t offset,
                size_t count
                ) = 0;

        virtual size_t read_column_array(
                const std::string &col_name,
                Column* col,
                column_array_buffer_ptr &buffer,
                size_t offset,
                size_t count
                ) = 0;

        virtual void flush_buffer(
                const std::string &col_name,
                Column* col
                ) = 0;


        virtual size_t read_image(
                Image* img,
                image_buffer_ptr buffer,
                const das::TinyVector<int, 11> &offset,
                const das::TinyVector<int, 11> &count,
                const das::TinyVector<int, 11> &stride
                ) = 0;

        virtual Column* create_column(
                const std::string &type,
                const std::string &array_size) = 0;

        virtual Image* create_image(
                const std::string &pixel_type) = 0;

        virtual void flush_buffer(Image* img) = 0;

        virtual bool release(Column *cff) = 0;

        virtual bool release(Image *iff) = 0;

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
        ColumnArray<T, Rank> get_column_array(const string &col_name, size_t start, size_t length);

        template <typename T, int Rank>
        void append_column_array(const string &col_name, ColumnArray<T, Rank> &a);

        /*template <typename T, int Rank>
        Array<T, Rank> get_image();*/

        template <typename T, int Rank>
        Array<T, Rank> get_image(
                const das::Range &r0,
                const das::Range &r1,
                const das::Range &r2,
                const das::Range &r3,
                const das::Range &r4,
                const das::Range &r5,
                const das::Range &r6,
                const das::Range &r7,
                const das::Range &r8,
                const das::Range &r9,
                const das::Range &r10
                );


        template <typename T, int Rank>
        void set_image(Array<T, Rank> &i);

        template <typename T, int Rank>
        void append_tiles(Array<T, Rank> &i);

        static
        StorageAccess*
        create(const std::string &db_alias, DasObject *obj);

        const DatabaseInfo &info;
    protected:

        StorageAccess(DasObject *obj, const DatabaseInfo &i) : obj_(obj), info(i) {
        }

        static
        const keyword_map&
        get_keywords(DasObject *ptr);

        static
        void
        get_columns_from_file(DasObject *ptr,
                std::map<std::string, Column*> &map);

        static
        void
        column_from_file(DasObject *ptr,
                const std::string &col_name,
                const Column &cf);

        static
        Column*
        column_from_file(DasObject *ptr,
                const std::string &col_name);

        static
        Image *
        image_ptr(DasObject *ptr);

        static
        void
        image_ptr(DasObject *ptr, const Image &iff);

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
        das::StorageAccess*
        storage_access(DasObject *ptr);

        static
        const boost::unordered_map<std::string, StorageAccess::keyword_type_ref>&
        get_keywords(DasObject *ptr) {
            return StorageAccess::get_keywords(ptr);
        }

        static
        void
        get_columns_from_file(DasObject *ptr,
                std::map<std::string, Column*> &map) {
            StorageAccess::get_columns_from_file(ptr, map);
        }

        static
        void
        column_from_file(DasObject *ptr,
                const std::string &col_name,
                const Column &cf) {
            StorageAccess::column_from_file(ptr, col_name, cf);
        }

        static
        Column*
        column_from_file(DasObject *ptr,
                const std::string &col_name) {
            return StorageAccess::column_from_file(ptr, col_name);
        }

        static
        Image *
        image_ptr(DasObject *ptr) {
            return StorageAccess::image_ptr(ptr);
        }

        static
        void
        image_ptr(DasObject *ptr, const Image &iff) {
            StorageAccess::image_ptr(ptr, iff);
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



#endif	/* STORAGE_ENGINE_HPP */

