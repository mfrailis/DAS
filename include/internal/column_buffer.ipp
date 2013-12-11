#ifndef COLUMN_BUFFER_IPP
#define COLUMN_BUFFER_IPP

#include "column_buffer.hpp"
#include <iterator>

template<int Rank>
bool
ColumnBuffer::check_shape(das::TinyVector<int, Rank> &s) {
    if (!is_init()) {
        throw das::bad_object();
    }

    if (rank_ != Rank)
        return false;

    if (Rank == 1)
        return true;

    for (size_t i = 1; i < Rank; ++i)
        if (shape_(i) != s(i))
            return false;

    if (shape_(0) != -1 && shape_(0) != s(0))
        return false;

    return true;

}

template<typename X>
class ColumnBuffer_add : public boost::static_visitor<void> {
public:

    ColumnBuffer_add(das::Array<X> &elem) : elem_(elem) {
    }

    void operator() (std::vector< das::ArrayStore<X> > &vec) const {
        if (elem_.size() > 0)
            vec.push_back(das::ArrayStore<X>(elem_));
    }

    template<typename Y>
    void operator() (std::vector< das::ArrayStore<Y> > &vec) const {
        size_t size = elem_.size();

        if (size == 0) return;

        Y *data = new Y[size];
        size_t i = 0;
        for (typename das::Array<X>::iterator it = elem_.begin(); it != elem_.end(); ++it) {
            data[i] = *it;
            ++i;
        }
        vec.push_back(das::ArrayStore<Y>(data, size));
    }

    void operator() (std::vector< das::ArrayStore<std::string> > &vec) const {
        throw das::bad_type();
    }

private:
    das::Array<X> &elem_;

};

template<>
class ColumnBuffer_add<std::string> : public boost::static_visitor<void> {
public:

    ColumnBuffer_add(das::Array<std::string> &elem) : elem_(elem) {
    }

    void operator() (std::vector< das::ArrayStore<std::string> > &vec) const {
        if (elem_.size() > 0)
            vec.push_back(das::ArrayStore<std::string>(elem_));
    }

    template<typename Y>
    void operator() (std::vector< das::ArrayStore<Y> > &vec) const {
        throw das::bad_type();
    }
private:
    das::Array<std::string> &elem_;
};

// column arrays

template<typename X, int Rank>
class ColumnBuffer_array_add : public boost::static_visitor<void> {
public:
    typedef das::ColumnArray<X, Rank> Elem_t;

    ColumnBuffer_array_add(das::ColumnArray<X, Rank> &elem) : elem_(elem) {
    }

    void operator() (std::vector< das::ArrayStore<X> > &vec) const {
        for (typename Elem_t::iterator it = elem_.begin(); it != elem_.end(); ++it)
            if (it->size() > 0)
                vec.push_back(das::ArrayStore<X>(*it));
    }

    template<typename Y>
    void operator() (std::vector< das::ArrayStore<Y> > &vec) const {
        size_t elements = elem_.size();

        if (elements == 0) return;
        for (size_t j = 0; j < elements; ++j) {
            size_t size = elem_(j).size();
            Y *data = new Y[size];
            size_t i = 0;
            for (typename das::Array<X, Rank>::iterator it = elem_(j).begin(); it != elem_(j).end(); ++it) {
                data[i] = *it;
                ++i;
            }
            das::TinyVector<int, Rank> shape(elem_(j).shape());
            vec.push_back(das::ArrayStore<Y>(data, shape));
        }
    }

    void operator() (std::vector< das::ArrayStore<std::string> > &vec) const {
        throw das::bad_type();
    }

private:
    Elem_t &elem_;

};

template<int Rank>
class ColumnBuffer_array_add<std::string, Rank> : public boost::static_visitor<void> {
public:
    typedef das::ColumnArray<std::string, Rank> Elem_t;

    ColumnBuffer_array_add(das::ColumnArray<std::string, Rank> &elem) : elem_(elem) {
    }

    void operator() (std::vector< das::ArrayStore<std::string> > &vec) const {
        for (typename Elem_t::iterator it = elem_.begin(); it != elem_.end(); ++it)
            if (it->size() > 0)
                vec.push_back(das::ArrayStore<std::string>(*it));
    }

    template<typename Y>
    void operator() (std::vector< das::ArrayStore<Y> > &vec) const {
        throw das::bad_type();
    }

private:
    Elem_t &elem_;
};

template<typename T>
void
ColumnBuffer::append(das::Array<T> &array) {
    if (!is_init()) {
        throw das::bad_object();
    }
    das::TinyVector<int, 1> s = array.extent();
    if (rank_ != 1)
        throw das::bad_array_shape();
    if (!check_shape(s))
        throw das::bad_array_shape();

    boost::apply_visitor(ColumnBuffer_add<T>(array), buffer_);
}

template<typename T, int Rank>
void ColumnBuffer::append(das::ColumnArray<T, Rank> &array) {
    if (!is_init()) {
        throw das::bad_object();
    }
    if (rank_ == 1 && shape_(0) == 1)
        throw das::bad_array_shape();
    for (typename das::ColumnArray<T, Rank>::iterator it = array.begin();
            it != array.end();
            ++it) {
        das::TinyVector<int, Rank> s = it->extent();
        if (!check_shape(s))
            throw das::bad_array_shape();
    }

    boost::apply_visitor(ColumnBuffer_array_add<T, Rank>(array), buffer_);
}

