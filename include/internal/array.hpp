#ifndef ARRAY_HPP
#define	ARRAY_HPP
#include <blitz/array.h>

namespace das {

    enum buffer_policy {
        duplicateData = blitz::duplicateData,
        deleteDataWhenDone = blitz::deleteDataWhenDone,
        neverDeleteData = blitz::neverDeleteData
    };

    template<typename Num_type, int Length>
    class TinyVector : public blitz::TinyVector<Num_type, Length> {
        typedef blitz::TinyVector<Num_type, Length> super;
    public:
        TinyVector(const super &vector) : super(vector) {
        }
        
        TinyVector(Num_type x0) : super(x0) {
        }

        TinyVector(Num_type x0, Num_type x1) : super(x0, x1) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2) : super(x0, x1, x2) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2, Num_type x3)
        : super(x0, x1, x2, x3) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2, Num_type x3,
                Num_type x4)
        : super(x0, x1, x2, x3, x4) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2, Num_type x3,
                Num_type x4, Num_type x5)
        : super(x0, x1, x2, x3, x4, x5) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2, Num_type x3,
                Num_type x4, Num_type x5, Num_type x6)
        : super(x0, x1, x2, x3, x4, x5, x5) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2, Num_type x3,
                Num_type x4, Num_type x5, Num_type x6, Num_type x7)
        : super(x0, x1, x2, x3, x4, x5, x6, x7) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2, Num_type x3,
                Num_type x4, Num_type x5, Num_type x6, Num_type x7,
                Num_type x8)
        : super(x0, x1, x2, x3, x4, x5, x6, x7, x8) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2, Num_type x3,
                Num_type x4, Num_type x5, Num_type x6, Num_type x7,
                Num_type x8, Num_type x9)
        : super(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) {
        }

        TinyVector(Num_type x0, Num_type x1, Num_type x2, Num_type x3,
                Num_type x4, Num_type x5, Num_type x6, Num_type x7,
                Num_type x8, Num_type x9, Num_type x10)
        : super(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) {
        }


    };

    template<typename P_numtype, int N_Rank = 1 >
    class Array : public blitz::Array<P_numtype, N_Rank> {
        typedef blitz::Array<P_numtype, N_Rank> super;
    public:

        Array(P_numtype *buffer, const TinyVector<int, N_Rank> &shape, buffer_policy flag)
        : super(buffer, shape, flag) {
        }

        Array() : super() {
        }
    };

    template<typename T>
    class Array<T, 1> : public blitz::Array<T, 1> {
        typedef blitz::Array<T, 1> super;
    public:

        Array(T *buffer, size_t length, buffer_policy flag)
        : super(buffer, blitz::shape(length), flag) {
        }

        Array() : super() {
        }
    };

}
#endif	/* ARRAY_HPP */

