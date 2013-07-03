#ifndef RESULT_ITERATOR_HPP
#define	RESULT_ITERATOR_HPP
#include <odb/object-result.hxx>
#include "db_bundle.ipp"

namespace das {
    namespace tpl {

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

            long long
            id() {
                return super::id();
            }

            const shared_ptr<T>&
            load() {
                if (! *loaded_) {
                    DbBundle db = bundle_.lock(true);
                    odb::session::current(*db.session());
                    *entry_ = super::load();
                    db.attach(*entry_);
                    *loaded_ = true;
                }
                return *entry_;
            }

            T&
            operator*() {
                load();
                return super::operator*();
            }

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
        operator!=(result_iterator<T> i, result_iterator<T> j) {
            return !i.equal(j);
        }

        template <typename T>
        inline bool
        operator==(result_iterator<T> i, result_iterator<T> j) {
            return i.equal(j);
        }

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
        operator!=(result_const_iterator<T> i, result_const_iterator<T> j) {
            return !i.equal(j);
        }

        template <typename T>
        inline bool
        operator==(result_const_iterator<T> i, result_const_iterator<T> j) {
            return i.equal(j);
        }
    }
}

#endif	/* RESULT_ITERATOR_HPP */

