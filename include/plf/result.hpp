#ifndef RESULT_PLF_HPP
#define	RESULT_PLF_HPP
#include "result_iterator.hpp"
namespace das {
    namespace plf {
        class ResultWrapper;
        
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

        private:
            shared_ptr<ResultWrapper> w_;
        };
    }
}
#include "../internal/plf/result.ipp"
#endif