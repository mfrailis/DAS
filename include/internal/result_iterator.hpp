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

            explicit result_iterator(const super& it, const DbBundle &db) : super(it), bundle_(db){}

            long long
            id(){
                return super::id();    
            }
            
            shared_ptr<T>
            load(){ 
                DbBundle db = bundle_.lock(true);
                odb::session::current(*db.session());
                shared_ptr<T> ptr = super::load();
                db.attach(ptr);
                return ptr;
            }
            
            T&
            operator*(){
                load();
                return super::operator*();
            }

            shared_ptr<T>
            operator-> (){
                return load();
            }

            result_iterator&
            operator++() {
                super::operator ++();
                return *this;
            }

            result_iterator
            operator++(int) {
                // All non-end iterators for a result object move together.
                //    
                super::operator ++();
                return *this;
            }

            bool
            equal(result_iterator j) const {
                return super::equal(j);
            }
            
            private:
                result_iterator();
                WeakDbBundle bundle_;
        };
        
        template <typename T>
        inline bool
        operator!= (result_iterator<T> i, result_iterator<T> j)
        {
            return !i.equal (j);
        }
        
        template <typename T>
        inline bool
        operator== (result_iterator<T> i, result_iterator<T> j)
        {
            return i.equal (j);
        }
    }
}

#endif	/* RESULT_ITERATOR_HPP */

