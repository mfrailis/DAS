#ifndef IMAGE_BUFFER_IPP
#define IMAGE_BUFFER_IPP

#include "image_buffer.hpp"

template<typename X, int N>
class ImageBuffer_add : public boost::static_visitor<void> {
public:

    ImageBuffer_add(das::Array<X, N> &elem, size_t rank, std::vector<ImageBufferEntry> &buffer)
    : elem_(elem), rank_(rank), buff_(buffer) {
    }

    void operator() (X* data_type) const {
        if (elem_.numElements() > 0)
            buff_.push_back(ImageBufferEntry(elem_, rank_));
    }

    template<typename Y>
    void operator() (Y* data_type) const {
        size_t size = elem_.numElements();

        if (size == 0) return;

        Y *data = new Y[size];
        size_t i = 0;
        for (typename das::Array<X, N>::iterator it = elem_.begin(); it != elem_.end(); ++it) {
            data[i] = *it;
            ++i;
        }
        buff_.push_back(ImageBufferEntry(
                das::Array<Y, N>(data, elem_.shape(), das::deleteDataWhenDone),
                rank_));
    }

private:
    das::Array<X, N> &elem_;
    std::vector<ImageBufferEntry> &buff_;
    size_t rank_;
};

template<typename T, int N>
void ImageBuffer::append(das::Array<T, N> &array, size_t rank) {
    if (N != rank && N != (rank - 1))
        throw das::incompatible_array_shape();


    if (size0_ == 0 && size1_ == 1) {
        /*
         * the buffer is empty and the shape of the image is uninizialized.
         * We deduce the shape from the shape from the array passed as argument
         */


        if (N == rank) {// we are adding more then one tiles
            /* note. we need to initialize all dimensions starting from the higher
             * so we don't need break statements
             */
            switch (N) {
                case 11: size10_ = array.extent(10);
                case 10: size9_ = array.extent(9);
                case 9: size8_ = array.extent(8);
                case 8: size7_ = array.extent(7);
                case 7: size6_ = array.extent(6);
                case 6: size5_ = array.extent(5);
                case 5: size4_ = array.extent(4);
                case 4: size3_ = array.extent(3);
                case 3: size2_ = array.extent(2);
                case 2: size1_ = array.extent(1);
            }
        } else {// we are adding one tile
            switch (N) {
                case 10: size9_ = array.extent(10);
                case 9: size8_ = array.extent(9);
                case 8: size7_ = array.extent(8);
                case 7: size6_ = array.extent(7);
                case 6: size5_ = array.extent(6);
                case 5: size4_ = array.extent(5);
                case 4: size3_ = array.extent(4);
                case 3: size2_ = array.extent(3);
                case 2: size1_ = array.extent(2);
                case 1: size1_ = array.extent(1);
            }
        }
    } else {
        if (N == rank) {
            /* note. we need to check all dimensions starting from the higher
             * so we don't need break statements
             */
            switch (N) {
                case 11: if (size10_ != array.extent(10)) throw das::incompatible_array_shape();
                case 10: if (size9_ != array.extent(9)) throw das::incompatible_array_shape();
                case 9: if (size8_ != array.extent(8)) throw das::incompatible_array_shape();
                case 8: if (size7_ != array.extent(7)) throw das::incompatible_array_shape();
                case 7: if (size6_ != array.extent(6)) throw das::incompatible_array_shape();
                case 6: if (size5_ != array.extent(5)) throw das::incompatible_array_shape();
                case 5: if (size4_ != array.extent(4)) throw das::incompatible_array_shape();
                case 4: if (size3_ != array.extent(3)) throw das::incompatible_array_shape();
                case 3: if (size2_ != array.extent(2)) throw das::incompatible_array_shape();
                case 2: if (size1_ != array.extent(1)) throw das::incompatible_array_shape();
            }
        } else {// we are adding one tile
            switch (N) {
                case 10: if (size9_ != array.extent(10)) throw das::incompatible_array_shape();
                case 9: if (size8_ != array.extent(9)) throw das::incompatible_array_shape();
                case 8: if (size7_ != array.extent(8)) throw das::incompatible_array_shape();
                case 7: if (size6_ != array.extent(7)) throw das::incompatible_array_shape();
                case 6: if (size5_ != array.extent(6)) throw das::incompatible_array_shape();
                case 5: if (size4_ != array.extent(5)) throw das::incompatible_array_shape();
                case 4: if (size3_ != array.extent(4)) throw das::incompatible_array_shape();
                case 3: if (size2_ != array.extent(3)) throw das::incompatible_array_shape();
                case 2: if (size1_ != array.extent(2)) throw das::incompatible_array_shape();
                case 1: if (size1_ != array.extent(1)) throw das::incompatible_array_shape();
            }
        }
    }


    boost::apply_visitor(ImageBuffer_add<T, N>(array, rank, buffer_), type_);

    if (N == rank)
        size0_ += array.extent(0);
    else
        size0_++;
}
/*
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
 */

template<class OutputIterator>
OutputIterator
ImageBuffer::copy(OutputIterator& begin,
        OutputIterator& end,
        const das::TinyVector<size_t, 11>& offset,
        const das::TinyVector<size_t, 11>& count,
        const das::TinyVector<size_t, 11>& stride){
    if(size0_ < stride[0] * count[0] + offset[0])
        throw das::bad_array_slice();
    if(size1_ < stride[1] * count[1] + offset[1])
        throw das::bad_array_slice();
    if(size2_ < stride[2] * count[2] + offset[2])
        throw das::bad_array_slice();
    if(size3_ < stride[3] * count[3] + offset[3])
        throw das::bad_array_slice();
    if(size4_ < stride[4] * count[4] + offset[4])
        throw das::bad_array_slice();
    if(size5_ < stride[5] * count[5] + offset[5])
        throw das::bad_array_slice();
    if(size6_ < stride[6] * count[6] + offset[6])
        throw das::bad_array_slice();
    if(size7_ < stride[7] * count[7] + offset[7])
        throw das::bad_array_slice();
    if(size8_ < stride[8] * count[8] + offset[8])
        throw das::bad_array_slice();
    if(size9_ < stride[9] * count[9] + offset[9])
        throw das::bad_array_slice();
    if(size10_ < stride[10] * count[10] + offset[10])
        throw das::bad_array_slice();
    
    //TODO
    return begin;
    
}
#endif