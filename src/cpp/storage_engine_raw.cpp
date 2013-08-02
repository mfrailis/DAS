#include "include/storage_engine_raw.hpp"
#include "../../include/exceptions.hpp"
#include <iostream>
#include <fstream>
#include <time.h> 
#include "../../include/ddl/info.hpp"
#include "../../include/das_object.hpp"
#include "../../include/internal/storage_engine.ipp"
using namespace std;

namespace das {
    namespace tpl {

        class RawStorageAccess_read_column : public boost::static_visitor<size_t> {
        public:

            RawStorageAccess_read_column(size_t offset, size_t count, const char* path)
            : o_(offset), c_(count), path_(path) {
            }

            size_t operator() (std::string* buff) const {
                throw not_implemented();
            }

            template<typename T>
            size_t operator() (T* buff) const {

                ifstream s;
                s.open(path_);
                T skip;
                while (s.good() && o_-- > 0)
                    s >> skip;

                size_t count = 0;

                while (s.good() && c_-- > 0)
                    s >> buff[count++];

                s.close();
                return count;
            }
            mutable size_t o_;
            mutable size_t c_;
            const char *path_;
        };

        class RawStorageAccess_write_column : public boost::static_visitor<size_t> {
        public:

            RawStorageAccess_write_column(size_t offset, size_t count, const char* path)
            : o_(offset), c_(count), path_(path) {
            }

            size_t operator() (std::string* buff) const {
                throw not_implemented();
            }

            template<typename T>
            size_t operator() (T* buff) const {
                fstream s;
                s.open(path_);

                if (o_ != -1) {
                    T skip;
                    while (s.good() && o_-- > 0)
                        s >> skip;
                } else {
                    s.seekp(0, std::ios_base::end);
                }

                size_t count = 0;
                while (s.good() && c_-- > 0)
                    s << buff[count++] << endl;

                s.close();
                return count;
            }
            mutable size_t o_;
            mutable size_t c_;
            const char *path_;
        };

        size_t
        RawStorageAccess::read(ColumnFromFile* c, column_buffer_ptr buffer, size_t offset, size_t count) {
            if (c->temp_path() != "")
                return boost::apply_visitor(
                    RawStorageAccess_read_column(offset, count, c->temp_path().c_str()),
                    buffer);
            else
                if (c->fname() != "")
                return boost::apply_visitor(
                    RawStorageAccess_read_column(offset, count, c->fname().c_str()),
                    buffer);

            return 0;
        }

        size_t
        RawStorageAccess::write(ColumnFromFile* c, column_buffer_ptr buffer, size_t offset, size_t count) {
            if (c->temp_path() != "")
                return boost::apply_visitor(
                    RawStorageAccess_write_column(offset, count, c->temp_path().c_str()),
                    buffer);
            else
                if (c->fname() != "")
                return boost::apply_visitor(
                    RawStorageAccess_write_column(offset, count, c->fname().c_str()),
                    buffer);
            else {
                std::string path("/mnt/DBs/data/");
                path += type_name(obj_);
                path += clock();
                c->temp_path(path);
                return boost::apply_visitor(
                        RawStorageAccess_write_column(offset, count, path.c_str()),
                        buffer);
            }
        }

        RawStorageTransaction::RawStorageTransaction(TransactionBundle &tb)
        : tb_(tb) {
        }

        void
        RawStorageTransaction::add(DasObject *ptr) {
            objs_.push_back(ptr);
        }

        void
        RawStorageTransaction::add(DasObject *ptr, const Extension &e) {
            objs_.push_back(ptr);
        }

        class RawStorageTransaction_flushColumn : public boost::static_visitor<void> {
        public:

            RawStorageTransaction_flushColumn(
            DasObject *obj,
                    const std::string &col_name,
                    const std::string &type_name,
                    ColumnFromFile *c,
                    RawStorageAccess *s)
            : obj_(obj), c_(c), s_(s), t_n_(type_name), c_n_(col_name) {
                if (c->temp_path() == "") {
                    std::string path("/mnt/DBs/data/");
                    path += type_name;
                    path += "_";
                    path += col_name;
                    path += "_";
                    path += clock();
                    c->temp_path(path);
                }
            }

            void flush() {
                column_type ct = DdlInfo::get_instance()->get_column_info(t_n_, c_n_).type_var_;
                boost::apply_visitor(*this,ct);
            }

            void operator() (std::string &native_type) {
                throw not_implemented();
            }

            template<typename T>
            void operator() (T &native_type) {
                typedef std::vector<std::pair<T*, size_t> > buckets_type;
                buckets_type bks = c_->buffer().buckets<T>();
                for (typename buckets_type::iterator it = bks.begin(); it != bks.end(); ++it) {
                    StorageAccess::column_buffer_ptr buffer = it->first;
                    s_->write(c_, buffer, -1, it->second);
                }

            }

        private:
            DasObject *obj_;
            ColumnFromFile* c_;
            RawStorageAccess* s_;
            std::string t_n_, c_n_;
        };

        void
        RawStorageTransaction::save() {
            for (std::vector<DasObject*>::iterator obj_it = objs_.begin();
                    obj_it != objs_.end(); ++obj_it) {
                std::map<std::string, ColumnFromFile*> map;
                DasObject* obj = *obj_it;
                StorageTransaction::get_columns_from_file(obj,map);

                for (std::map<std::string, ColumnFromFile*>::iterator m_it = map.begin();
                        m_it != map.end(); ++m_it) {
                    RawStorageAccess *rsa = dynamic_cast<RawStorageAccess*>(storage_access(obj));
                    RawStorageTransaction_flushColumn f(
                            obj,
                            type_name(obj),
                            m_it->first,
                            m_it->second,
                            rsa);
                    f.flush();
                }

            }
        }



    }

}