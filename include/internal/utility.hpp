#ifndef UTILITY_HPP
#define	UTILITY_HPP
#include <vector>
#include <utility> // std::pair
#include <boost/any.hpp>
#include <odb/tr1/memory.hxx>
#include "array.hpp"
#include "../exceptions.hpp"

namespace das {

    template<typename T>
    class ArrayDeleter {
    public:

        void operator() (T* t) {
            delete [] t;
        }
    };

    template<typename T>
    class ArrayStore {
    public:
        typedef T* iterator;

        template<int Rank>
        ArrayStore(Array<T, Rank> &a) :
        ptr_(a.data()),
        owner_(a),
        shape_(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
        rank_(a.rank()) {
            for (int i = 0; i < Rank; ++i)
                shape_(i) = a.extent(i);
        }

        ArrayStore(T* p, size_t dim) :
        ptr_(p),
        owner_(std::tr1::shared_ptr<T>(p,ArrayDeleter<T>())),
        shape_(dim, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
        rank_(1) {

        }

        template<int Rank>
        ArrayStore(T* p, const TinyVector<int, Rank>& s) :
        ptr_(p),
        owner_(std::tr1::shared_ptr<T>(p,ArrayDeleter<T>())),
        shape_(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1),
        rank_(s.length()) {
            for (int i = 0; i < Rank; ++i)
                shape_(i) = s(i);
        }

        T*
        data() const {
            return ptr_;
        }

        template<typename U, int Rank>
        Array<U, Rank>
        copy_array() {
            if (Rank != rank_)
                throw das::bad_array_size();
            
            size_t s = size();
            
            U* data = new U[s];
            for(size_t j=0; j<s;++j)
                data[j] = ptr_[j];
            
            TinyVector<int, Rank> shape;
            for (int i = 0; i < Rank; ++i)
                shape(i) = shape_(i);

            return Array<U,Rank>(data,shape,deleteDataWhenDone);
        }

        iterator
        begin() {
            return ptr_;
        }

        iterator
        end() {
            return ptr_ + size();
        }

        std::pair<T*, size_t>
        pair() const {
            return std::pair<T*, size_t>(ptr_, size());
        }

        size_t size() const {
            size_t s = 1;
            for (int i = 0; i < 11; ++i)
                s *= shape_(i);
            return s;
        }

        size_t rank() const {
            return rank_;
        }

    private:
        T* ptr_;
        boost::any owner_;
        TinyVector<int, 11> shape_;
        size_t rank_;
    };

    template<typename T>
    class ColumnArrayBuffer {
        typedef std::pair<size_t, T*> item;
        typedef std::vector<item> buff_type;

    public:

        ColumnArrayBuffer() {
        }

        ColumnArrayBuffer(const ColumnArrayBuffer& rhs) {
            buff_.swap(rhs.buff_);
        }

        ColumnArrayBuffer&
                operator =(const ColumnArrayBuffer& rhs) {
            buff_.swap(rhs.buff_);
        }

        size_t
        size() {
            return buff_.size();
        }

        void
        add(T* ptr, size_t size) {
            buff_.push_back(item(size, ptr));
        }

        template<typename U, int N_Rank>
        void
        release(Array<U, N_Rank> *buffer, size_t count, const TinyVector<int, N_Rank>& shape) {
            if (count != size())
                throw das::wrong_size();

            size_t elems = 1;
            for (size_t i = 1; i < N_Rank; ++i)
                elems *= shape(i);

            for (typename buff_type::iterator it = buff_.begin(); it != buff_.end(); ++it)
                if (it->first % elems != 0)
                    throw das::bad_array_shape();

            TinyVector<int, N_Rank> s(shape);
            Array<U, N_Rank> *b = new Array<U, N_Rank>[size()];
            for (size_t i = 0; i < count; ++i) {
                s(0) = buff_[i].first / elems;
                U* tmp = new U[buff_[i].first];
                for (size_t j = 0; j < buff_[i].first; ++j)
                    tmp[j] = buff_[i].second[j];
                buffer[i].reference(Array<U, N_Rank>(tmp, s, deleteDataWhenDone));
                delete [] buff_[i].second;
            }

            buff_.clear();
        }

        template<int N_Rank>
        void
        release(Array<T, N_Rank> *buffer, size_t count, const TinyVector<int, N_Rank>& shape) {
            if (count != size())
                throw das::wrong_size();

            size_t elems = 1;
            for (size_t i = 1; i < N_Rank; ++i)
                elems *= shape(i);

            for (typename buff_type::iterator it = buff_.begin(); it != buff_.end(); ++it)
                if (it->first % elems != 0)
                    throw das::bad_array_shape();

            TinyVector<int, N_Rank> s(shape);
            for (size_t i = 0; i < count; ++i) {
                s(0) = buff_[i].first / elems;
                buffer[i].reference(Array<T, N_Rank>(buff_[i].second, s, deleteDataWhenDone));
            }

            buff_.clear();
        }

        ~ColumnArrayBuffer() {
            for (typename buff_type::iterator it = buff_.begin(); it != buff_.end(); ++it)
                delete [] it->second;
        }

    private:
        mutable buff_type buff_;
    };

}


#endif	/* UTILITY_HPP */

