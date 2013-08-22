#ifndef IMAGE_BUFFER_HPP
#define	IMAGE_BUFFER_HPP

#include <vector>
#include <exception>
#include "array.hpp"
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include "../ddl/info.hpp"

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
    const T*
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

    ImageBuffer(const std::string &type,
            unsigned int &size1,
            unsigned int &size2,
            unsigned int &size3,
            unsigned int &size4,
            unsigned int &size5,
            unsigned int &size6,
            unsigned int &size7,
            unsigned int &size8,
            unsigned int &size9,
            unsigned int &size10)
    : is_init_(false), size0_(0), size1_(size1), size2_(size2), size3_(size3),
    size4_(size4), size5_(size5), size6_(size6), size7_(size7), size8_(size8),
    size9_(size9), size10_(size10) {
        init(type);
    }

    ImageBuffer(unsigned int &size1,
            unsigned int &size2,
            unsigned int &size3,
            unsigned int &size4,
            unsigned int &size5,
            unsigned int &size6,
            unsigned int &size7,
            unsigned int &size8,
            unsigned int &size9,
            unsigned int &size10)
    : is_init_(false), size0_(0), size1_(size1), size2_(size2), size3_(size3),
    size4_(size4), size5_(size5), size6_(size6), size7_(size7), size8_(size8),
    size9_(size9), size10_(size10) {
    }

    void init(const std::string &type);

    bool is_init() const {
        return is_init_;
    }

    bool empty();

    int
    extent(size_t rank) {
        switch (rank) {
            case 0: return size0_;
            case 1: return size1_;
            case 2: return size2_;
            case 3: return size3_;
            case 4: return size4_;
            case 5: return size5_;
            case 6: return size6_;
            case 7: return size7_;
            case 8: return size8_;
            case 9: return size9_;
            case 10: return size10_;
            default:
                return 1;
        }
    }

    template<typename T, int N>
    void append(das::Array<T, N> &array, size_t rank);

    const std::vector<ImageBufferEntry>&
    buckets() const {
        return buffer_;
    }

    void
    clear() {
        buffer_.clear();
        size0_ = 0;
    }

    unsigned int
    num_elements() const {
        return size0_ * size1_ * size2_ * size3_ * size4_ *
                size5_ * size6_ * size7_ * size8_ * size9_ * size10_;
    }


    template<class OutputIterator>
    size_t
    copy(OutputIterator &begin,
            const das::TinyVector<int, 11> &offset,
            const das::TinyVector<int, 11> &count,
            const das::TinyVector<int, 11> &stride);

private:
    image_type type_;
    std::vector<ImageBufferEntry> buffer_;
    bool is_init_;

    unsigned int size0_;
    unsigned int &size1_;
    unsigned int &size2_;
    unsigned int &size3_;
    unsigned int &size4_;
    unsigned int &size5_;
    unsigned int &size6_;
    unsigned int &size7_;
    unsigned int &size8_;
    unsigned int &size9_;
    unsigned int &size10_;
};


#endif	/* IMAGE_BUFFER_HPP */
