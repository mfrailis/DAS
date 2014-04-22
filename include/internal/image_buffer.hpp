#ifndef IMAGE_BUFFER_HPP
#define	IMAGE_BUFFER_HPP

#include <deque>
#include <exception>
#include "array.hpp"
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include "../ddl/info.hpp"

class Image;

class ImageBufferEntry {
public:
    typedef boost::variant<
    char*,
    short*,
    int*,
    float*,
    double*
    > data_ptr;

    template <typename T, int N>
    ImageBufferEntry(das::Array<T, N> &array, size_t rank)
    : array_(array), shape_(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), data_(array.data()) {
        size_t i = 0;

        if (array.rank() < rank)
            ++i;

        for (size_t j = 0; j < array.rank(); ++j)
            shape_[i++] = array.extent(j);
    }

    template <typename T, int N>
    das::Array<T, N> *
    cast() {
        return boost::any_cast<das::Array<T, N> >(&array_);
    }

    const das::TinyVector<int, 11>&
    shape() const {
        return shape_;
    }

    template<typename T>
    T*
    data() const {
        return boost::get<T*>(data_);
    }

    size_t
    num_elements() const {
        size_t elems = 1;
        for (int i = 0; i < 11; ++i) {
            elems *= shape_[i];
        }
        return elems;
    }
private:
    ImageBufferEntry();
    das::TinyVector<int, 11> shape_;
    boost::any array_;
    data_ptr data_;
};

class ImageBuffer {
public:

    ImageBuffer(const std::string &type, Image *iff)
    : is_init_(false), iff_(iff), size0_(0){
        init(type);
    }

    ImageBuffer(Image *iff)
    : is_init_(false), iff_(iff), size0_(0){
    }

    void init(const std::string& type);

    bool is_init() const {
        return is_init_;
    }
    
    bool empty();

    template<typename T, int N>
    void append(das::Array<T, N> &array);

    const std::deque<ImageBufferEntry>&
    buckets() const {
        return buffer_;
    }

    void
    clear() {
        buffer_.clear();
        size0_ = 0;
    }

    unsigned int
    num_elements() const;

    const unsigned int&
    tiles() const {return size0_;}
    
    template<class OutputIterator>
    size_t
    copy(OutputIterator &begin,
            const das::TinyVector<int, 11> &offset,
            const das::TinyVector<int, 11> &count,
            const das::TinyVector<int, 11> &stride);

private:
    ImageBuffer();
    image_type type_;
    std::deque<ImageBufferEntry> buffer_;
    bool is_init_;
    Image *iff_;

    unsigned int size0_;
};


#endif	/* IMAGE_BUFFER_HPP */
