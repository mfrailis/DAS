#ifndef DAS_TPL_RESULT_ITERATOR_HPP
#define	DAS_TPL_RESULT_ITERATOR_HPP
#include <odb/object-result.hxx>
#include "../internal/db_bundle.ipp"

namespace das {
    namespace tpl {

        /**
         * @brief implement the stream-like iterator interface.
         * 
         * For convenience the id() method is provided which does not require
         * any object loading. The load method allows the user to manually
         * load an object from the Result container.
         */
        template <typename T>
        class result_iterator : private odb::result_iterator<T, odb::class_object> {
        private:
            typedef odb::result_iterator<T, odb::class_object> super;

        public:

            typedef std::ptrdiff_t difference_type;
            typedef std::input_iterator_tag iterator_category;

            explicit result_iterator(const super& it, const DbBundle &db)
            : super(it),
            bundle_(db),
            loaded_(new bool()),
            entry_(new shared_ptr<T>()) {
                *loaded_ = false;
            }

            /**
             * Retrieves the id of the current result-object without loading it
             */
            long long
            id() {
                return super::id();
            }
            
            /**
             * Explicitly loads the object in the current session. If the object
             * was already present in the session, the same shared_ptr is returned.
             */
            const shared_ptr<T>&
            load() {
                if (! *loaded_) {
                    DbBundle db = bundle_.lock(true);
                    shared_ptr<odb::session> s = db.lock_session(true);
                    odb::session::current(*s);
                    *entry_ = super::load();
                    (*entry_)->self_ = *entry_;
                    db.attach(*entry_,false);
                    *loaded_ = true;
                }
                return *entry_;
            }
            
            /**
             * Implicitly loads the object in the current session if it wasn't.
             * @return reference to the object stored in the current session.
             */
            T&
            operator*() {
                load();
                return super::operator*();
            }
            
            
            /**
             * Implicitly loads the object in the current session if it wasn't.
             * @return the shared_ptr stored in the current session.
             */
            const shared_ptr<T>&
            operator-> () {
                return load();
            }

            result_iterator&
            operator++() {
                super::operator ++();
                *loaded_ = false;
                return *this;
            }

            /**
             * Due to the stream-like nature of the iterator, all the iterators
             * must refer to the same object, so pre and post increment are
             * semantically interchangeable, but pre-increment avoids iterator 
             * copies.
             */
            result_iterator
            operator++(int) {
                // All non-end iterators for a result object move together.
                //    
                super::operator ++();
                *loaded_ = false;
                return *this;
            }

            bool
            equal(result_iterator j) const {
                return super::equal(j);
            }

        private:
            shared_ptr<bool> loaded_;
            shared_ptr< shared_ptr<T> > entry_;
            WeakDbBundle bundle_;
        };

        template <typename T>
        inline bool
        operator!=(const result_iterator<T> &i, const result_iterator<T> &j) {
            return !i.equal(j);
        }

        template <typename T>
        inline bool
        operator==(const result_iterator<T> &i, const result_iterator<T> &j) {
            return i.equal(j);
        }
        
        /**
         * @brief implement the stream-like const_iterator interface.
         * 
         * These methods avoid to load the objects in the session returning
         * only const and temporary references.
         */
        template <typename T>
        class result_const_iterator : private odb::result_iterator<T, odb::class_object> {
        private:
            typedef odb::result_iterator<T, odb::class_object> super;

        public:

            typedef std::ptrdiff_t difference_type;
            typedef std::input_iterator_tag iterator_category;

            explicit result_const_iterator(const super& it) : super(it) {
            }

            long long
            id() {
                return super::id();
            }

            const T&
            operator*() {
                odb::session::reset_current();
                return super::operator*();
            }

            const T*
            operator-> () {
                odb::session::reset_current();
                return &(super::operator*());
            }

            result_const_iterator&
            operator++() {
                super::operator ++();
                return *this;
            }

            result_const_iterator
            operator++(int) {
                // All non-end iterators for a result object move together.
                //    
                super::operator ++();
                return *this;
            }

            bool
            equal(result_const_iterator j) const {
                return super::equal(j);
            }
        };

        template <typename T>
        inline bool
        operator!=(const result_const_iterator<T> &i, const result_const_iterator<T> &j) {
            return !i.equal(j);
        }

        template <typename T>
        inline bool
        operator==(const result_const_iterator<T> &i, const result_const_iterator<T> &j) {
            return i.equal(j);
        }
    }
}

#endif	/* RESULT_ITERATOR_HPP */

