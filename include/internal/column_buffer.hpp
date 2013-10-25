#ifndef COLUMN_BUFFER_HPP
#define	COLUMN_BUFFER_HPP

#include <vector>
#include <exception>
#include "array.hpp"
#include "utility.hpp"
#include <boost/variant.hpp>

/*template<typename T>
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
*/
class ColumnBuffer {
public:
    typedef boost::variant<  
    std::vector< das::ArrayStore<char> >,
    std::vector< das::ArrayStore<short> >,
    std::vector< das::ArrayStore<int> >,
    std::vector< das::ArrayStore<long long> >,
    std::vector< das::ArrayStore<float> >,
    std::vector< das::ArrayStore<double> >,
    std::vector< das::ArrayStore<bool> >,
    std::vector< das::ArrayStore<unsigned char> >,
    std::vector< das::ArrayStore<unsigned short> >,
    std::vector< das::ArrayStore<unsigned int> >,
    std::vector< das::ArrayStore<std::string> >
    > buffer_type;

    ColumnBuffer(const std::string &type, const std::string &array_size);

    ColumnBuffer();

    void init_type(const std::string &type);
    
    void init_shape(const std::string &array_size);
    
    bool is_init() const{
        return is_init_type_ && is_init_shape_;
    }
    
    template<int Rank>
    bool
    check_shape(das::TinyVector<int, Rank> &s);
    
    const size_t&
    rank() const{
        if (!is_init_shape_) {
            std::cout << "buffer type uninitialized" << std::endl;
            throw std::exception();
        } 
        return rank_;
    }
    
    const das::TinyVector<int,11>&
    extent() const{
        if (!is_init_shape_) {
            std::cout << "buffer type uninitialized" << std::endl;
            throw std::exception();
        } 
        return shape_;
    }

    bool empty();

    size_t size() const;

    template<typename T>
    void append(das::Array<T> &array);
    
    template<typename T,int Rank>
    void append(das::ColumnArray<T,Rank> &array);

    template<class OutputIterator>
    OutputIterator
    copy(OutputIterator &begin, OutputIterator &end, size_t offset);

/*    template<typename T>
    BufferIterator<T>
    begin() {
        return get_iterator<T>(false);
    }

    template<typename T>
    BufferIterator<T>
    end() {
        return get_iterator<T>(true);
    }
*/
    template<typename T>
    std::vector<std::pair<T*, size_t> >
    buckets();

    void
    clear();

private:
//    template<typename T>
//    BufferIterator<T> get_iterator(bool is_end);
    buffer_type buffer_;
    bool is_init_type_;
    bool is_init_shape_;
    das::TinyVector<int,11> shape_;
    size_t rank_;
};


#endif	/* COLUMN_BUFFER_HPP */

