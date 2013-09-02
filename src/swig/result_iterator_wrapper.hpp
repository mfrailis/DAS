

#ifndef RESULT_ITERATOR_WRAPPER_H
#define RESULT_ITERATOR_WRAPPER_H

#include "internal/result_iterator.hpp"

namespace das {

  struct stop_iteration {};

  template <typename T>
  class result_iterator_wrapper {

  public:
    typedef typename das::tpl::result_iterator<T> res_iter;
    
    result_iterator_wrapper(res_iter curr, res_iter end)
      :curr_(curr), end_(end) 
    {
    }

    const std::tr1::shared_ptr<T>
    next()
    {
      if (curr_ != end_)
        {
          std::tr1::shared_ptr<T> val = curr_.load();
          ++curr_;
          return val;
        }
      throw stop_iteration();
    }


    result_iterator_wrapper<T>&
    __iter__()
    {
      return *this;
    }
    

  private:
    
    res_iter curr_;
    res_iter end_;
    
  };


}
#endif
