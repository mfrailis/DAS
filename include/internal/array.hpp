#ifndef ARRAY_HPP
#define	ARRAY_HPP
#include <blitz/array.h>
#include <tr1/memory>

namespace das {
 
    using std::tr1::shared_ptr;

    enum buffer_policy {
        duplicateData = blitz::duplicateData,
        deleteDataWhenDone = blitz::deleteDataWhenDone,
        neverDeleteData = blitz::neverDeleteData
    };


    class Deallocator {
    public:
        virtual void operator() () {}
    };
    
    template<typename Num_type, int Length>
    class TinyVector : public blitz::TinyVector<Num_type, Length> {
        typedef blitz::TinyVector<Num_type, Length> super;
    public:
        
        TinyVector() : super() {
        }       
        
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

    template<typename P_numtype, int N_Rank = 1>
    class Array : public blitz::Array<P_numtype, N_Rank> {
        typedef blitz::Array<P_numtype, N_Rank> super;
    public:

        Array(P_numtype *buffer, const TinyVector<int, N_Rank> &shape, buffer_policy flag,
              Deallocator *dealloc = 0)
        : super(buffer, shape, (blitz::preexistingMemoryPolicy) flag), policy_(flag), dealloc_(dealloc)  {
        }

        Array() : super() {
        }
        
        ~Array() {
            if (policy_ == neverDeleteData && super::numReferences() == 1)
                if (dealloc_)
                    (*dealloc_)();         
        }
        
    private:
        buffer_policy policy_;
        shared_ptr<Deallocator> dealloc_;
    };


}
#endif	/* ARRAY_HPP */

