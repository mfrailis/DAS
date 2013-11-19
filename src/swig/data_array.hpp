

#ifndef DATA_ARRAY_H
#define DATA_ARRAY_H

#include "Python.h"

#include <string>
#include <boost/unordered_map.hpp>
#include <odb/tr1/memory.hxx>

#include "numpy/arrayobject.h"

#include "internal/array.hpp"
#include "das_object.hpp"


using std::tr1::shared_ptr;

namespace das {

  // TODO: map also a string type, probably to NPY_STRING or NPY_OBJECT

  template<typename T>
  class numpy_type_map {
  public:
    static const int typenum;
  };

  template<>
  const int numpy_type_map<char>::typenum = NPY_BYTE;

  template<>
  const int numpy_type_map<short>::typenum = NPY_SHORT;

  template<>
  const int numpy_type_map<int>::typenum = NPY_INT;  

  template<>
  const int numpy_type_map<long long>::typenum = NPY_LONGLONG;

  template<>
  const int numpy_type_map<float>::typenum = NPY_FLOAT;

  template<>
  const int numpy_type_map<double>::typenum = NPY_DOUBLE;

  template<>
  const int numpy_type_map<bool>::typenum = NPY_BOOL;

  template<>
  const int numpy_type_map<unsigned char>::typenum = NPY_UBYTE;

  template<>
  const int numpy_type_map<unsigned short>::typenum = NPY_USHORT;

  template<>
  const int numpy_type_map<unsigned int>::typenum = NPY_UINT;
  
  template<>
  const int numpy_type_map<std::string>::typenum = NPY_OBJECT;


  // TODO: check if a reference to the DAS Array can be
  // substituted directly with a reference to the C++ raw array
  template <typename T>
  void delete_array(PyObject* capsule)
  {
    das::Array<T> *array = static_cast<das::Array<T>* > (PyCapsule_GetPointer(capsule, 0));
    delete array;
  }
  
  
  class NpyDeallocator: public das::Deallocator
  {
  public:
    NpyDeallocator(PyObject* py_obj):
      obj(py_obj)
    {}
      
    void operator() () {Py_DECREF(obj);}
    
  private:
    PyObject *obj;
  };


  template <typename T>
  inline PyObject* convert_to_numpy(das::Array<T>& array)
  {
    das::Array<T>* array_ptr = new das::Array<T>(array);
    npy_intp size = {array.size()};
    PyObject *py_array = PyArray_SimpleNewFromData(1, &size, numpy_type_map<T>::typenum, 
                                                   array.data());
    // The next operation is necessary so that the C++ array is deleted using
    // delete and not free
    ((PyArrayObject*) py_array)->base = PyCapsule_New(array_ptr, 0, delete_array<T>);
    // No need to fix strides
    return py_array;
  }
  
  
  template<>
  inline PyObject* convert_to_numpy(das::Array<std::string>& array)
  {
    npy_intp size = {array.size()};
    PyObject *py_array = PyArray_SimpleNew(1, &size, numpy_type_map<std::string>::typenum);
    for (npy_intp i = 0; i < size; i++)
      PyArray_SETITEM(py_array, 
                      PyArray_GETPTR1(py_array, i),
                      SWIG_From_std_string(array(i)));
      
    return py_array;
  }

  
  template <typename T>
  inline das::Array<T> convert_to_array(PyObject* py_obj)
  {
    das::TinyVector<int,1> shape(0);    
    npy_intp* dimensions = PyArray_DIMS(py_obj);
    
    // TODO: check that dimension fits an integer
    shape[0] = (int)dimensions[0];  
    
    T* data = (T*) PyArray_DATA(py_obj);
    NpyDeallocator *dealloc = new NpyDeallocator(py_obj);
    
    return das::Array<T>(data, shape, das::neverDeleteData, dealloc);
  }
  
  
  template<>
  inline das::Array<std::string> convert_to_array(PyObject* py_obj)
  {
    das::TinyVector<int,1> shape(0);    
    npy_intp* dimensions = PyArray_DIMS(py_obj);
    
    // TODO: check that dimension fits an integer
    shape[0] = (int)dimensions[0];  
    
    das::Array<std::string> array(shape);
    
    for (int i = 0; i < array.size(); i++)
      array[i] = SWIG_Python_str_AsChar(PyArray_GETITEM(py_obj, 
                                                        PyArray_GETPTR1(py_obj, i)));
    
    return array;    
  }

  template <typename T>
  PyObject* get_numpy_column(DasObject *obj, const std::string &col_name, 
                             size_t start, size_t length)
  {
    das::Array<T> column = obj->get_column<T>(col_name, start, length);
    return convert_to_numpy(column);
  }
  
  
  template <typename T>
  void append_numpy_column(DasObject *obj, const std::string &col_name,
                           PyObject* py_array)
  {
    int typenum = numpy_type_map<T>::typenum;
    PyObject* npy_array = PyArray_FromArray((PyArrayObject*) py_array, PyArray_DescrFromType(typenum),
                                            NPY_CARRAY);
    das::Array<T> array = convert_to_array<T>(npy_array);
    obj->append_column(col_name, array);
  }


  struct das_object_func_ptr {
    
    PyObject* (*get_column)(DasObject *obj, const std::string &col_name, 
                            size_t start, size_t length);
    
    void (*append_column)(DasObject *obj, const std::string &col_name,
                           PyObject* py_array);
    
  };

  template <typename T>
  struct das_object_func_ptr_T: public das_object_func_ptr
  {
    das_object_func_ptr_T()
    {      
      get_column = get_numpy_column<T>;
      append_column = append_numpy_column<T>;
    }
    
  };


  static boost::unordered_map<std::string, das_object_func_ptr> map_das_object_methods;

  static void initialize_das()
  {
    map_das_object_methods["byte"] = das_object_func_ptr_T<char>();
    map_das_object_methods["char"] = das_object_func_ptr_T<char>();
    map_das_object_methods["int16"] = das_object_func_ptr_T<short>();
    map_das_object_methods["int32"] = das_object_func_ptr_T<int>();
    map_das_object_methods["int64"] = das_object_func_ptr_T<long long>();
    map_das_object_methods["float32"] = das_object_func_ptr_T<float>();
    map_das_object_methods["float64"] = das_object_func_ptr_T<double>();
    map_das_object_methods["boolean"] = das_object_func_ptr_T<bool>();
    map_das_object_methods["uint8"] = das_object_func_ptr_T<unsigned char>();
    map_das_object_methods["uint16"] = das_object_func_ptr_T<unsigned short>();
    map_das_object_methods["uint32"] = das_object_func_ptr_T<unsigned int>();
    map_das_object_methods["string"] = das_object_func_ptr_T<std::string>();
  }


}


#endif
