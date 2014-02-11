#ifndef COLUMN_BUFFER_HPP
#define	COLUMN_BUFFER_HPP

#include <deque>
#include <exception>
#include "array.hpp"
#include "utility.hpp"
#include <boost/variant.hpp>


class ColumnBuffer {
public:
    typedef boost::variant<  
    std::deque< das::ArrayStore<char> >,
    std::deque< das::ArrayStore<short> >,
    std::deque< das::ArrayStore<int> >,
    std::deque< das::ArrayStore<long long> >,
    std::deque< das::ArrayStore<float> >,
    std::deque< das::ArrayStore<double> >,
    std::deque< das::ArrayStore<bool> >,
    std::deque< das::ArrayStore<unsigned char> >,
    std::deque< das::ArrayStore<unsigned short> >,
    std::deque< das::ArrayStore<unsigned int> >,
    std::deque< das::ArrayStore<std::string> >
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

    template<class T>
    T*
    copy(T* begin, T* end, size_t offset);

    template<typename T>
    std::deque<std::pair<T*, size_t> >
    buckets();

    void
    clear();

private:
    buffer_type buffer_;
    bool is_init_type_;
    bool is_init_shape_;
    das::TinyVector<int,11> shape_;
    size_t rank_;
};


#endif	/* COLUMN_BUFFER_HPP */

