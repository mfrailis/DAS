
%{
#include "das_object.hpp"
#include "data_array.hpp"
%}

%import "ddl_info.i"

#define SWIG_SHARED_PTR_SUBNAMESPACE tr1
%include <std_shared_ptr.i>
%shared_ptr(DasObject)


class DasObject {
public:

  const KeywordInfo&
    get_keyword_info(std::string keyword_name) throw (std::out_of_range); 

  const ColumnInfo&
    get_column_info(std::string column_name) throw (std::out_of_range); 

  bool is_dirty() const;

  bool is_new() const; 

  const std::string&
    dbUserId() const; 

  const long long&
    creationDate() const; 

  void
    creationDate(long long &creationDate);
  
  const short&
    version() const; 

  const std::string&
    name() const;


  //polimorphic interface

  virtual bool is_table(); 

  virtual bool is_image(); 

protected:

  DasObject();

  DasObject(const std::string &name, const std::string &db_alias);

};


%extend DasObject {
  PyObject* get_column(const std::string &col_name, size_t start, size_t length)
  {
    std::string col_type = self->get_column_info(col_name).type;
    return das::map_das_object_methods[col_type].get_column(self, col_name, 
                                                            start, length);
  }
  
  void append_column(const std::string &col_name, PyObject* array)
  {
    std::string col_type = self->get_column_info(col_name).type;
    das::map_das_object_methods[col_type].append_column(self, col_name, array);
  }

}

%init %{
  import_array();
  das::initialize_das();
%}
