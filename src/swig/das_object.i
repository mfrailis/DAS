
%{
#include "das_object.hpp"
#include "data_array.hpp"
%}

%import "ddl_info.i"

#define SWIG_SHARED_PTR_SUBNAMESPACE tr1
%include <std_shared_ptr.i>
%shared_ptr(DasObject)

%typemap(out) boost::posix_time::ptime {
    boost::gregorian::date date = $1.date();
    boost::posix_time::time_duration td = $1.time_of_day();
  $result = PyDateTime_FromDateAndTime((int)date.year(),
           (int)date.month(),
           (int)date.day(),
            td.hours(),
            td.minutes(),
            td.seconds(),
            get_usecs(td));
}

class DasObject{
public:

  const KeywordInfo&
    get_keyword_info(std::string keyword_name) throw(std::out_of_range);

    const ColumnInfo &
    get_column_info(std::string column_name) throw(std::out_of_range);

    const ImageInfo &
    get_image_info() throw(std::out_of_range);

    bool is_dirty() const;

    bool is_new() const;

    const std::string &
    dbUserId() const;

    const boost::posix_time::ptime &
    creationDate() const;

    const short&
    version() const;

    const std::string &
    name() const;


    //polimorphic interface

    virtual bool is_table();

    virtual bool is_image();

    long long
    get_column_size(const std::string & col_name);

    long long
    get_column_array_size(const std::string & col_name);

    unsigned int
    get_image_extent(int extent);

protected:

    DasObject();

    DasObject(const std::string &name, const std::string & db_alias);

};

%extend DasObject{
    PyObject * get_column(const std::string &col_name, size_t start = 0, size_t length = -1) {
        std::string col_type = self->get_column_info(col_name).type;
        return das::map_das_object_methods.at(col_type).get_column(self, col_name,
        start, length);
    }

    void append_column(const std::string &col_name, PyObject * array) {
        std::string col_type = self->get_column_info(col_name).type;
        das::map_das_object_methods.at(col_type).append_column(self, col_name, array);
    }

    void append_column_array(const std::string &col_name, PyObject * array) {
        const ColumnInfo& info = self->get_column_info(col_name);
        int extent = ColumnInfo::array_extent(info.array_size).size();
        das::map_das_object_methods.at(info.type).extent_map.at(extent).
        append_column_array(self, col_name, array);
    }

    /*PyObject * get_column_array(const std::string &col_name, size_t start = 0, size_t length = -1) {
        const ColumnInfo& info = self->get_column_info(col_name);
        int extent = ColumnInfo::array_extent(info.array_size).size();
        das::map_das_object_methods.at(info.type).extent_map.at(extent).
        get_column_array(self, col_name, start, length);
    }*/

    void append_tiles(PyObject * array) {
        std::string img_type = self->get_image_info().type;
        das::map_das_object_methods.at(img_type).append_tiles(self, array);
    }

    void set_image(PyObject * array) {
        std::string img_type = self->get_image_info().type;
        das::map_das_object_methods.at(img_type).set_image(self, array);
    }

    PyObject* get_image(
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
    das::Range r10 = das::Range::all()
    ) {
        std::string img_type = self->get_image_info().type;
        size_t dim = self->get_image_info().dimensions;
        return das::map_das_object_methods.at(img_type).extent_map.at(dim).
        get_image(self, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    }

}

%init %{
    import_array();
    das::initialize_das();
%}
