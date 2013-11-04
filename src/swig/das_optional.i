
%{
#include "optional.hpp"
%}

%include "std_string.i"
     

%define %das_opt_typemap(Type)
%typemap(out, fragment=SWIG_From_frag(Type)) das::optional<Type> {
  if($1.is_initialized())
    $result = SWIG_From(Type)(*$1);
  else
  {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}
%enddef

%typemap(out, fragment=SWIG_From_frag(std::string)) das::optional<std::string> {
  if((&$1)->is_initialized())
    $result = SWIG_From(std::string)(*(&$1));
  else
  {
    $result = Py_None;
    Py_INCREF(Py_None);
  }
}


%das_opt_typemap(bool);
%das_opt_typemap(char);
%das_opt_typemap(short);
%das_opt_typemap(int);
%das_opt_typemap(long long);
%das_opt_typemap(float);
%das_opt_typemap(double);


%apply das::optional<int> {das::optional<signed char>};

