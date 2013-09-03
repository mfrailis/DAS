#ifndef DAS_PLF_RESULT_ITERATOR_HPP
#define	DAS_PLF_RESULT_ITERATOR_HPP
#include <typeinfo>

namespace das {
    namespace plf {

        class ResultIteratorWrapper;
        class ResultIteratorWrapperConst;

        /**
         * @brief implement the stream-like iterator interface.
         * 
         * For convenience the id() method is provided which does not require
         * any object loading. The load method allows the user to manually
         * load an object from the Result container.
         */
        class result_iterator {
        public:

            typedef std::ptrdiff_t difference_type;
            typedef std::input_iterator_tag iterator_category;

            explicit result_iterator(ResultIteratorWrapper* wrapper)
            : w_(wrapper) {
            }

            /**
             * Retrieves the id of the current result-object without loading it
             */
            long long id();

            /**
             * Explicitly loads the object in the current session. If the object
             * was already present in the session, the same shared_ptr is returned.
             */
            const shared_ptr<DasObject>
            load();

            /**
             * Implicitly loads the object in the current session if it wasn't.
             * @return reference to the object stored in the current session.
             */
            DasObject&
            operator*();

            /**
             * Implicitly loads the object in the current session if it wasn't.
             * @return the shared_ptr stored in the current session.
             */
            const shared_ptr<DasObject>
            operator-> ();

            result_iterator&
            operator++();

            /**
             * Due to the stream-like nature of the iterator, all the iterators
             * must refer to the same object, so pre and post increment are
             * semantically interchangeable, but pre-increment avoids iterator 
             * copies.
             */
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

        /**
         * @brief implement the stream-like const_iterator interface.
         * 
         * These methods avoid to load the objects in the session returning
         * only const and temporary references.
         */
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
