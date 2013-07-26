#ifndef STORAGE_ENGINE_HPP
#define	STORAGE_ENGINE_HPP
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tr1/memory>
#include <boost/variant.hpp>
#include "column.hpp"

using namespace std;
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;

typedef boost::variant<
char,
short,
int,
long long,
float,
double,
bool,
unsigned char,
unsigned int,
std::string
>
keyword_type;

class DasObject;

namespace das {
    namespace tpl {

        class StorageEngine {
        public:
            static
            shared_ptr<StorageEngine>
            get_engine(const std::string &identifier);

            virtual void add(shared_ptr<DasObject> &ptr, const Extension &e) = 0;
            virtual void save() = 0;
            virtual void commit() = 0;
            virtual void rollback() = 0;
            virtual ~StorageEngine() = 0;

            template <typename T>
            static Array get_column(DasObject *ptr, const string &col_name/*, other args*/);

            template <typename T>
            static void append_column(DasObject *ptr, const string &col_name, Array &a);

            template <typename T>
            static Image get_image(DasObject *ptr/*, other args*/);

            template <typename T>
            static void set_image(DasObject *ptr, Image);

            static init_engine(const std::string &db_alias);

        protected:
            StorageEngine(shared_ptr<Database> &db);

            static
            void
            get_keywords(shared_ptr<DasObject> &ptr,
                    std::map<std::string, keyword_type> &map) {
                ptr->get_keywords(m);
            }

            static
            const ColumnFromFile *
            column_from_file(shared_ptr<DasObject> &ptr,
                    const std::string &col_name) {
                return ptr->column_file(name);
            }

            static
            void
            column_from_file(shared_ptr<DasObject> &ptr,
                    const std::string &col_name,
                    const ColumnFromFile &cf) {
                ptr->column_file(name, cf);
            }

            static
            const ImageFromFile *
            image_from_file(shared_ptr<DasObject> &ptr);

            static
            void
            image_from_file(shared_ptr<DasObject> &ptr, const ImageFromFile &ifff);

            const std::string&
            type_name(shared_ptr<DasObject> &ptr) {
                return ptr->type_name_;
            }

        };
    }
}



#endif	/* STORAGE_ENGINE_HPP */

