
%{
#include "ddl/info.hpp"
%}

%include "std_string.i"

struct KeywordInfo {

    KeywordInfo(const std::string& _name,
                const std::string& _type,
                const std::string& _unit,
                const std::string& _description);

    std::string name;
    std::string type;
    std::string unit;
    std::string description;
private:
    KeywordInfo();
};

struct ImageInfo {

  ImageInfo(const std::string& _type, size_t _dimensions);

  std::string type;
  size_t dimensions;

private:
    // we do not allow default constructor because the type_var_ member unassigned brings undefined behaviour in boost visits
    ImageInfo();
};

struct ColumnInfo {

    ColumnInfo(const std::string& _name,
               const std::string& _type,
               const std::string& _unit,
               const std::string& _description,
               size_t _max_sting_length);

    std::string name;
    std::string type;
    std::string unit;
    std::string description;
    size_t max_string_length;

private:
    // we do not allow default constructor because the type_var_ member unassigned brings undefined behaviour in boost visits
    ColumnInfo();
};

struct AssociationInfo {

    std::string association_type;
    std::string association_table;
    std::string association_key;
    std::string object_key;

private:
    AssociationInfo();
};

class DdlInfo {
public:

    virtual
    const KeywordInfo&
    get_keyword_info(const std::string &type_name, const std::string &keyword_name)
      const throw (std::out_of_range);

    virtual
    const ColumnInfo&
    get_column_info(const std::string &type_name, const std::string &column_name)
      const throw (std::out_of_range);
    
    virtual
    const ImageInfo&
    get_image_info(const std::string &type_name)
      const throw (std::out_of_range);


    static DdlInfo*
    get_instance();
    

    static DdlInfo*
    get_instance(const std::string &ddl_name)
      throw (std::out_of_range);

    virtual
    const AssociationInfo&
    get_association_info(const std::string &type_name, const std::string &association_name)
      const throw (std::out_of_range);

protected:

    DdlInfo();


};

