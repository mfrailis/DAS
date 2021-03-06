#ifndef STORAGE_ENGINE_IPP
#define	STORAGE_ENGINE_IPP
#include "../storage_engine.hpp"
#include "../ddl/info.hpp"
#include "../das_object.hpp"
#include "column_buffer.ipp"
#include "image_buffer.ipp"
#include "log.hpp"
#include "../das_object.hpp"
#include <boost/variant.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>





namespace das {



    template<typename T>
    class StorageAccess_get_column : public boost::static_visitor<Array<T> > {
    public:

        StorageAccess_get_column(StorageAccess *acc, const std::string &col_name, Column *c, size_t start, size_t length)
        : sa_(acc), c_(c), s_(start), l_(length), cn_(col_name) {
        }

        template<typename U >
        Array<T> operator()(U& native_type) const {
            using boost::interprocess::unique_ptr;

            unique_ptr<T, ArrayDeleter<T> > buffer(new T[l_]);
            T* b = &buffer.get()[0];
            T* e = &buffer.get()[l_]; //first index not valid
            size_t count = 0;
            if (c_->store_size() > s_) {
                size_t size = c_->store_size() - s_;
                unique_ptr<U, ArrayDeleter<U> > buff_temp(new U[size]);
                StorageAccess::column_buffer_ptr tb = buff_temp.get();

                count = sa_->read_column(cn_, c_, tb, s_, size);

                if (count < size)
                    throw io_exception();

                for (size_t i = 0; i < count; ++i)
                    buffer.get()[i] = buff_temp.get()[i];
                s_ = 0;
            }else{
                s_ -= c_->store_size();
            }

            b += count;
            size_t missing = 0;
            if (count < l_) {
                T* cached = c_->buffer().copy(b, e, s_);
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
            if (c_->store_size() > s_) {
                /*
                 * calculate the amount of date to read from file:
                 *  min(file_size - offset, length) 
                 */
                size_t to_read = c_->store_size() - s_;
                to_read = to_read > l_ ? l_ : to_read;

                StorageAccess::column_buffer_ptr tb = buffer.get();

                count = sa_->read_column(cn_, c_, tb, s_, to_read);

                if (count < to_read)
                    throw io_exception();
                s_ = 0;
            }else{
                s_ -= c_->store_size();
            }
            
            b += count;
            size_t missing = 0;
            if (count < l_) {
                T* cached = c_->buffer().copy(b, e, s_);
                missing = e - cached;
            }
            if (missing > 0)
                throw io_exception();

            return Array<T>(buffer.release(), l_, das::deleteDataWhenDone);
        }

        Array<T> operator()(std::string & native_type) const {
            throw das::bad_type();
        }

    private:
        StorageAccess *sa_;
        Column *c_;
        mutable size_t s_;
        size_t l_;
        const std::string &cn_;
    };

    template<>
    class StorageAccess_get_column<std::string> : public boost::static_visitor<Array<std::string> > {
    public:

        StorageAccess_get_column(StorageAccess *acc, const std::string &col_name, Column *c, size_t start, size_t length)
        : sa_(acc), c_(c), s_(start), l_(length), cn_(col_name) {
        }

        template<typename T >
        Array<std::string> operator()(T & native_type) const {
            throw das::bad_type();
        }

        Array<std::string> operator()(std::string &native_type) const {
            using boost::interprocess::unique_ptr;

            unique_ptr<std::string, ArrayDeleter<std::string> > buffer(new std::string[l_]);
            std::string* b = &buffer.get()[0];
            std::string* e = &buffer.get()[l_]; //first not valid index
            size_t count = 0;

            // check if we need to read some (or all) data from file
            if (c_->store_size() > s_) {
                /*
                 * calculate the amount of data to read from file:
                 *  min(file_size - offset, length) 
                 */
                size_t to_read = c_->store_size() - s_;
                to_read = to_read > l_ ? l_ : to_read;
                StorageAccess::column_buffer_ptr tb = buffer.get();

                count = sa_->read_column(cn_, c_, tb, s_, to_read);

                if (count < to_read)
                    throw io_exception();
                s_ = 0;
            }else{
                s_ -= c_->store_size();
            }
            
            b += count;
            size_t missing = 0;
            if (count < l_) {
                std::string* cached = c_->buffer().copy(b, e, s_);
                missing = e - cached;
            }
            if (missing > 0)
                throw io_exception();

            return Array<std::string>(buffer.release(), l_, das::deleteDataWhenDone);
        }

