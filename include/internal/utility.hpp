#ifndef UTILITY_HPP
#define	UTILITY_HPP
#include <vector>
#include <utility> // std::pair
#include "array.hpp"

namespace das{
    
    template<typename T>
    class ArrayDeleter {
    public:

        void operator() (T* t) {
            delete [] t;
        }
    };
    
    template<typename T>
    class ColumnArrayBuffer{
        typedef std::pair<size_t,T*> item;
        typedef std::vector<item> buff_type;

        public:
            ColumnArrayBuffer(){
            }
            
            ColumnArrayBuffer(const ColumnArrayBuffer& rhs){
                buff_.swap(rhs.buff_);
            }
            
            ColumnArrayBuffer&
            operator =(const ColumnArrayBuffer& rhs){
                buff_.swap(rhs.buff_);  
            }
            
            size_t
            size(){
                return buff_.size();
            }
            
            void
            add(T* ptr, size_t size){
                buff_.push_back(item(size,ptr));
            }
            
            
            template<typename U, int N_Rank>
            void
            release(Array<U,N_Rank> *buffer, size_t count, const TinyVector<int,N_Rank>& shape){
                if(count != size())
                    throw das::wrong_size();
                
                size_t elems = 1;
                for(size_t i=1; i<N_Rank; ++i)
                    elems *= shape(i);
                
                for(typename buff_type::iterator it=buff_.begin(); it!=buff_.end(); ++it)
                    if(it->first % elems != 0)
                        throw das::bad_array_shape();
                
                TinyVector<int,N_Rank> s(shape);
                Array<U,N_Rank> *b = new Array<U,N_Rank>[size()];
                for(size_t i=0; i<count; ++i){
                    s(0) = buff_[i].first / elems;
                    U* tmp = new U[buff_[i].first];
                    for(size_t j=0; j<buff_[i].first; ++j)
                        tmp[j] = buff_[i].second[j];
                    buffer[i] = Array<U,N_Rank>(tmp,s,deleteDataWhenDone);
                    delete [] buff_[i].second;
                }
                
                buff_.clear();
            }            
            
            template<int N_Rank>
            void
            release(Array<T,N_Rank> *buffer, size_t count, const TinyVector<int,N_Rank>& shape){
                if(count != size())
                    throw das::wrong_size();
                
                size_t elems = 1;
                for(size_t i=1; i<N_Rank; ++i)
                    elems *= shape(i);
                
                for(typename buff_type::iterator it=buff_.begin(); it!=buff_.end(); ++it)
                    if(it->first % elems != 0)
                        throw das::bad_array_shape();
                
                TinyVector<int,N_Rank> s(shape);
                for(size_t i=0; i<count; ++i){
                    s(0) = buff_[i].first / elems;
                    buffer[i] = Array<T,N_Rank>(buff_[i].second,s,deleteDataWhenDone);
                }
                
                buff_.clear();
            }
            
            ~ColumnArrayBuffer(){
                for(typename buff_type::iterator it=buff_.begin(); it!=buff_.end(); ++it)
                    delete [] it->second;
            }
        
        private:
            mutable buff_type buff_;
    };
    
}


#endif	/* UTILITY_HPP */

