#ifndef STORAGE_ENGINE_IPP
#define	STORAGE_ENGINE_IPP
#include "../storage_engine.hpp"
#include "../ddl/info.hpp"
#include "../das_object.hpp"
#include "column_buffer.ipp"
#include "log.hpp"
#include <boost/variant.hpp>

namespace das {
    namespace tpl {

        template<typename T>
        class StorageAccess_get_column : public boost::static_visitor<Array<T> > {
        public:

            StorageAccess_get_column(StorageAccess *acc, ColumnFromFile *c, size_t start, size_t length)
            : sa_(acc), c_(c), s_(start), l_(length) {
            }

            template<typename U >
            Array<T> operator()(U& native_type) const {
                T* buffer = new T[l_];
                T* b = &buffer[0];
                T* e = &buffer[l_]; //first index not valid
                size_t count = 0;
                if (c_->file_size() > s_) {
                    size_t size = c_->file_size() - s_;
                    U *buff_temp = new U[size];
                    StorageAccess::column_buffer_ptr tb = buff_temp;

                    count = sa_->read(c_, tb, s_, size);

                    for (size_t i = 0; i < count; ++i)
                        buffer[i] = buff_temp[i];

                    delete[] buff_temp;


                    /* 
                     * data missing in the file, avoid gaps between data not
                     * loaded from file and memory buffer returning only the data
                     * succesfully loaded from file
                     */
                    if (count < size)
                        return Array<T>(buffer, count, das::deleteDataWhenDone);
                }
                b += count;
                size_t missing = 0;
                if (count < l_) {
                    T* cached = c_->buffer().copy(b, e, 0);
                    missing = e - cached;
                }
                return Array<T>(buffer, l_ - missing, das::deleteDataWhenDone);
            }

            Array<T> operator()(T& native_type) const {
                T* buffer = new T[l_];
                T* b = &buffer[0];
                T* e = &buffer[l_]; //first not valid index
                size_t count = 0;
                
                // check if we need to read some (or all) data from file
                if (c_->file_size() > s_) { 
                    /*
                     * calculate the amount of date to read from file:
                     *  min(file_size - offset, length) 
                     */
                    size_t to_read = c_->file_size() - s_;
                    to_read = to_read > l_ ? l_ : to_read;
                    StorageAccess::column_buffer_ptr tb = buffer;
                    count = sa_->read(c_, tb, s_, to_read);
                    if (count < to_read)
                        return Array<T>(buffer, count, das::deleteDataWhenDone);
                }
                b += count;
                size_t missing = 0;
                if (count < l_) {
                    T* cached = c_->buffer().copy(b, e, 0);
                    missing = e - cached;
                }
                return Array<T>(buffer, l_ - missing, das::deleteDataWhenDone);
            }

            Array<T> operator()(std::string & native_type) const {
                throw das::not_implemented();
            }

        private:
            StorageAccess *sa_;
            ColumnFromFile *c_;
            size_t s_;
            size_t l_;
        };

        template<>
        class StorageAccess_get_column<std::string> : public boost::static_visitor<Array<std::string> > {
        public:

            StorageAccess_get_column(ColumnFromFile *c, size_t start, size_t length)
            : c_(c), s_(start), l_(length) {
            }

            template<typename T >
            Array<std::string> operator()(T & native_type) const {
                throw das::not_implemented();
            }
        private:
            ColumnFromFile *c_;
            size_t s_;
            size_t l_;
        };

        template<typename T>
        Array<T>
        StorageAccess::get_column(const string& col_name, size_t start, size_t length) {
            ColumnFromFile *c = obj_->column_from_file(col_name); //throw if bad name
            if (!c || length == 0) { //no data, return an empty array
                return Array<T>();
            }
            column_type type = DdlInfo::get_instance(db_alias(obj_))->
                    get_column_info(type_name(obj_), col_name).type_var_;

            return boost::apply_visitor(StorageAccess_get_column<T>(this, c, start, length), type);
        }
        
        template<typename T>
        void
        StorageAccess::append_column(const string &col_name, Array<T> &a) {
            ColumnFromFile *c = obj_->column_from_file(col_name); //throw if bad name

            if (!c) {
                ColumnFromFile cff(DdlInfo::get_instance()->get_column_info(obj_->type_name_, col_name).type);
                obj_->column_from_file(col_name, cff);
                c = obj_->column_from_file(col_name);
            }

            /*
             * even if we have direct write set we have to flush the buffer 
             * anyway because the option may be just set and the buffer may
             * contain data. So we use the buffer.append<T> function that handles
             * type casting properly and then, if direct write is set, flush the
             * content on the file. Note that in case of type matching, we pass
             * around just references
             */
            c->buffer().append(a);            
            
            if (!obj_->storage_access()->buffered_only() &&
                    !obj_->storage_access()->info.buffered_data())
            {
                obj_->storage_access()->flush_buffer(col_name,c);
            }
        }

        inline
        void
        StorageAccess::get_keywords(DasObject *ptr,
                std::map<std::string, keyword_type> &m) {
            ptr->get_keywords(m);
        }

        inline
        void
        StorageAccess::get_columns_from_file(DasObject *ptr,
                std::map<std::string, ColumnFromFile*> &map) {
            return ptr->get_columns_from_file(map);
        }

        inline
        void
        StorageAccess::column_from_file(DasObject *ptr,
                const std::string &col_name,
                const ColumnFromFile &cf) {
            ptr->column_from_file(col_name, cf);
        }
        
        inline
        ColumnFromFile*
        StorageAccess::column_from_file(DasObject *ptr,
                const std::string &col_name) {
            return ptr->column_from_file(col_name);
        }

        inline
        const ImageFromFile *
        StorageAccess::image_from_file(DasObject *ptr) {
            return ptr->image_from_file();
        }

        inline
        void
        StorageAccess::image_from_file(DasObject *ptr, const ImageFromFile &iff) {
            ptr->image_from_file(iff);
        }

        inline
        const std::string&
        StorageAccess::type_name(DasObject *ptr) {
            return ptr->type_name_;
        }

        inline
        const std::string&
        StorageAccess::db_alias(DasObject *ptr) {
            return ptr->bundle_.alias();
        }

        inline
        das::tpl::StorageAccess*
        StorageTransaction::storage_access(DasObject *ptr) {
            return ptr->storage_access();
        }

    }
}

#endif