    private:
        StorageAccess *sa_;
        Column *c_;
        mutable size_t s_;
        size_t l_;
        const std::string &cn_;
    };

  // column_array
    template<typename T,int Rank>
    class StorageAccess_get_column_array : public boost::static_visitor<ColumnArray<T,Rank> > {
        typedef Array<T,Rank> T_elem;
        public:

        
        StorageAccess_get_column_array(StorageAccess *acc, const std::string &col_name, Column *c, size_t start, size_t length)
        : sa_(acc), c_(c), s_(start), l_(length), cn_(col_name) {
        }

        TinyVector<int, Rank>
        get_shape() const{
            das::TinyVector<int, Rank> s;
            std::vector<int> shape = ColumnInfo::array_extent(c_->get_array_size());
            
            size_t rank = shape.size();
            if(Rank != rank)
                throw das::bad_array_size();
            
            for(size_t i=0; i<rank; ++i)
                s(i) = shape[(rank-i)-1];
                        
            return s;
        }

        template<typename U >
        ColumnArray<T,Rank>
        operator()(U& native_type) const {
            using boost::interprocess::unique_ptr;

            TinyVector<int, Rank> shape = get_shape();
            
            unique_ptr<T_elem, ArrayDeleter<T_elem> > buffer(new T_elem[l_]);
            T_elem* b = &buffer.get()[0];
            T_elem* e = &buffer.get()[l_]; //first index not valid


            size_t count = 0;

            // check if we need to read some (or all) data from file
            if (c_->store_size() > s_) {
                /*
                 * calculate the amount of date to read from file:
                 *  min(file_size - offset, length) 
                 */
                size_t to_read = c_->store_size() - s_;
                to_read = to_read > l_ ? l_ : to_read;

                StorageAccess::column_array_buffer_ptr ub = ColumnArrayBuffer<U>();

                count = sa_->read_column_array(cn_, c_, ub, s_, to_read);

                if (count < to_read)
                    throw io_exception();
                
                boost::get<ColumnArrayBuffer<U> >(ub).release(buffer.get(),count,shape);

                s_ = 0;
            }else{
                s_ -= c_->store_size();
            }


            b += count;
            size_t missing = 0;
            if (count < l_) {
                T_elem* cached = c_->buffer().copy(b, e, s_);
                missing = e - cached;
            }
            if (missing > 0)
                throw io_exception();

            return ColumnArray<T,Rank>(buffer.release(), l_,deleteDataWhenDone);
        
        }

        ColumnArray<T,Rank> operator()(std::string & native_type) const {
            throw das::bad_type();
        }

    private:
        StorageAccess *sa_;
        Column *c_;
        mutable size_t s_;
        size_t l_;
        const std::string &cn_;
    };

    template<int Rank>
    class StorageAccess_get_column_array<std::string,Rank> : public boost::static_visitor<ColumnArray<std::string, Rank> > {
        typedef Array<std::string,Rank> T_elem;
        public:

        
        StorageAccess_get_column_array(StorageAccess *acc, const std::string &col_name, Column *c, size_t start, size_t length)
        : sa_(acc), c_(c), s_(start), l_(length), cn_(col_name) {
        }

        TinyVector<int, Rank>
        get_shape() const{
            das::TinyVector<int, Rank> s;
            std::vector<int> shape = ColumnInfo::array_extent(c_->get_array_size());
            
            size_t rank = shape.size();
            if(Rank != rank)
                throw das::bad_array_size();
            
            for(size_t i=0; i<rank; ++i)
                s(i) = shape[(rank-i)-1];
                        
            return s;
        }
        
        template<typename T>
        ColumnArray<std::string,Rank> operator()(T& native_type) const {
            throw das::bad_type();
        }

