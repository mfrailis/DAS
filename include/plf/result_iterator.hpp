#ifndef DAS_PLF_RESULT_ITERATOR_HPP
#define	DAS_PLF_RESULT_ITERATOR_HPP
#include <typeinfo>

namespace das {
    namespace plf {

        class ResultIteratorWrapper;
        class ResultIteratorWrapperConst;

        class result_iterator {
        public:

            typedef std::ptrdiff_t difference_type;
            typedef std::input_iterator_tag iterator_category;

            explicit result_iterator(ResultIteratorWrapper* wrapper)
            : w_(wrapper) {
            }

            long long id();

            const shared_ptr<DasObject> load();

            DasObject& operator*();

            const shared_ptr<DasObject> operator-> ();

            result_iterator& operator++();

            result_iterator
            operator++(int);

            bool
            equal(result_iterator j)const;

        private:
            shared_ptr<ResultIteratorWrapper> w_;
        };

        inline bool
        operator!=(const result_iterator &i, const result_iterator &j) {
            return !i.equal(j);
        }

        inline bool
        operator==(const result_iterator &i, const result_iterator &j) {
            return i.equal(j);
        }

        class result_const_iterator {
        public:

            typedef std::ptrdiff_t difference_type;
            typedef std::input_iterator_tag iterator_category;

            explicit result_const_iterator(ResultIteratorWrapperConst* wrapper)
            : w_(wrapper) {
            }

            long long id();

            const DasObject& operator*();

            const DasObject* operator-> ();

            result_const_iterator& operator++();

            result_const_iterator operator++(int);

            bool equal(result_const_iterator j) const;
        private:
            shared_ptr<ResultIteratorWrapperConst> w_;
        };

        inline bool
        operator!=(const result_const_iterator &i, const result_const_iterator &j) {
            return !i.equal(j);
        }

        inline bool
        operator==(const result_const_iterator &i, const result_const_iterator &j) {
            return i.equal(j);
        }
    }
}
#include "../internal/plf/result_iterator.ipp"
#endif