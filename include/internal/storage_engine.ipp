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
            column_type type = DdlInfo::get_instance()->
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

            if (!buffered_only() && !info.buffered_data()) {
                flush_buffer(col_name, c);
            }
        }

        template <typename T, int Rank>
        void
        StorageAccess::append_tiles(Array<T, Rank> &t) {
            ImageFromFile *i = obj_->image_from_file(); //throw if type does not provide image data

            if (!i) {
                ImageFromFile iff(DdlInfo::get_instance()->get_image_info(obj_->type_name_).type);
                obj_->image_from_file(iff);
                i = obj_->image_from_file();
            }

            /*
             * considerations as for append coulmn.
             */
            i->buffer().append(t);

            if (!buffered_only() && !info.buffered_data()) {
                flush_buffer(i);
            }

        }

        template <typename T, int Rank>
        void
        StorageAccess::set_image(Array<T, Rank> &t) {
            ImageFromFile iff(DdlInfo::get_instance()->get_image_info(obj_->type_name_).type);
            obj_->image_from_file(iff);
            ImageFromFile* i = obj_->image_from_file();


            /*
             * considerations as for append coulmn.
             */
            i->buffer().append(t);

            if (!buffered_only() && !info.buffered_data()) {
                flush_buffer(i);
            }

        }

        template <typename T, int Rank>
        class StorageAccess_get_image : public boost::static_visitor<Array<T, Rank> > {
        public:

            StorageAccess_get_image(
                    StorageAccess *acc,
                    ImageFromFile *i,
                    const TinyVector<int, 11> &offset,
                    const TinyVector<int, 11> &count,
                    const TinyVector<int, 11> &stride,
                    const TinyVector<int, Rank> &shape
                    )
            : sa_(acc), i_(i), off_(offset), cnt_(count), str_(stride), shape_(shape) {
            }

            template<typename U >
            Array<T, Rank> operator()(U& native_type) const {
                using boost::interprocess::unique_ptr;

                const size_t elems = cnt_[0] * cnt_[1] * cnt_[2] * cnt_[3] * cnt_[4] *
                        cnt_[5] * cnt_[6] * cnt_[7] * cnt_[8] * cnt_[9] * cnt_[10];

                unique_ptr<T, ArrayDeleter<T> > buffer(new T[elems]);
                T* b = &buffer.get()[0];

                size_t tiles_count = 0;

                // check if we need to read some (or all) tiles from file
                if (i_->file_tiles() > off_[0]) {
                    /*
                     * calculate the amount of tiles to read from file:
                     *  min(1+(file_tiles - offset[0] -1)/stride[0], count[0]) 
                     */

                    size_t tiles_to_read = 1;
                    tiles_to_read += ((i_->file_tiles() - 1) - off_[0]) / str_[0];
                    tiles_to_read = tiles_to_read > cnt_[0] ? cnt_[0] : tiles_to_read;


                    unique_ptr<U, ArrayDeleter<U> > buff_temp(new U[(elems / cnt_[0]) * tiles_to_read]);
                    StorageAccess::image_buffer_ptr tb = buff_temp.get();

                    TinyVector<int, 11> count(
                            tiles_to_read,
                            cnt_[1],
                            cnt_[2],
                            cnt_[3],
                            cnt_[4],
                            cnt_[5],
                            cnt_[6],
                            cnt_[7],
                            cnt_[8],
                            cnt_[9],
                            cnt_[10]);

                    size_t el_from_file = sa_->read(i_, tb, off_, count, str_);


                    if (el_from_file < tiles_to_read * cnt_[1] * cnt_[2] * cnt_[3] * cnt_[4] *
                            cnt_[5] * cnt_[6] * cnt_[7] * cnt_[8] * cnt_[9] * cnt_[10])
                        throw io_exception();

                    tiles_count = tiles_to_read;


                    T* buff_ptr = buffer.get();
                    U* tmp_buff_ptr = buff_temp.get();

                    for (size_t i = 0; i < (elems / cnt_[0]) * tiles_to_read; ++i)
                        buff_ptr[i] = tmp_buff_ptr[i];

                }

                TinyVector<int, 11> offset(off_);
                TinyVector<int, 11> count(cnt_);

                if (tiles_count != 0) {
                    offset[0] = (i_->file_tiles() - ((tiles_count - 1) * str_[0])) - off_[0];
                    offset[0] = str_[0] - offset[0];
                    count[0] -= tiles_count;
                }

                if (count[0] > 0) {
                    T* ptr = buffer.get() + (elems / cnt_[0]) * tiles_count;
                    size_t buff_elems = count[0] * count[1] * count[2] * count[3] *
                            count[4] * count[5] * count[6] * count[7] *
                            count[8] * count[9] * count[10];
                    DAS_DBG(if (ptr > buffer.get() + elems)
                            DAS_LOG_DBG("arithmetic buffer overflow in storage_engine.ipp");
                            );
                    size_t c = i_->buffer().copy(ptr, offset, count, str_);

                    DAS_DBG(
                    if (c > buff_elems)
                            DAS_LOG_DBG("arithmetic buffer overflow in storage_engine.ipp");
                            );

                    if (c != buff_elems) {
                        throw io_exception();
                    }
                }
                return Array<T, Rank>(buffer.release(), shape_, das::deleteDataWhenDone);

            }

            Array<T, Rank> operator()(T& native_type) const {
                using boost::interprocess::unique_ptr;

                size_t elems = cnt_[0] * cnt_[1] * cnt_[2] * cnt_[3] * cnt_[4] *
                        cnt_[5] * cnt_[6] * cnt_[7] * cnt_[8] * cnt_[9] * cnt_[10];

                unique_ptr<T, ArrayDeleter<T> > buffer(new T[elems]);
                T* b = &buffer.get()[0];

                size_t tiles_count = 0;

                // check if we need to read some (or all) tiles from file
                if (i_->file_tiles() > off_[0]) {
                    /*
                     * calculate the amount of tiles to read from file:
                     *  min(1+(file_tiles - offset[0] -1)/stride[0], count[0]) 
                     */

                    size_t tiles_to_read = 1;
                    tiles_to_read += ((i_->file_tiles() - 1) - off_[0]) / str_[0];
                    tiles_to_read = tiles_to_read > cnt_[0] ? cnt_[0] : tiles_to_read;

                    TinyVector<int, 11> count(
                            tiles_to_read,
                            cnt_[1],
                            cnt_[2],
                            cnt_[3],
                            cnt_[4],
                            cnt_[5],
                            cnt_[6],
                            cnt_[7],
                            cnt_[8],
                            cnt_[9],
                            cnt_[10]);

                    StorageAccess::image_buffer_ptr tb = buffer.get();

                    size_t el_from_file = sa_->read(i_, tb, off_, count, str_);


                    if (el_from_file < tiles_to_read * cnt_[1] * cnt_[2] * cnt_[3] * cnt_[4] *
                            cnt_[5] * cnt_[6] * cnt_[7] * cnt_[8] * cnt_[9] * cnt_[10])
                        throw io_exception();

                    tiles_count = tiles_to_read;

                }

                TinyVector<int, 11> offset(off_);
                TinyVector<int, 11> count(cnt_);


                if (tiles_count != 0) {
                    offset[0] = (i_->file_tiles() - ((tiles_count - 1) * str_[0])) - off_[0];
                    offset[0] = str_[0] - offset[0];
                    count[0] -= tiles_count;
                }

                if (count[0] > 0) {
                    T* ptr = buffer.get() + (elems / cnt_[0]) * tiles_count;
                    size_t buff_elems = count[0] * count[1] * count[2] * count[3] *
                            count[4] * count[5] * count[6] * count[7] *
                            count[8] * count[9] * count[10];
                    DAS_DBG(if (ptr > buffer.get() + elems)
                            DAS_LOG_DBG("arithmetic buffer overflow in storage_engine.ipp");
                            );
                    size_t c = i_->buffer().copy(ptr, offset, count, str_);

                    DAS_DBG(
                    if (c > buff_elems)
                            DAS_LOG_DBG("arithmetic buffer overflow in storage_engine.ipp");
                            );

                    if (c != buff_elems) {
                        throw io_exception();
                    }
                }
                return Array<T, Rank>(buffer.release(), shape_, das::deleteDataWhenDone);
            }


        private:
            StorageAccess *sa_;
            ImageFromFile *i_;
            const TinyVector<int, 11> &cnt_;
            const TinyVector<int, 11> &off_;
            const TinyVector<int, 11> &str_;
            const TinyVector<int, Rank> shape_;
        };

        template <typename T, int Rank>
        Array<T, Rank>
        StorageAccess::get_image(
                const TinyVector<int, Rank> &offset,
                const TinyVector<int, Rank> &count,
                const TinyVector<int, Rank> &stride
                ) {
            using boost::interprocess::unique_ptr;

            TinyVector<int, 11> cnt_(0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
            TinyVector<int, 11> off_(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            TinyVector<int, 11> str_(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            size_t elems = 1;
            for (size_t i = 0; i < Rank; ++i) {
                elems *= count[i];
                cnt_[i] = count[i];
                off_[i] = offset[i];
                str_[i] = stride[i];
            }
            DAS_LOG_DBG("IMAGE copy: " << elems << " elements");
            unique_ptr<T, ArrayDeleter<T> > buffer(new T[elems]);
            T* begin = buffer.get();


            ImageFromFile *i = obj_->image_from_file();
            if (i == NULL)
                throw das::empty_image();

            image_type type = DdlInfo::get_instance()->
                    get_image_info(type_name(obj_)).type_var_;

            return boost::apply_visitor(StorageAccess_get_image<T, Rank>(this, i, off_, cnt_, str_, count), type);

        }

        template <typename T, int Rank>
        Array<T, Rank>
        StorageAccess::get_image() {
            using boost::interprocess::unique_ptr;
            ImageFromFile *i_ = obj_->image_from_file();
            if (i_ == NULL)
                throw das::empty_image();

            if (Rank != i_->rank())
                throw das::bad_array_slice();

            TinyVector<int, 11> cnt_(0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
            TinyVector<int, 11> off_(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            TinyVector<int, 11> str_(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            TinyVector<int, Rank> shape;
            size_t elems = 1;
            for (size_t i = 0; i < Rank; ++i) {
                cnt_[i] = i_->extent(i);
                str_[i] = 1;
                shape[i] = cnt_[i];
                elems *= cnt_[i];
            }

            DAS_LOG_DBG("IMAGE copy: " << elems << " elements");
            unique_ptr<T, ArrayDeleter<T> > buffer(new T[elems]);
            T* begin = buffer.get();

            image_type type = DdlInfo::get_instance()->
                    get_image_info(type_name(obj_)).type_var_;

            return boost::apply_visitor(StorageAccess_get_image<T, Rank>(this, i_, off_, cnt_, str_, shape), type);
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
        ImageFromFile *
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