#ifndef RESULT_PLF_HPP
#define	RESULT_PLF_HPP
#include "result_iterator.hpp"
namespace das {
    namespace plf {
        class ResultWrapper;
        /**
         * @brief Polymorphic container of a query result.
         * 
         * It provides const and non const access to the result of a database
         * query through iterators.
         */
        class Result{
        public:
            typedef das::plf::result_iterator iterator;
            typedef das::plf::result_const_iterator const_iterator;

            Result(ResultWrapper* wrapper) : w_(wrapper){
            }

            iterator
            begin();

            iterator
            end();        

            const_iterator
            cbegin();

            const_iterator
            cend();
            
            bool
            empty() const;

            size_t
            size() const;

        private:
            shared_ptr<ResultWrapper> w_;
        };
    }
}
#include "../internal/plf/result.ipp"
#endif
