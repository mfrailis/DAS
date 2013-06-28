#ifndef RESULT_HPP
#define	RESULT_HPP
#include <odb/database.hxx>
#include "result_iterator.hpp"
#include "db_bundle.ipp"

namespace das {namespace tpl {
    
    template<typename T>
    class Result : private odb::result<T> {
        public:
            typedef result_iterator<T> iterator;
            typedef odb::result<T> super;          
            Result(const odb::result<T> &r, const DbBundle &db) : odb::result<T>(r), bundle_(db) {
            }
            
            iterator
            begin(){return iterator(super::begin(),bundle_.lock());}
            
            iterator
            end(){return iterator(super::end(),bundle_.lock());}           
            
            private:

                WeakDbBundle bundle_;
    };
}}
#endif	/* RESULT_HPP */

