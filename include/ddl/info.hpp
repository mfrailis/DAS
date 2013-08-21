#ifndef DAS_INFO_HPP
#define DAS_INFO_HPP
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>
#include <string>

typedef boost::variant<
char,
short,
int,
long long,
float,
double,
bool,
unsigned char,
unsigned short,
unsigned int,
std::string
> column_type;

typedef boost::variant<
char,
short,
int,
float,
double
> image_type;

typedef boost::variant<
char,
short,
int,
long long,
float,
double,
bool,
unsigned char,
unsigned int,
std::string
>
keyword_type;

struct KeywordInfo {

    KeywordInfo(const std::string& _name,
            const std::string& _type,
            const std::string& _unit,
            const std::string& _description)
    : name(_name),
    type(_type),
    unit(_unit),
    description(_description) {
    }
    std::string name;
    std::string type;
    std::string unit;
    std::string description;
private:
    KeywordInfo();
};

struct ImageInfo {

    ImageInfo(const std::string& _type, size_t _dimensions)
    : type(_type), dimensions(_dimensions) {
        if (_type == "byte") {
            type_var_ = static_cast<char> (0);
        } else if (_type == "int16") {
            type_var_ = static_cast<short> (0);
        } else if (_type == "int32") {
            type_var_ = static_cast<int> (0);
        } else if (_type == "float32") {
            type_var_ = static_cast<float> (0);
        } else if (_type == "float64") {
            type_var_ = static_cast<double> (0);
        }
    }

    std::string type;
    size_t dimensions;
    image_type type_var_;
private:
    // we do not allow default constructor because the type_var_ member unassigned brings undefined behaviour in boost visits
    ImageInfo();
};

struct ColumnInfo {

    ColumnInfo(const std::string& _name,
            const std::string& _type,
            const std::string& _unit,
            const std::string& _description,
            size_t _max_sting_length)
    : name(_name),
    type(_type),
    unit(_unit),
    description(_description),
    max_string_length(_max_sting_length) {
        if (_type == "byte" || _type == "char") {
            type_var_ = static_cast<char> (0);
        } else if (_type == "int16") {
            type_var_ = static_cast<short> (0);
        } else if (_type == "int32") {
            type_var_ = static_cast<int> (0);
        } else if (_type == "int64") {
            type_var_ = static_cast<long long> (0);
        } else if (_type == "float32") {
            type_var_ = static_cast<float> (0);
        } else if (_type == "float64") {
            type_var_ = static_cast<double> (0);
        } else if (_type == "boolean") {
            type_var_ = static_cast<bool> (0);
        } else if (_type == "uint8") {
            type_var_ = static_cast<unsigned char> (0);
        } else if (_type == "uint16") {
            type_var_ = static_cast<unsigned short> (0);
        } else if (_type == "uint32") {
            type_var_ = static_cast<unsigned int> (0);
        } else if (_type == "string") {
            type_var_ = std::string();
        }
    }

    std::string name;
    std::string type;
    std::string unit;
    std::string description;
    size_t max_string_length;
    column_type type_var_;
private:
    // we do not allow default constructor because the type_var_ member unassigned brings undefined behaviour in boost visits
    ColumnInfo();
};

struct AssociationInfo {

    AssociationInfo(const std::string& ass_type,
            const std::string& table_name,
            const std::string& ass_key,
            const std::string& obj_key)
    : association_type(ass_type),
    association_table(table_name),
    association_key(ass_key),
    object_key(obj_key) {
    }
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
    const throw (std::out_of_range) {
        return all_keywords_.at(type_name).at(keyword_name);
    }

    virtual
    const ColumnInfo&
    get_column_info(const std::string &type_name, const std::string &column_name)
    const throw (std::out_of_range) {
        return all_columns_.at(type_name).at(column_name);
    }
    
    virtual
    const ImageInfo&
    get_image_info(const std::string &type_name)
    const throw (std::out_of_range) {
        return all_images_.at(type_name);
    }

    static DdlInfo*
    get_instance() {
        if (!instance_) {
            instance_ = new DdlInfo();
            instance_->init();
        }
        return instance_;
    }

    static DdlInfo*
    get_instance(const std::string &ddl_name)
    throw (std::out_of_range) {
        return get_instance()->ddl_map_.at(ddl_name);
    }

    virtual
    const AssociationInfo&
    get_association_type(const std::string &type_name, const std::string &association_name)
    const throw (std::out_of_range) {
        return all_associations_.at(type_name).at(association_name);
    }

protected:
    typedef boost::unordered_map< std::string, AssociationInfo > Association_map;
    typedef boost::unordered_map< std::string, KeywordInfo > Keyword_map;
    typedef boost::unordered_map< std::string, ColumnInfo > Column_map;

    static boost::unordered_map< std::string, Keyword_map > all_keywords_;
    static boost::unordered_map< std::string, Column_map > all_columns_;
    static boost::unordered_map< std::string, ImageInfo > all_images_;
    static boost::unordered_map< std::string, Association_map > all_associations_;

    DdlInfo() {
    }



private:
    void init();
    boost::unordered_map< std::string, DdlInfo* > ddl_map_;
    static DdlInfo* instance_;
};

#endif
