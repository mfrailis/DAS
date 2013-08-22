#ifndef IMAGE_BUFFER_IPP
#define IMAGE_BUFFER_IPP

#include "image_buffer.hpp"
#include "log.hpp"

template<typename X, int N>
class ImageBuffer_add : public boost::static_visitor<void> {
public:

    ImageBuffer_add(das::Array<X, N> &elem, size_t rank, std::vector<ImageBufferEntry> &buffer)
    : elem_(elem), rank_(rank), buff_(buffer) {
    }

    void operator() (X data_type) const {
        if (elem_.numElements() > 0)
            buff_.push_back(ImageBufferEntry(elem_, rank_));
    }

    template<typename Y>
    void operator() (Y data_type) const {
        size_t size = elem_.numElements();

        if (size == 0) return;

        Y *data = new Y[size];
        size_t i = 0;
        for (typename das::Array<X, N>::iterator it = elem_.begin(); it != elem_.end(); ++it) {
            data[i] = *it;
            ++i;
        }
        das::Array<Y, N> a(data, elem_.shape(), das::deleteDataWhenDone);
        buff_.push_back(ImageBufferEntry(a,rank_));
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


template<class OutputIterator>
class ImageBuffer_copy : public boost::static_visitor<size_t> {
public:

    ImageBuffer_copy(
            OutputIterator &begin,
            const das::TinyVector<int, 11> &offset,
            const das::TinyVector<int, 11> &count,
            const das::TinyVector<int, 11> &stride,
            std::vector<ImageBufferEntry> &buff)
    : b_(begin), offset_(offset),
    count_(count), stride_(stride), buffer_(buff) {
    }

    template<typename T>
    size_t operator() (T &native_type) const {
        size_t count = 0;
        OutputIterator next = b_;

        std::vector<ImageBufferEntry>::iterator buff_it = buffer_.begin();
        if (buff_it == buffer_.end()) return count;

        size_t off0 = offset_[0];
        size_t off_tile0 = 0;

        const das::TinyVector<int, 11> &shape = buff_it->shape();

        while (off0 > 0 && buff_it != buffer_.end()) {
            off_tile0 = buff_it->shape()[0];
            if (off0 <= off_tile0) {
                off_tile0 -= off0;
                off0 = 0;
            } else {
                off0 -= off_tile0;
                ++buff_it;
            }
        }

        if (buff_it == buffer_.end()) return count;

        size_t dsp[] = {
            /*dim 0*/ shape[1] * shape[2] * shape[3] * shape[4] * shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
            /*dim 1*/ shape[2] * shape[3] * shape[4] * shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
            /*dim 2*/ shape[3] * shape[4] * shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
            /*dim 3*/ shape[4] * shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
            /*dim 4*/ shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
            /*dim 5*/ shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
            /*dim 6*/ shape[7] * shape[8] * shape[9] * shape[10],
            /*dim 7*/ shape[8] * shape[9] * shape[10],
            /*dim 8*/ shape[9] * shape[10],
            /*dim 9*/ shape[10],
            /*dim10*/ 1
        };

        const T * ptrs[11];

        size_t tiles = count_[0];

        while (tiles > 0 && buff_it != buffer_.end()) {
            ptrs[0] = buff_it->data<T>() + off_tile0 * dsp[0];
            off_tile0 = 0; //we need the remain offset only for the first batch of tiles
            size_t bucket_tiles = buff_it->shape()[0] < count_[0] ? shape[0] : count_[0];

            DAS_DBG_NO_SCOPE(
                    size_t DBG_batch_elem = buff_it->num_elements();
                    const T* DBG_batch_begin = buff_it->data<T>();
                    )

            for (size_t d0 = 0; d0 < bucket_tiles; ++d0) {
                if (d0 != 0)
                    ptrs[0] += dsp[0] * stride_[0];
                ptrs[1] = ptrs[0];
                for (size_t d1 = 0; d1 < count_[1]; ++d1) {
                    ptrs[1] += d1 == 0 ? dsp[1] * offset_[1] : dsp[1] * stride_[1];
                    ptrs[2] = ptrs[1];
                    for (size_t d2 = 0; d2 < count_[2]; ++d2) {
                        ptrs[2] += d2 == 0 ? dsp[2] * offset_[2] : dsp[2] * stride_[2];
                        ptrs[3] = ptrs[2];
                        for (size_t d3 = 0; d3 < count_[3]; ++d3) {
                            ptrs[3] += d3 == 0 ? dsp[3] * offset_[3] : dsp[3] * stride_[3];
                            ptrs[4] = ptrs[3];
                            for (size_t d4 = 0; d4 < count_[4]; ++d4) {
                                ptrs[4] += d4 == 0 ? dsp[4] * offset_[4] : dsp[4] * stride_[4];
                                ptrs[5] = ptrs[4];
                                for (size_t d5 = 0; d5 < count_[5]; ++d5) {
                                    ptrs[5] += d5 == 0 ? dsp[5] * offset_[4] : dsp[5] * stride_[5];
                                    ptrs[6] = ptrs[5];
                                    for (size_t d6 = 0; d6 < count_[6]; ++d6) {
                                        ptrs[6] += d6 == 0 ? dsp[6] * offset_[6] : dsp[6] * stride_[6];
                                        ptrs[7] = ptrs[6];
                                        for (size_t d7 = 0; d7 < count_[7]; ++d7) {
                                            ptrs[7] += d7 == 0 ? dsp[7] * offset_[7] : dsp[7] * stride_[7];
                                            ptrs[8] = ptrs[7];
                                            for (size_t d8 = 0; d8 < count_[8]; ++d8) {
                                                ptrs[8] += d8 == 0 ? dsp[8] * offset_[8] : dsp[8] * stride_[8];
                                                ptrs[9] = ptrs[8];
                                                for (size_t d9 = 0; d9 < count_[9]; ++d9) {
                                                    ptrs[9] += d9 == 0 ? dsp[9] * offset_[9] : dsp[9] * stride_[9];
                                                    ptrs[10] = ptrs[9];
                                                    for (size_t d10 = 0; d10 < count_[10]; ++d10) {
                                                        ptrs[10] += d10 == 0 ? dsp[10] * offset_[10] : dsp[10] * stride_[10];
                                                        DAS_DBG_NO_SCOPE(
                                                                size_t DBG_offset = ptrs[10] - DBG_batch_begin;
                                                        if (DBG_offset > DBG_batch_elem)
                                                                DAS_LOG_DBG("IMAGE BUFFER OVERFLOW!!! requested offset " << DBG_offset << " in array[" << DBG_batch_elem << "]");
                                                        return count;
                                                                );
                                                        *next++ = *ptrs[10];
                                                        ++count;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return count;
    }

private:
    OutputIterator b_;
    const das::TinyVector<int, 11> &offset_;
    const das::TinyVector<int, 11> &count_;
    const das::TinyVector<int, 11> &stride_;
    std::vector<ImageBufferEntry> &buffer_;
};

template<class OutputIterator>
size_t
ImageBuffer::copy(OutputIterator& begin,
        const das::TinyVector<int, 11>& offset,
        const das::TinyVector<int, 11>& count,
        const das::TinyVector<int, 11>& stride) {
    
    if (!is_init_) {
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();
    }
    
    if (size0_ < stride[0] * count[0] + offset[0])
        throw das::bad_array_slice();
    if (size1_ < stride[1] * count[1] + offset[1])
        throw das::bad_array_slice();
    if (size2_ < stride[2] * count[2] + offset[2])
        throw das::bad_array_slice();
    if (size3_ < stride[3] * count[3] + offset[3])
        throw das::bad_array_slice();
    if (size4_ < stride[4] * count[4] + offset[4])
        throw das::bad_array_slice();
    if (size5_ < stride[5] * count[5] + offset[5])
        throw das::bad_array_slice();
    if (size6_ < stride[6] * count[6] + offset[6])
        throw das::bad_array_slice();
    if (size7_ < stride[7] * count[7] + offset[7])
        throw das::bad_array_slice();
    if (size8_ < stride[8] * count[8] + offset[8])
        throw das::bad_array_slice();
    if (size9_ < stride[9] * count[9] + offset[9])
        throw das::bad_array_slice();
    if (size10_ < stride[10] * count[10] + offset[10])
        throw das::bad_array_slice();

    
    return boost::apply_visitor(
            ImageBuffer_copy<OutputIterator>(begin,offset,count,stride,buffer_),
            type_);
}
#endif