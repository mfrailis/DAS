#include "internal/column_buffer.hpp"
#include <iterator>

template<typename T>
inline
bool operator==(const BufferIterator<T> &lhs, const BufferIterator<T> &rhs) {
    return lhs.equal(rhs);
}

template<typename T>
inline
bool operator!=(const BufferIterator<T> &lhs, const BufferIterator<T> &rhs) {
    return !(lhs.equal(rhs));
}

template<typename T>
inline
BufferIterator<T>&
BufferIterator<T>::operator++() {
    if (vb_ != ve_) {
        if (ab_ != ae_) {
            ++ab_;
        }
        //while allows to skip possible empty containers
        while (ab_ == ae_) {
            ++vb_;
            if (vb_ == ve_) {
                ab_ = das_iterator();
                ae_ = das_iterator();
                break;
            }
            ab_ = vb_->begin();
            ae_ = vb_->end();
        }
    } else {
        ab_ = das_iterator();
        ae_ = das_iterator();
    }
    return *this;
}

template<typename T>
inline
void
BufferIterator<T>::seek_b() {
    vb_ = vec_.begin();
    ve_ = vec_.end();
    if (vb_ != ve_) {
        ab_ = vb_->begin();
        ae_ = vb_->end();

        //while allows to skip possible empty containers
        while (ab_ == ae_) {
            ++vb_;
            if (vb_ == ve_) {
                ab_ = das_iterator();
                ae_ = das_iterator();
                break;
            }
            ab_ = vb_->begin();
            ae_ = vb_->end();
        }
    } else {
        ab_ = das_iterator();
        ae_ = das_iterator();
    }
}

template<typename T>
inline
void
BufferIterator<T>::seek_e() {
    vb_ = ve_ = vec_.end();

    if (vb_ != vec_.begin()) {
        --vb_;
        ab_ = ae_ = vb_->end();
        ++vb_;
    } else {
        ab_ = das_iterator();
        ae_ = das_iterator();
    }
}

template<typename X>
class ColumnBuffer_add : public boost::static_visitor<void> {
public:

    ColumnBuffer_add(das::Array<X> &elem) {
        elem_ = &elem;
    }

    void operator() (std::vector< das::Array<X> > &vec) {
        if (elem_->size() > 0)
            vec.push_back(*elem_);
    }

    template<typename Y>
    void operator() (std::vector< das::Array<Y> > &vec) {
        size_t size = elem_->size();

        if (size == 0) return;

        Y *data = new Y[size];
        size_t i = 0;
        for (typename das::Array<X>::iterator it = elem_->begin(); it != elem_->end(); ++it) {
            data[i] = *it;
            ++i;
        }
        vec.push_back(das::Array<Y>(data, size, das::deleteDataWhenDone));
    }

    void operator() (std::vector< das::Array<std::string> > &vec) {
        std::cout << "string conversion not supported yet" << std::endl;

    }

    das::Array<X> *elem_;

};

template<typename T>
void
ColumnBuffer::append(das::Array<T> &array) {
    if (!is_init_) {
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();
    }
    ColumnBuffer_add<T> v(array);
    boost::apply_visitor(v, buffer_);
}

template<class OutputIterator>
class ColumnBuffer_copy : public boost::static_visitor<OutputIterator> {
public:

    ColumnBuffer_copy(OutputIterator &begin, OutputIterator &end, size_t offset) :
    b_(begin), e_(end), o_(offset) {
    }

    template<typename T>
    OutputIterator operator() (std::vector< das::Array<T> > &vec) {
        typename std::vector< das::Array<T> >::iterator v_it = vec.begin();
        if (v_it == vec.end()) return b_;
        size_t first_offset = 0;

        while (o_ > 0) {
            size_t size = v_it->size();
            if (o_ >= size) {
                o_ -= size;
                ++v_it;
            } else {
                first_offset = o_;
                o_ = 0;
            }
            if (v_it == vec.end()) return b_;
        }
        typename das::Array<T>::iterator a_it = v_it->begin();
        while (first_offset-- > 0) ++a_it;

        while (a_it == v_it->end()) {
            ++v_it;
            if (v_it == vec.end())
                return b_;
            a_it = v_it->begin();
        }


        while (b_ != e_) {
            *b_ = *a_it;
            ++b_;
            ++a_it;
            while (a_it == v_it->end()) {
                ++v_it;
                if (v_it == vec.end())
                    return b_;
                a_it = v_it->begin();
            }
        }

        return b_;
    }

    OutputIterator operator() (std::vector< das::Array<std::string> > &vec) {
        std::cout << "string copy not implemented yet" << std::endl;
        return b_;
    }

private:
    OutputIterator b_;
    OutputIterator e_;
    size_t o_;
};

template<class OutputIterator>
OutputIterator
ColumnBuffer::copy(OutputIterator &begin, OutputIterator &end, size_t offset) {
    if (!is_init_) {
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();
    }
    ColumnBuffer_copy<OutputIterator> bcp(begin, end, offset);
    return boost::apply_visitor(bcp, buffer_);
}

template<typename T>
class ColumnBuffer_iterator : public boost::static_visitor<BufferIterator<T> > {
public:

    BufferIterator<T>
    operator() (std::vector< das::Array<T> > &vec) const {
        return BufferIterator<T>(vec);
    }

    template<typename U>
    BufferIterator<T>
    operator() (std::vector< das::Array<U> > &vec) const {
        std::cout << "type mismatch" << std::endl;
        throw std::exception();
    }
};

template<typename T>
inline
BufferIterator<T>
ColumnBuffer::get_iterator(bool is_end) {
    if (!is_init_) {
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();
    }
    BufferIterator<T> it = boost::apply_visitor(ColumnBuffer_iterator<T>(), buffer_);
    if (is_end)
        it.seek_e();
    return it;
}
