
%{
#include "tpl/result_iterator.hpp"
#include "tpl/result.hpp"
#include "result_iterator_wrapper.hpp"
%}




%nodefaultctor das::tpl::Result;
%copyctor das::tpl::Result;
%feature("valuewrapper") das::tpl::Result;
namespace das {

  namespace tpl {

    template<typename T>
    class result_iterator;

    template<typename T>
    class Result {
    public:
      typedef result_iterator<T> res_iter;
    };
  }

  struct stop_iteration {};

  template <typename T>
  class result_iterator_wrapper {

  public:
    typedef typename das::tpl::result_iterator<T> res_iter;

    result_iterator_wrapper(res_iter curr, res_iter end);

    const std::tr1::shared_ptr<T>
      next() throw(stop_iteration);


    result_iterator_wrapper<T>&
      __iter__();
    
  };


}


%typemap(throws) das::stop_iteration %{
  (void)$1;
  SWIG_SetErrorObj(PyExc_StopIteration, SWIG_Py_Void());
  SWIG_fail;
  %}


%extend das::tpl::Result {

  
  das::result_iterator_wrapper<T>
  __iter__()
  {
    return das::result_iterator_wrapper<T>(self->begin(), self->end());
  }

  
};
