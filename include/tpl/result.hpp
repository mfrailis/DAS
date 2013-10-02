#ifndef RESULT_TPL_HPP
#define	RESULT_TPL_HPP
#include <odb/database.hxx>
#include "result_iterator.hpp"
#include "../internal/db_bundle.ipp"

namespace das {
    namespace tpl {

        /**
         * @brief Template container of a query result.
         * 
         * It provides const and non const access to the result of a database
         * query through iterators.
         */
        template<typename T>
        class Result : private odb::result<T> {
        public:
            typedef result_iterator<T> iterator;
            typedef result_const_iterator<T> const_iterator;
            typedef odb::result<T> super;

            Result(const odb::result<T> &r, const DbBundle &db) : odb::result<T>(r), bundle_(db) {
            }

            iterator
            begin() {
                return iterator(super::begin(), bundle_.lock());
            }

            iterator
            end() {
                return iterator(super::end(), bundle_.lock());
            }

            const_iterator
            cbegin(){
                return const_iterator(super::begin());
            }

            const_iterator
            cend(){
                return const_iterator(super::end());
            }

            bool
            empty() const{
                return super::empty();
            }

            size_t
            size() const{
                return super::size();
            }

        private:

            WeakDbBundle bundle_;
        };
    }
}
#endif	/* RESULT_HPP */