        ColumnArray<std::string,Rank> operator()(std::string & native_type) const {
                      using boost::interprocess::unique_ptr;

            TinyVector<int, Rank> shape = get_shape();
            
            unique_ptr<T_elem, ArrayDeleter<T_elem> > buffer(new T_elem[l_]);
            T_elem* b = &buffer.get()[0];
            T_elem* e = &buffer.get()[l_]; //first index not valid


            size_t count = 0;

            // check if we need to read some (or all) data from file
            if (c_->store_size() > s_) {
                /*
                 * calculate the amount of date to read from file:
                 *  min(file_size - offset, length) 
                 */
                size_t to_read = c_->store_size() - s_;
                to_read = to_read > l_ ? l_ : to_read;

                StorageAccess::column_array_buffer_ptr tb = ColumnArrayBuffer<std::string>();

                count = sa_->read_column_array(cn_, c_, tb, s_, to_read);

                if (count < to_read)
                    throw io_exception();
                

                boost::get<ColumnArrayBuffer<std::string> >(tb).release(buffer.get(),count,shape);

                s_ = 0;
            }else{
                s_ -= c_->store_size();
            }


            b += count;
            size_t missing = 0;
            if (count < l_) {
                T_elem* cached = c_->buffer().copy(b, e, s_);
                missing = e - cached;
            }
            if (missing > 0)
                throw io_exception();

            return ColumnArray<std::string,Rank>(buffer.release(), l_,deleteDataWhenDone);
        }

    private:
        StorageAccess *sa_;
        Column *c_;
        mutable size_t s_;
        size_t l_;
        const std::string &cn_;
    };


    
    
    template<typename T>
    Array<T>
    StorageAccess::get_column(const string& col_name, size_t start, size_t length) {
        Column *c = obj_->column_ptr(col_name); //throw if bad name
        if (!c || length == 0) { //no data, throw exception
            throw empty_column();
        }
        if (c->get_array_size() != "1") throw bad_array_size();

        column_type type = DdlInfo::get_instance()->
                get_column_info(type_name(obj_), col_name).type_var_;

        return boost::apply_visitor(StorageAccess_get_column<T>(this, col_name, c, start, length), type);
    }

