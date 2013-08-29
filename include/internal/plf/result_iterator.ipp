#include "../../tpl/result_iterator.hpp"

namespace das {
    namespace plf {

        class ResultIteratorWrapper {
        public:
            virtual long long id() = 0;

            virtual const shared_ptr<DasObject> load() = 0;

            virtual DasObject& operator*() = 0;

            virtual const shared_ptr<DasObject> operator-> () = 0;

            virtual void operator++() = 0;

            virtual ResultIteratorWrapper* operator++(int) = 0;

            virtual bool equal(const shared_ptr<ResultIteratorWrapper> &j) const = 0;

            virtual ~ResultIteratorWrapper() {
            }

        };

        template<typename Das_type>
        class ResultIteratorWrapperImp
        : public ResultIteratorWrapper, private tpl::result_iterator<Das_type> {
            typedef tpl::result_iterator<Das_type> super;
            friend class tpl::result_iterator<Das_type>;
        public:

            ResultIteratorWrapperImp(const super& res) : super(res) {
            }

            virtual long long id() {
                return super::id();
            }

            virtual const shared_ptr<DasObject> load() {
                return super::load();
            }

            virtual DasObject& operator*() {
                return super::operator*();
            }

            virtual const shared_ptr<DasObject> operator-> () {
                return super::operator-> ();
            }

            virtual void operator++() {
                super::operator++();
            }

            virtual ResultIteratorWrapper* operator++(int) {
                ResultIteratorWrapper* riw =
                        new ResultIteratorWrapperImp(super::operator++(1));
                return riw;
            }

            virtual bool equal(const shared_ptr<ResultIteratorWrapper> &j) const {
                typedef ResultIteratorWrapperImp<Das_type> this_type;

                shared_ptr<this_type> j_ =
                        std::tr1::dynamic_pointer_cast<this_type> (j);

                const super& s_ = static_cast<super>(*j_);
                return super::equal(s_);
            }

            virtual ~ResultIteratorWrapperImp() {
            }

        };

        inline
        long long
        result_iterator::id() {
            return w_->id();
        }

        inline
        const shared_ptr<DasObject>
        result_iterator::load() {
            return w_->load();
        }

        inline
        DasObject&
        result_iterator::operator*() {
            return w_->operator*();
        }

        inline
        const shared_ptr<DasObject>
        result_iterator::operator-> () {
            return w_->operator->();
        }

        inline
        result_iterator&
        result_iterator::operator++() {
            w_->operator++();
            return *this;
        }

        inline
        result_iterator
        result_iterator::operator++(int) {
            ResultIteratorWrapper* riw = w_->operator++(1);
            return result_iterator(riw);
        }

        inline
        bool
        result_iterator::equal(result_iterator j) const {
            /* 
             * ResultIteratorWrapper is a polimorphic object so typeid
             * comparsion does not return false positive.
             */
            return (typeid (*w_) == typeid (*(j.w_))) && w_->equal(j.w_);
        }

        class ResultIteratorWrapperConst {
        public:
            virtual long long id() = 0;

            virtual const DasObject& operator*() = 0;

            virtual const DasObject* operator-> () = 0;

            virtual void operator++() = 0;

            virtual ResultIteratorWrapperConst* operator++(int) = 0;

            virtual bool equal(const shared_ptr<ResultIteratorWrapperConst> &j) const = 0;
        };

        template<typename Das_type>
        class ResultIteratorWrapperConstImp
        : public ResultIteratorWrapperConst, private tpl::result_const_iterator<Das_type> {
            typedef tpl::result_const_iterator<Das_type> super;
            friend class tpl::result_const_iterator<Das_type>;
        public:

            ResultIteratorWrapperConstImp(const super& res) : super(res) {
            }

            virtual long long id() {
                return super::id();
            }

            virtual const DasObject& operator*() {
                return super::operator*();
            }

            virtual const DasObject* operator-> () {
                return super::operator-> ();
            }

            virtual void operator++() {
                super::operator++();
            }

            virtual ResultIteratorWrapperConst* operator++(int) {
                ResultIteratorWrapperConst* riw =
                        new ResultIteratorWrapperConstImp(super::operator++(1));
                return riw;
            }

            virtual bool equal(const shared_ptr<ResultIteratorWrapperConst> &j) const {
                typedef ResultIteratorWrapperConstImp<Das_type> this_type;

                shared_ptr<this_type> j_ =
                        std::tr1::dynamic_pointer_cast<this_type> (j);
                
                const super& s_ = static_cast<super>(*j_);
                return super::equal(s_);
            }

            virtual ~ResultIteratorWrapperConstImp() {
            }
        };

        inline
        long long
        result_const_iterator::id() {
            return w_->id();
        }

        inline
        const DasObject&
        result_const_iterator::operator*() {
            return w_->operator*();
        }

        inline
        const DasObject*
        result_const_iterator::operator-> () {
            return w_->operator->();
        }

        inline
        result_const_iterator&
        result_const_iterator::operator++() {
            w_->operator++();
            return *this;
        }

        inline
        result_const_iterator
        result_const_iterator::operator++(int) {
            ResultIteratorWrapperConst* riw = w_->operator++(1);
            return result_const_iterator(riw);
        }

        inline
        bool
        result_const_iterator::equal(result_const_iterator j) const {
            /* 
             * ResultIteratorWrapperConst is a polimorphic object so typeid
             * comparsion does not return false positive.
             */
            return (typeid (*w_) == typeid (*(j.w_))) && w_->equal(j.w_);
        }

    }
}