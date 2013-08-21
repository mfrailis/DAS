#ifndef STORAGE_ENGINE_IPP
#define	STORAGE_ENGINE_IPP
#include "../storage_engine.hpp"
#include "../ddl/info.hpp"
#include "../das_object.hpp"
#include "column_buffer.ipp"
#include "image_buffer.ipp"
#include "log.hpp"
#include <boost/variant.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>





namespace das {
    namespace tpl {

        template<typename T>
        class ArrayDeleter {
        public:

            void operator() (T* t) {
                delete [] t;
            }
        };

        template<typename T>
        class StorageAccess_get_column : public boost::static_visitor<Array<T> > {
        public:

            StorageAccess_get_column(StorageAccess *acc, const std::string &col_name, ColumnFromFile *c, size_t start, size_t length)
            : sa_(acc), c_(c), s_(start), l_(length), cn_(col_name) {
            }

            template<typename U >
            Array<T> operator()(U& native_type) const {
                using boost::interprocess::unique_ptr;

                unique_ptr<T, ArrayDeleter<T> > buffer(new T[l_]);
                T* b = &buffer.get()[0];
                T* e = &buffer.get()[l_]; //first index not valid
                size_t count = 0;
                if (c_->file_size() > s_) {
                    size_t size = c_->file_size() - s_;
                    unique_ptr<U, ArrayDeleter<U> > buff_temp(new U[size]);
                    StorageAccess::column_buffer_ptr tb = buff_temp.get();

                    count = sa_->read(cn_, c_, tb, s_, size);

                    if (count < size)
                        throw io_exception();

                    for (size_t i = 0; i < count; ++i)
                        buffer.get()[i] = buff_temp.get()[i];
                }

                b += count;
                size_t missing = 0;
                if (count < l_) {
                    T* cached = c_->buffer().copy(b, e, 0);
                    missing = e - cached;
                }
                if (missing > 0) {
                    throw io_exception();
                }

                return Array<T>(buffer.release(), l_, das::deleteDataWhenDone);
            }

            Array<T> operator()(T& native_type) const {
                using boost::interprocess::unique_ptr;

                unique_ptr<T, ArrayDeleter<T> > buffer(new T[l_]);
                T* b = &buffer.get()[0];
                T* e = &buffer.get()[l_]; //first index not valid

                size_t count = 0;

                // check if we need to read some (or all) data from file
                if (c_->file_size() > s_) {
                    /*
                     * calculate the amount of date to read from file:
                     *  min(file_size - offset, length) 
                     */
                    size_t to_read = c_->file_size() - s_;
                    to_read = to_read > l_ ? l_ : to_read;

                    StorageAccess::column_buffer_ptr tb = buffer.get();

                    count = sa_->read(cn_, c_, tb, s_, to_read);

                    if (count < to_read)
                        throw io_exception();

                }
                b += count;
                size_t missing = 0;
                if (count < l_) {
                    T* cached = c_->buffer().copy(b, e, 0);
                    missing = e - cached;
                }
                if (missing > 0)
                    throw io_exception();

                return Array<T>(buffer.release(), l_, das::deleteDataWhenDone);
            }

            Array<T> operator()(std::string & native_type) const {
                throw das::not_implemented();
            }

        private:
            StorageAccess *sa_;
            ColumnFromFile *c_;
            size_t s_;
            size_t l_;
            const std::string &cn_;
        };

        template<>
        class StorageAccess_get_column<std::string> : public boost::static_visitor<Array<std::string> > {
        public:

            StorageAccess_get_column(StorageAccess *acc, const std::string &col_name, ColumnFromFile *c, size_t start, size_t length)
            : sa_(acc), c_(c), s_(start), l_(length), cn_(col_name) {
            }

            template<typename T >
            Array<std::string> operator()(T & native_type) const {
                throw das::not_implemented();
            }

            Array<std::string> operator()(std::string &native_type) const {
                using boost::interprocess::unique_ptr;

                unique_ptr<std::string, ArrayDeleter<std::string> > buffer(new std::string[l_]);
                std::string* b = &buffer.get()[0];
                std::string* e = &buffer.get()[l_]; //first not valid index
                size_t count = 0;

                // check if we need to read some (or all) data from file
                if (c_->file_size() > s_) {
                    /*
                     * calculate the amount of data to read from file:
                     *  min(file_size - offset, length) 
                     */
                    size_t to_read = c_->file_size() - s_;
                    to_read = to_read > l_ ? l_ : to_read;
                    StorageAccess::column_buffer_ptr tb = buffer.get();

                    count = sa_->read(cn_, c_, tb, s_, to_read);

                    if (count < to_read)
                        throw io_exception();

                }
                b += count;
                size_t missing = 0;
                if (count < l_) {
                    std::string* cached = c_->buffer().copy(b, e, 0);
                    missing = e - cached;
                }
                if (missing > 0)
                    throw io_exception();

                return Array<std::string>(buffer.release(), l_, das::deleteDataWhenDone);
            }

        private:
            StorageAccess *sa_;
            ColumnFromFile *c_;
            size_t s_;
            size_t l_;
            const std::string &cn_;
        };

        template<typename T>
        Array<T>
        StorageAccess::get_column(const string& col_name, size_t start, size_t length) {
            ColumnFromFile *c = obj_->column_from_file(col_name); //throw if bad name
            if (!c || length == 0) { //no data, throw exception
                throw empty_column();
            }
            column_type type = DdlInfo::get_instance(db_alias(obj_))->
                    get_column_info(type_name(obj_), col_name).type_var_;

            return boost::apply_visitor(StorageAccess_get_column<T>(this, col_name, c, start, length), type);
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
                    !obj_->storage_access()->info.buffered_data()) {
                obj_->storage_access()->flush_buffer(col_name, c);
            }
        }

        template <typename T, int Rank>
        void
        StorageAccess::append_tiles(Array<T, Rank> &t) {
            ImageFromFile *i = obj_->image_from_file(); //throw if type does not provide image data

            const ImageInfo &info = DdlInfo::get_instance()->get_image_info(obj_->type_name_);

            if (!i) {
                ImageFromFile iff(info.type);
                obj_->image_from_file(iff);
                i = obj_->image_from_file();
            }

            /*
             * considerations as for append coulmn.
             */
            i->buffer().append(t, info.dimensions);

            if (!obj_->storage_access()->buffered_only() &&
                    !obj_->storage_access()->info.buffered_data()) {
                obj_->storage_access()->flush_buffer(i);
            }

        }

        /*                
                 template <typename T, int Rank>
                 Array<T,Rank> get_image();

                 template <typename T, int Rank>
                 void set_image(Array<T,Rank> &i);
         */




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