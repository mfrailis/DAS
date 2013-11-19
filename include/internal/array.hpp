#ifndef ARRAY_HPP
#define	ARRAY_HPP
#include <blitz/array.h>
#include <exception>

namespace das {
 
    enum buffer_policy {
        duplicateData = blitz::duplicateData,
        deleteDataWhenDone = blitz::deleteDataWhenDone,
        neverDeleteData = blitz::neverDeleteData
    };


    class Deallocator {
    public:
        virtual void operator() () {}
        virtual ~Deallocator(){}
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

    inline TinyVector<int, 1> shape(int n1) {
        return TinyVector<int, 1>(n1);
    }

    inline TinyVector<int, 2> shape(int n1, int n2) {
        return TinyVector<int, 2>(n1, n2);
    }

    inline TinyVector<int, 3> shape(int n1, int n2, int n3) {
        return TinyVector<int, 3>(n1, n2, n3);
    }

    inline TinyVector<int, 4> shape(int n1, int n2, int n3, int n4) {
        return TinyVector<int, 4>(n1, n2, n3, n4);
    }

    inline TinyVector<int, 5> shape(int n1, int n2, int n3, int n4,
            int n5) {
        return TinyVector<int, 5>(n1, n2, n3, n4, n5);
    }

    inline TinyVector<int, 6> shape(int n1, int n2, int n3, int n4,
            int n5, int n6) {
        return TinyVector<int, 6>(n1, n2, n3, n4, n5, n6);
    }

    inline TinyVector<int, 7> shape(int n1, int n2, int n3, int n4,
            int n5, int n6, int n7) {
        return TinyVector<int, 7>(n1, n2, n3, n4, n5, n6, n7);
    }

    inline TinyVector<int, 8> shape(int n1, int n2, int n3, int n4,
            int n5, int n6, int n7, int n8) {
        return TinyVector<int, 8>(n1, n2, n3, n4, n5, n6, n7, n8);
    }

    inline TinyVector<int, 9> shape(int n1, int n2, int n3, int n4,
            int n5, int n6, int n7, int n8, int n9) {
        return TinyVector<int, 9>(n1, n2, n3, n4, n5, n6, n7, n8, n9);
    }

    inline TinyVector<int, 10> shape(int n1, int n2, int n3, int n4,
            int n5, int n6, int n7, int n8, int n9, int n10) {
        return TinyVector<int, 10>(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10);
    }

    inline TinyVector<int, 11> shape(int n1, int n2, int n3, int n4,
            int n5, int n6, int n7, int n8, int n9, int n10, int n11) {
        return TinyVector<int, 11>(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11);
    }

    class bad_range : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "bad range values";
        }

    };

    class Range : public blitz::Range {
        typedef blitz::Range super;
    public:

        Range() : super() {
        }

        Range(int slicePosition) : super(slicePosition) {
        }

        Range(int first, int last, int stride = 1) : super(first, last, stride) {
            if (stride < 1) throw bad_range();
            if (last < first) throw bad_range();
            if (first < 0) throw bad_range();
        }

        size_t length() const {
            size_t len = last() - first();
            int div = len / stride();
            if (len % stride() != 0)
                return div + 1;
            else
                return div;
        }

        static Range all() {
            return Range();
        }
    };

    template<typename P_numtype, int N_Rank = 1 >
    class Array : public blitz::Array<P_numtype, N_Rank> {
        typedef blitz::Array<P_numtype, N_Rank> super;
    public:

        Array(P_numtype *buffer, const TinyVector<int, N_Rank> &shape, buffer_policy flag,
              Deallocator *dealloc = 0)
        : super(buffer, shape, (blitz::preexistingMemoryPolicy) flag), policy_(flag), dealloc_(dealloc)  {
        }

        Array() : super() {
        }
        
        Array(const TinyVector<int, N_Rank>& extent)
        : super(extent), policy_(deleteDataWhenDone), dealloc_(0)
        {
        }
        
        
        ~Array() {
            if (policy_ == neverDeleteData && super::numReferences() == 1)
                if (dealloc_)
                  {
                    (*dealloc_)();
                    delete dealloc_;
                    dealloc_ = 0;
                  }
        }
        
    private:
        buffer_policy policy_;
        Deallocator *dealloc_;

    };

    template<typename P_numtype, int N_Rank = 1 >
    class ColumnArray : public Array< Array<P_numtype, N_Rank> > {
        typedef Array< Array<P_numtype, N_Rank> > super;
        typedef Array<P_numtype, N_Rank> item;
    public:

        ColumnArray(item* buffer, size_t size, buffer_policy flag) :
        super(buffer, TinyVector<int, 1>(size), flag) {
        }

        ColumnArray() : super() {
        }
        

    };

}
#endif	/* ARRAY_HPP */

