#ifndef ARRAY_HPP
#define	ARRAY_HPP
#include <blitz/array.h>

namespace das {

    enum buffer_policy {
        duplicateData = blitz::duplicateData,
        deleteDataWhenDone = blitz::deleteDataWhenDone,
        neverDeleteData = blitz::neverDeleteData
    };

    template<typename T>
    class Array : public blitz::Array<T, 1> {
        typedef blitz::Array<T, 1> super;
    public:

        Array(T *buffer, size_t length, buffer_policy flag) 
        : super(buffer, blitz::shape(length), flag) {
        }

        Array() : super() {
        }
    };

    template<typename T>
    class Image : public blitz::Array<T, 2> {
    };

}
#endif	/* ARRAY_HPP */