    template<typename T>
    void
    StorageAccess::append_column(const string &col_name, Array<T> &a) {
        Column *c = obj_->column_ptr(col_name); //throw if bad name

        if (!c) {
            const ColumnInfo& info = DdlInfo::get_instance()->get_column_info(obj_->type_name_, col_name);
            Column* cff = create_column(info.type, info.array_size);
            obj_->column_ptr(col_name, *cff);
            c = obj_->column_ptr(col_name);
            delete cff;
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
    StorageAccess::append_column_array(const string &col_name, ColumnArray<T,Rank> &a) {
        Column *c = obj_->column_ptr(col_name); //throw if bad name

        if (!c) {
            const ColumnInfo& info = DdlInfo::get_instance()->get_column_info(obj_->type_name_, col_name);
            Column* cff=create_column(info.type, info.array_size);
            obj_->column_ptr(col_name, *cff);
            c = obj_->column_ptr(col_name);
            delete cff;
        }

        /*
         * considerations as for append coulmn.
         */
        c->buffer().append(a);

        if (!buffered_only() && !info.buffered_data()) {
            flush_buffer(col_name, c);
        }
    }

    template <typename T, int Rank>
    ColumnArray<T,Rank>
    StorageAccess::get_column_array(const std::string &col_name, size_t start, size_t length) {

        Column *c = obj_->column_ptr(col_name); //throw if bad name
        if (!c || length == 0) { //no data, throw exception
            throw empty_column();
        }
        if (c->get_array_size() == "1") throw bad_array_size();

        column_type type = DdlInfo::get_instance()->
                get_column_info(type_name(obj_), col_name).type_var_;

        return boost::apply_visitor(StorageAccess_get_column_array<T,Rank>(this, col_name, c, start, length), type);
    }

    template <typename T, int Rank>
    void
    StorageAccess::append_tiles(Array<T, Rank> &t) {
        Image *i = obj_->image_ptr(); //throw if type does not provide image data

        if (!i) {
            Image * iff = create_image(DdlInfo::get_instance()->get_image_info(obj_->type_name_).type);
            obj_->image_ptr(*iff);
            i = obj_->image_ptr();
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
        Image* iff = create_image(DdlInfo::get_instance()->get_image_info(obj_->type_name_).type);
        Image* i = obj_->image_ptr();
        if(i)
            iff->reset(i); /*iff.fname(i->fname());*/
            
        obj_->image_ptr(*iff);
        i = obj_->image_ptr();


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
                Image *i,
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
            if (i_->store_tiles() > off_[0]) {
                /*
                 * calculate the amount of tiles to read from file:
                 *  min(1+(file_tiles - offset[0] -1)/stride[0], count[0]) 
                 */

                size_t tiles_to_read = 1;
                tiles_to_read += ((i_->store_tiles() - 1) - off_[0]) / str_[0];
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

                size_t el_from_file = sa_->read_image(i_, tb, off_, count, str_);


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
                offset[0] = (i_->store_tiles() - ((tiles_count - 1) * str_[0])) - off_[0];
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
            if (i_->store_tiles() > off_[0]) {
                /*
                 * calculate the amount of tiles to read from file:
                 *  min(1+(file_tiles - offset[0] -1)/stride[0], count[0]) 
                 */

                size_t tiles_to_read = 1;
                tiles_to_read += ((i_->store_tiles() - 1) - off_[0]) / str_[0];
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

                size_t el_from_file = sa_->read_image(i_, tb, off_, count, str_);


                if (el_from_file < tiles_to_read * cnt_[1] * cnt_[2] * cnt_[3] * cnt_[4] *
                        cnt_[5] * cnt_[6] * cnt_[7] * cnt_[8] * cnt_[9] * cnt_[10])
                    throw io_exception();

                tiles_count = tiles_to_read;

            }

            TinyVector<int, 11> offset(off_);
            TinyVector<int, 11> count(cnt_);


            if (tiles_count != 0) {
                offset[0] = (i_->store_tiles() - ((tiles_count - 1) * str_[0])) - off_[0];
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
        Image *i_;
        const TinyVector<int, 11> &cnt_;
        const TinyVector<int, 11> &off_;
        const TinyVector<int, 11> &str_;
        const TinyVector<int, Rank> shape_;
    };

    template <typename T, int Rank>
    Array<T, Rank>
    StorageAccess::get_image(
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
            ) {
        using boost::interprocess::unique_ptr;

        Image *i_ = obj_->image_ptr();
        if (i_ == NULL /*|| (i_->store_tiles() == 0 && i_->buffer().empty())*/)
            throw das::empty_image();

        if (Rank != i_->rank())
            throw das::bad_array_slice();

        das::Range range[] = {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10};
        TinyVector<int, 11> cnt_(0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
        TinyVector<int, 11> off_(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        TinyVector<int, 11> str_(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        TinyVector<int, Rank> shape;
        size_t elems = 1;
        for (size_t i = 0; i < Rank; ++i) {
            if (range[i].last() > i_->extent(i))
                throw das::bad_range();
            if (range[i].first() == range[i].last())
                range[i] = das::Range(range[i].first(), i_->extent(i), range[i].stride());
            off_[i] = range[i].first();
            str_[i] = range[i].stride();
            cnt_[i] = range[i].length();

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
    const StorageAccess::keyword_map&
    StorageAccess::get_keywords(DasObject *ptr) {
        return ptr->get_keywords();
    }

    inline
    void
    StorageAccess::get_columns_from_file(DasObject *ptr,
            std::map<std::string, Column*> &map) {
        return ptr->populate_column_map(map);
    }

    inline
    void
    StorageAccess::column_from_file(DasObject *ptr,
            const std::string &col_name,
            const Column &c) {
        ptr->column_ptr(col_name, c);
    }

    inline
    Column*
    StorageAccess::column_from_file(DasObject *ptr,
            const std::string &col_name) {
        return ptr->column_ptr(col_name);
    }

    inline
    Image *
    StorageAccess::image_ptr(DasObject *ptr) {
        return ptr->image_ptr();
    }

    inline
    void
    StorageAccess::image_ptr(DasObject *ptr, const Image &i) {
        ptr->image_ptr(i);
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
    das::StorageAccess*
    StorageTransaction::storage_access(DasObject *ptr) {
        return ptr->storage_access();
    }

}

#endif