template<class U>
class ColumnBuffer_copy : public boost::static_visitor<U*> {
public:

    ColumnBuffer_copy(U* begin, U* end, size_t offset) :
    b_(begin), e_(end), o_(offset) {
    }

    template<typename T>
    U* operator() (std::vector< das::ArrayStore<T> > &vec) {
        typename std::vector< das::ArrayStore<T> >::iterator v_it = vec.begin();
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
        typename das::ArrayStore<T>::iterator a_it = v_it->begin();
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

    U* operator() (std::vector< das::ArrayStore<std::string> > &vec) {
        return b_;
    }

private:
    U* b_;
    U* e_;
    size_t o_;
};

template<>
class ColumnBuffer_copy<std::string> : public boost::static_visitor<std::string*> {
public:

    ColumnBuffer_copy(std::string* begin, std::string* end, size_t offset) :
    b_(begin), e_(end), o_(offset) {
    }

    std::string* operator() (std::vector< das::ArrayStore<std::string> > &vec) {
        std::vector< das::ArrayStore<std::string> >::iterator v_it = vec.begin();
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
        das::ArrayStore<std::string>::iterator a_it = v_it->begin();
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

    template<typename T>
    std::string* operator() (std::vector< das::ArrayStore<T> > &vec) {
        return b_;
    }

private:
    std::string* b_;
    std::string* e_;
    size_t o_;
};

// column_array

template<typename X, int Rank>
class ColumnBuffer_copy< das::Array<X, Rank> >
: public boost::static_visitor< das::Array<X, Rank>* > {
public:

    typedef das::Array<X, Rank>* OutputIterator;

    ColumnBuffer_copy(OutputIterator begin, OutputIterator end, size_t offset) :
    b_(begin), e_(end), o_(offset) {
    }

    template<typename U>
    OutputIterator
    operator() (std::vector< das::ArrayStore<U> > &vec) {
        typename std::vector< das::ArrayStore<U> >::iterator v_it = vec.begin();
        if (v_it == vec.end()) return b_;
        size_t first_offset = 0;

        while (o_ > 0) {
            ++v_it;
            --o_;
            if (v_it == vec.end()) return b_;
        }

        while (b_ != e_) {
            // do not reference the array in the buffer. Make a copy

            b_->reference(v_it->template copy_array<X, Rank>());
            ++b_;
            ++v_it;
            if (v_it == vec.end())

                return b_;
        }
        return b_;
    }

    OutputIterator operator() (std::vector< das::ArrayStore<std::string> > &vec) {
        throw das::not_implemented();
    }

private:
    OutputIterator b_;
    OutputIterator e_;
    size_t o_;
};

template<int Rank>
class ColumnBuffer_copy< das::Array<std::string, Rank> >
: public boost::static_visitor< das::Array<std::string, Rank>* > {
public:

    typedef das::Array<std::string, Rank>* OutputIterator;

    ColumnBuffer_copy(OutputIterator begin, OutputIterator end, size_t offset) :
    b_(begin), e_(end), o_(offset) {
    }

    OutputIterator 
    operator() (std::vector< das::ArrayStore<std::string> > &vec) {
        std::vector< das::ArrayStore<std::string> >::iterator v_it = vec.begin();
        if (v_it == vec.end()) return b_;
        size_t first_offset = 0;

        while (o_ > 0) {
            ++v_it;
            --o_;
            if (v_it == vec.end()) return b_;
        }

        while (b_ != e_) {
            // do not reference the array in the buffer. Make a copy

            b_->reference(v_it->template copy_array<std::string, Rank>());
            ++b_;
            ++v_it;
            if (v_it == vec.end())

                return b_;
        }
        return b_;
    }

    template<typename U>
    OutputIterator
    operator() (std::vector< das::ArrayStore<U> > &vec) {
        throw das::bad_type();
    }


private:
    OutputIterator b_;
    OutputIterator e_;
    size_t o_;
};

template<class T >
T*
ColumnBuffer::copy(T* begin, T* end, size_t offset) {
    if (!is_init()) {
        throw das::bad_object();
    }
    ColumnBuffer_copy<T> bcp(begin, end, offset);

    return boost::apply_visitor(bcp, buffer_);
}

template<typename T>
class ColumnBuffer_buckets : public boost::static_visitor<std::vector<std::pair<T*, size_t> > > {
public:

    template<typename U>
    std::vector<std::pair<T*, size_t> >
    operator() (std::vector< das::ArrayStore<U> > &vec) const {
        throw das::bad_type();
    }

    std::vector<std::pair<T*, size_t> >
    operator() (std::vector< das::ArrayStore<T> > &vec) const {
        std::vector<std::pair<T*, size_t> > bks;
        for (typename std::vector< das::ArrayStore<T> >::iterator it = vec.begin();
                it != vec.end(); ++it)
            bks.push_back(it->pair());

        return bks;
    }
};

template<typename T >
std::vector<std::pair<T*, size_t> >
ColumnBuffer::buckets() {

    return boost::apply_visitor(ColumnBuffer_buckets<T>(), buffer_);
}

class ColumnBuffer_clear : public boost::static_visitor<void> {
public:

    template<typename T>
    void
    operator() (std::vector< das::ArrayStore<T> > &vec) const {

        vec.clear();
    }

};

inline
void
ColumnBuffer::clear() {
    boost::apply_visitor(ColumnBuffer_clear(), buffer_);
}

#endif