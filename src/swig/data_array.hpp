

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
    void delete_array(PyObject* capsule) {
        das::Array<T> *array = static_cast<das::Array<T>*> (PyCapsule_GetPointer(capsule, 0));
        delete array;
    }

    class NpyDeallocator : public das::Deallocator {
    public:

        NpyDeallocator(PyObject* py_obj) :
        obj(py_obj) {
        }

        void operator() () {
            Py_DECREF(obj);
        }

    private:
        PyObject *obj;
    };

    template <typename T, int Rank>
    class Convert {
    public:

        inline PyObject* to_numpy(das::Array<T, Rank>& array) const {
            das::Array<T, Rank>* array_ptr = new das::Array<T, Rank>(array);
            npy_intp rank = Rank;
            npy_intp shape[Rank];

            for (size_t i = 0; i < Rank; ++i)
                shape[i] = array.extent(i);

            PyObject *py_array = PyArray_SimpleNewFromData(rank, shape, numpy_type_map<T>::typenum,
                    array.data());
            // The next operation is necessary so that the C++ array is deleted using
            // delete and not free
            ((PyArrayObject*) py_array)->base = PyCapsule_New(array_ptr, 0, delete_array<T>);
            // No need to fix strides
            return py_array;
        }

        inline das::Array<T, Rank> to_array(PyObject* py_obj) const {
            das::TinyVector<int, Rank> shape;
            npy_intp* dimensions = PyArray_DIMS(py_obj);

            for (size_t i = 0; i < Rank; ++i)
                shape[i] = (int) dimensions[i]; // TODO: check that dimension fits an integer

            T* data = (T*) PyArray_DATA(py_obj);
            NpyDeallocator *dealloc = new NpyDeallocator(py_obj);

            return das::Array<T, Rank>(data, shape, das::neverDeleteData, dealloc);
        }
    };

    template <typename T>
    class Convert<T, 1> {
    public:

        inline PyObject* to_numpy(das::Array<T>& array) const {
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

        inline das::Array<T> to_array(PyObject* py_obj) const {
            das::TinyVector<int, 1> shape(0);
            npy_intp* dimensions = PyArray_DIMS(py_obj);

            // TODO: check that dimension fits an integer
            shape[0] = (int) dimensions[0];

            T* data = (T*) PyArray_DATA(py_obj);
            NpyDeallocator *dealloc = new NpyDeallocator(py_obj);

            return das::Array<T>(data, shape, das::neverDeleteData, dealloc);
        }
    };

    template<>
    class Convert<std::string, 1> {
    public:

        inline PyObject* to_numpy(das::Array<std::string>& array) const {
            npy_intp size = {array.size()};
            PyObject *py_array = PyArray_SimpleNew(1, &size, numpy_type_map<std::string>::typenum);
            for (npy_intp i = 0; i < size; i++)
                PyArray_SETITEM(py_array,
                    PyArray_GETPTR1(py_array, i),
                    SWIG_From_std_string(array(i)));

            return py_array;
        }

        inline das::Array<std::string> to_array(PyObject* py_obj) const {
            das::TinyVector<int, 1> shape(0);
            npy_intp* dimensions = PyArray_DIMS(py_obj);

            // TODO: check that dimension fits an integer
            shape[0] = (int) dimensions[0];

            das::Array<std::string> array(shape);

            // TODO: deallocate char array memory after assigning it to the das::array
            // to avoid a memory leak
            for (int i = 0; i < array.size(); i++)
                array(i) = SWIG_Python_str_AsChar(PyArray_GETITEM(py_obj,
                    PyArray_GETPTR1(py_obj, i)));

            return array;
        }
    };

    template <typename T>
    PyObject* get_numpy_column(DasObject *obj, const std::string &col_name,
            size_t start, size_t length) {
        das::Array<T> column = obj->get_column<T>(col_name, start, length);
        return Convert<T, 1>().to_numpy(column);
    }

    template <typename T>
    void append_numpy_column(DasObject *obj, const std::string &col_name,
            PyObject* py_obj) {
        int typenum = numpy_type_map<T>::typenum;
        PyObject* npy_array = PyArray_FromAny(py_obj, PyArray_DescrFromType(typenum),
                1, 1, 0, 0);
        das::Array<T> array = Convert<T, 1>().to_array(npy_array);
        obj->append_column(col_name, array);
    }

    template <typename T, int Rank>
    PyObject* get_numpy_image(DasObject *obj,
            das::Range r0 = das::Range::all(),
            das::Range r1 = das::Range::all(),
            das::Range r2 = das::Range::all(),
            das::Range r3 = das::Range::all(),
            das::Range r4 = das::Range::all(),
            das::Range r5 = das::Range::all(),
            das::Range r6 = das::Range::all(),
            das::Range r7 = das::Range::all(),
            das::Range r8 = das::Range::all(),
            das::Range r9 = das::Range::all(),
            das::Range r10 = das::Range::all()) {
        das::Array<T, Rank> image = obj->get_image<T, Rank>(
                r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
        std::cout << "das::Array" << endl << image << std::endl;
        return Convert<T, Rank>().to_numpy(image);
    }

    template <typename T, int Rank>
    void append_numpy_tiles(DasObject *obj, PyObject* npy_array, bool set) {
        das::Array<T, Rank> array = Convert<T, Rank>().to_array(npy_array);
        if(set)
            obj->set_image(array);
        else
            obj->append_tiles(array);
    }

    struct das_object_image_fun_ptr {
        PyObject* (*get_image)(DasObject *obj,
                das::Range r0,
                das::Range r1,
                das::Range r2,
                das::Range r3,
                das::Range r4,
                das::Range r5,
                das::Range r6,
                das::Range r7,
                das::Range r8,
                das::Range r9,
                das::Range r10);

        void (*append_tiles)(DasObject*, PyObject*,bool);

    };

    template <typename T, int R>
    struct das_object_image_fun_ptr_T_R : public das_object_image_fun_ptr {

        das_object_image_fun_ptr_T_R() {
            get_image = get_numpy_image<T, R>;
            append_tiles = append_numpy_tiles<T, R>;
        }

    };
    
    template<int R>
    struct das_object_image_fun_ptr_T_R<std::string,R> : public das_object_image_fun_ptr {

        das_object_image_fun_ptr_T_R() {
            get_image = get_image_error;
            append_tiles = append_tiles_error;
        }
        
        static PyObject* get_image_error(DasObject *obj,
                das::Range r0,
                das::Range r1,
                das::Range r2,
                das::Range r3,
                das::Range r4,
                das::Range r5,
                das::Range r6,
                das::Range r7,
                das::Range r8,
                das::Range r9,
                das::Range r10){
            throw das::bad_type();
        }

        static void append_tiles_error (DasObject* obj, PyObject* py_array,bool set){
            throw das::bad_type();
        }

    };

    struct das_object_func_ptr {
        typedef boost::unordered_map<int, das_object_image_fun_ptr> ext_ptr_map;
        PyObject* (*get_column)(DasObject *obj, const std::string &col_name,
                size_t start, size_t length);

        void (*append_column)(DasObject *obj, const std::string &col_name,
                PyObject* py_array);

        void append_tiles(DasObject *obj, PyObject* py_array) {
            append_tiles_impl(image_map, obj, py_array,false);
        }
        
        void set_image(DasObject *obj, PyObject* py_array) {
            append_tiles_impl(image_map, obj, py_array,true);
        }

        ext_ptr_map image_map;
    protected:
        void (*append_tiles_impl)(ext_ptr_map&, DasObject*, PyObject*,bool);
        PyObject* (*get_image_impl)(
                ext_ptr_map&, DasObject*, das::Range, das::Range, das::Range,
                das::Range, das::Range, das::Range, das::Range, das::Range,
                das::Range, das::Range, das::Range);
    };

    template <typename T>
    struct das_object_func_ptr_T : public das_object_func_ptr {

        das_object_func_ptr_T() {
            get_column = get_numpy_column<T>;
            append_column = append_numpy_column<T>;
            append_tiles_impl = append_tiles_;

            image_map[1] = das_object_image_fun_ptr_T_R<T, 1>();
            image_map[2] = das_object_image_fun_ptr_T_R<T, 2>();
            image_map[3] = das_object_image_fun_ptr_T_R<T, 3>();
            image_map[4] = das_object_image_fun_ptr_T_R<T, 4>();
            image_map[5] = das_object_image_fun_ptr_T_R<T, 5>();
            image_map[6] = das_object_image_fun_ptr_T_R<T, 6>();
            image_map[7] = das_object_image_fun_ptr_T_R<T, 7>();
            image_map[8] = das_object_image_fun_ptr_T_R<T, 8>();
            image_map[9] = das_object_image_fun_ptr_T_R<T, 9>();
            image_map[10] = das_object_image_fun_ptr_T_R<T, 10>();
            image_map[11] = das_object_image_fun_ptr_T_R<T, 11>();
        }

        static void append_tiles_(
                das_object_func_ptr::ext_ptr_map &map,
                DasObject *obj,
                PyObject* py_obj,
                bool set) {
            int typenum = numpy_type_map<T>::typenum;
            PyObject* npy_array = PyArray_FromAny(py_obj,
                    PyArray_DescrFromType(typenum), 1, 11, 0, 0);
            map.at(PyArray_NDIM(npy_array)).append_tiles(obj, npy_array,set);
        }
    };

    static boost::unordered_map<std::string, das_object_func_ptr> map_das_object_methods;

    static void initialize_das() {
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
