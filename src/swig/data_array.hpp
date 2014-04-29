#ifndef DATA_ARRAY_H
#define DATA_ARRAY_H

#include "Python.h"

#include <string>
#include <boost/unordered_map.hpp>
#include "numpy/arrayobject.h"
#include "internal/array.hpp"
#include "das_object.hpp"

namespace das {

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

    struct das_object_extent_fun_ptr {
        PyObject * (*get_image)(DasObject *obj,
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

        void (*append_tiles)(DasObject*, PyObject*, bool);

        void (*append_column_array)(DasObject *obj, const std::string &col_name,
                PyObject* py_array);

        PyObject * (*get_column_array)(DasObject *obj, const std::string &col_name,
                size_t start, ssize_t length);

    };

    struct das_object_func_ptr {
        typedef boost::unordered_map<int, das_object_extent_fun_ptr> ext_ptr_map;
        PyObject * (*get_column)(DasObject *obj, const std::string &col_name,
                size_t start, ssize_t length);

        void (*append_column)(DasObject *obj, const std::string &col_name,
                PyObject* py_array);

        void append_tiles(DasObject *obj, PyObject* py_array) {
            append_tiles_impl(extent_map, obj, py_array, false);
        }

        void set_image(DasObject *obj, PyObject* py_array) {
            append_tiles_impl(extent_map, obj, py_array, true);
        }



        ext_ptr_map extent_map;
    protected:
        void (*append_tiles_impl)(ext_ptr_map&, DasObject*, PyObject*, bool);
        PyObject * (*get_image_impl)(
                ext_ptr_map&, DasObject*, das::Range, das::Range, das::Range,
                das::Range, das::Range, das::Range, das::Range, das::Range,
                das::Range, das::Range, das::Range);
    };

static boost::unordered_map<std::string, das_object_func_ptr> map_das_object_methods;
void initialize_das();
}


#endif
