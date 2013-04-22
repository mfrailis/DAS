#ifndef DASVECTOR_HPP_INCLUDED
#define DASVECTOR_HPP_INCLUDED
#include "DasObject.hpp"
#include <vector>

template <typename T>
class DasVector;

template <typename T>
void swap (DasVector<T> &x, DasVector<T> &y);


template <typename T>
class DasVector
{
public:
    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;
    typedef typename std::vector<T>::reverse_iterator reverse_iterator;
    typedef typename std::vector<T>::const_reverse_iterator const_reverse_iterator;
    typedef typename std::vector<T>::size_type size_type;
    typedef typename std::vector<T>::reference reference;
    typedef typename std::vector<T>::const_reference const_reference;
    typedef typename std::vector<T>::value_type value_type;

    DasVector(DasObject &obj, std::vector<T> &vec)
        : vec_(vec), obj_(obj){}

    iterator
    begin(){return vec_.begin();}

    const_iterator
    begin() const {return vec_.begin();}

    iterator
    end(){return vec_.end();}

    const_iterator
    end() const {return vec_.end();}

    reverse_iterator
    rbegin(){return vec_.rbegin();}

    const_reverse_iterator
    rbegin() const{return vec_.rbegin();}

    reverse_iterator
    rend() {return vec_.rend();}

    const_reverse_iterator
    rend() const {return vec_.rend();}

    size_type
    size() const {return vec_.size();}

    bool
    empty() const {return vec_.empty();}

    reference
    operator[] (size_type n) {return vec_[n];}

    const_reference
    operator[] (size_type n) const {return vec_[n];}

    reference
    at (size_type n) {return vec_.at(n);}

    const_reference
    at (size_type n) const {return vec_.at(n);}

    reference
    front() {return vec_.front();}

    const_reference
    front() const {return vec_.front();}

    reference
    back() {return vec_.back();}

    const_reference
    back() const {return vec_.back();}

    template <class InputIterator>
    void
    assign (InputIterator first, InputIterator last)
    {
        obj_.is_dirty_=true;
        vec_.assign<InputIterator>(first,last);
    }

    void
    assign (size_type n, const value_type& val)
    {
        obj_.is_dirty_=true;
        vec_.assign(n,val);
    }

    void
    push_back (const value_type& val)
    {
        obj_.is_dirty_=true;
        vec_.push_back(val);
    }

    void
    pop_back()
    {
        obj_.is_dirty_=true;
        vec_.pop_back();
    }

    iterator
    insert (iterator position, const value_type& val)
    {
        obj_.is_dirty_=true;
        vec_.insert(position,val);
    }

    void
    insert (iterator position, size_type n, const value_type& val)
    {
        obj_.is_dirty_=true;
        vec_.insert(position,n,val);
    }

    template <class InputIterator>
    void
    insert (iterator position, InputIterator first, InputIterator last)
    {
        obj_.is_dirty_=true;
        vec_.insert<InputIterator>(position,first,last);
    }

    iterator
    erase (iterator position)
    {
        obj_.is_dirty_=true;
        vec_.erase(position);
    }

    iterator
    erase (iterator first, iterator last)
    {
        obj_.is_dirty_=true;
        vec_.erase(first,last);
    }

    void
    swap (DasVector& x)
    {
        obj_.is_dirty_=true;
        vec_.swap(x.vec_);
    }

    void
    clear()
    {
        obj_.is_dirty_=true;
        vec_.clear();
    }


private:
    friend void ::swap<>(DasVector<T> &x, DasVector<T> &y);
    DasVector(const DasVector<T>&);               //forbidden
    DasVector<T>& operator=(const DasVector<T>&); //forbidden

    std::vector<T> &vec_;
    DasObject &obj_;
};


template <typename T>
void swap (DasVector<T> &x, DasVector<T> &y)
{
    x.obj_.is_dirty_=true;
    y.obj_.is_dirty_=true;
    swap(x.vec_,y.vec_);
}

#endif // DASVECTOR_HPP_INCLUDED
