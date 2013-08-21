#ifndef COLUMN_BUFFER_HPP
#define	COLUMN_BUFFER_HPP

#include <vector>
#include <exception>
#include "array.hpp"
#include <boost/variant.hpp>

template<typename T>
class BufferIterator {
public:

    BufferIterator(std::vector< das::Array<T> > &vec) : vec_(vec) {
        seek_b();
    }

    BufferIterator(const BufferIterator<T> &it)
    : vec_(it.vec_),
    ab_(it.ab_),
    ae_(it.ae_),
    vb_(it.vb_),
    ve_(it.ve_) {
    }

    T& operator*() {
        return *ab_;
    }

    T*
    operator-> () {
        return ab_.operator->();
    }

    BufferIterator<T>&
    operator++();

    BufferIterator<T>
    operator++(int) {
        BufferIterator<T> m(*this);
        operator++();
        return m;
    }

    bool equal(const BufferIterator<T> &rhs) const {
        return ab_ == rhs.ab_;
    }

    void seek_b();
    void seek_e();
private:
    typedef typename das::Array<T>::iterator das_iterator;
    typedef typename std::vector< das::Array<T> >::iterator std_iterator;


    das_iterator ab_;
    das_iterator ae_;
    std_iterator vb_;
    std_iterator ve_;
    std::vector< das::Array<T> > &vec_;

};

template<typename T>
bool operator==(const BufferIterator<T> &lhs, const BufferIterator<T> &rhs);

template<typename T>
bool operator!=(const BufferIterator<T> &lhs, const BufferIterator<T> &rhs);

class ColumnBuffer {
public:
    typedef boost::variant<
    std::vector< das::Array<char> >,
    std::vector< das::Array<short> >,
    std::vector< das::Array<int> >,
    std::vector< das::Array<long long> >,
    std::vector< das::Array<float> >,
    std::vector< das::Array<double> >,
    std::vector< das::Array<bool> >,
    std::vector< das::Array<unsigned char> >,
    std::vector< das::Array<unsigned short> >,
    std::vector< das::Array<unsigned int> >,
    std::vector< das::Array<std::string> >
    > buffer_type;

    ColumnBuffer(const std::string &type);

    ColumnBuffer();

    void init(const std::string &type);

    bool is_init() {
        return is_init_;
    }

    bool empty();

    size_t size() const;

    template<typename T>
    void append(das::Array<T> &array);

    template<class OutputIterator>
    OutputIterator
    copy(OutputIterator &begin, OutputIterator &end, size_t offset);

    template<typename T>
    BufferIterator<T>
    begin() {
        return get_iterator<T>(false);
    }

    template<typename T>
    BufferIterator<T>
    end() {
        return get_iterator<T>(true);
    }

    template<typename T>
    std::vector<std::pair<T*, size_t> >
    buckets();

    void
    clear();

private:
    template<typename T>
    BufferIterator<T> get_iterator(bool is_end);
    buffer_type buffer_;
    bool is_init_;
};


#endif	/* COLUMN_BUFFER_HPP */

