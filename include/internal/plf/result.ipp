#include "../../tpl/result.hpp"
#include "../../plf/result_iterator.hpp"

namespace das {
    namespace plf {

        class ResultWrapper {
        public:
            virtual result_iterator begin() = 0;

            virtual result_iterator end() = 0;

            virtual result_const_iterator cbegin() = 0;

            virtual result_const_iterator cend() = 0;
            
            virtual bool empty() const = 0;
            
            virtual size_t size() const = 0;

            virtual ~ResultWrapper(){}
        };

        template<typename Das_type>
        class ResultWrapperImp : public ResultWrapper, private tpl::Result<Das_type> {
            typedef tpl::Result<Das_type> super;
        public:
            ResultWrapperImp(const super& res) : super(res) {
            }

            virtual result_iterator begin(){
                ResultIteratorWrapper* riw=
                        new ResultIteratorWrapperImp<Das_type>(super::begin());
                return result_iterator(riw);
            }

            virtual result_iterator end(){
                ResultIteratorWrapper* riw=
                        new ResultIteratorWrapperImp<Das_type>(super::end());
                return result_iterator(riw);
            }

            virtual result_const_iterator cbegin(){
                ResultIteratorWrapperConst* riw=
                        new ResultIteratorWrapperConstImp<Das_type>(super::cbegin());
                return result_const_iterator(riw);
            }

            virtual result_const_iterator cend(){
                ResultIteratorWrapperConst* riw=
                        new ResultIteratorWrapperConstImp<Das_type>(super::cend());
                return result_const_iterator(riw);
            }
            
            virtual bool empty() const{
                return super::empty();
            }
            
            virtual size_t size() const{
                return super::size();
            }
            
            

            virtual ~ResultWrapperImp() {
            }
        };

        inline
        result_iterator
        Result::begin() {
            return w_->begin();
        }

        inline
        result_iterator
        Result::end() {
            return w_->end();
        }

        inline
        result_const_iterator
        Result::cbegin(){
            return w_->cbegin();
        }

        inline
        result_const_iterator
        Result::cend(){
            return w_->cend();
        }


    }
}