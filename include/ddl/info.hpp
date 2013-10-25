#ifndef DAS_INFO_HPP
#define DAS_INFO_HPP
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>
#include <string>
#include <vector>
#include <exception>
#include <typeinfo>
#include <odb/tr1/memory.hxx>
#include "../exceptions.hpp"

using std::tr1::shared_ptr;

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
signed char,
char,
short,
int,
long long,
float,
double,
bool,
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
            const std::string& _array_size,           
            const std::string& _description,
            size_t _max_sting_length)
    : name(_name),
    type(_type),
    unit(_unit),
    description(_description),
    array_size(_array_size),
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
    std::string array_size;
    size_t max_string_length;
    column_type type_var_;
    
    static std::vector<int>
    array_extent(const std::string &array_size);
    
private:
    // we do not allow default constructor because the type_var_ member unassigned brings undefined behaviour in boost visits
    ColumnInfo();
};

class bad_multiplicity : public std::exception {
public:

    virtual const char*
    what() const throw () {
        return "wrong association multiplicity";
    }
};

class DasObject;

class AssociationAccess {
public:

    virtual shared_ptr<DasObject> get_one(DasObject* obj) {
        throw bad_multiplicity();
    }

    virtual void set_one(DasObject* obj, const shared_ptr<DasObject> &assoc) {
        throw bad_multiplicity();
    }

    virtual std::vector<shared_ptr<DasObject> > get_many(DasObject* obj) {
        throw bad_multiplicity();
    }

    virtual void set_many(DasObject* obj, const std::vector<shared_ptr<DasObject> > &assoc) {
        throw bad_multiplicity();
    }

    virtual ~AssociationAccess() {
    }
};

template<class Das_type, class Assoc_type>
class AssociationAccessImp_one : public AssociationAccess {
    typedef void (Das_type::*set_method_ptr)(shared_ptr<Assoc_type>&);
    typedef shared_ptr<Assoc_type> (Das_type::*get_method_ptr)();
public:

    AssociationAccessImp_one(get_method_ptr getter, set_method_ptr setter)
    : get_method(getter), set_method(setter) {
    }

    virtual shared_ptr<DasObject> get_one(DasObject* obj) {
        return (dynamic_cast<Das_type*> (obj)->*get_method)();
    }

    virtual void set_one(DasObject* obj, const shared_ptr<DasObject> &assoc) {
        shared_ptr<Assoc_type> a = std::tr1::dynamic_pointer_cast<Assoc_type>(assoc);
        (dynamic_cast<Das_type*> (obj)->*set_method)(a);
    }

private:
    get_method_ptr get_method;
    set_method_ptr set_method;
};

template<class Das_type, class Assoc_type>
class AssociationAccessImp_many : public AssociationAccess {
    typedef void (Das_type::*set_method_ptr)(std::vector<shared_ptr<Assoc_type> >&);
    typedef std::vector<shared_ptr<Assoc_type> >(Das_type::*get_method_ptr)();
    typedef std::vector<shared_ptr<Assoc_type> > ass_vec;
    typedef std::vector<shared_ptr<DasObject > > das_vec;
public:

    AssociationAccessImp_many(get_method_ptr getter, set_method_ptr setter)
    : get_method(getter), set_method(setter) {
    }

    virtual std::vector<shared_ptr<DasObject> > get_many(DasObject* obj) {
        ass_vec av = (dynamic_cast<Das_type*> (obj)->*get_method)();
        das_vec dv;
        for (typename ass_vec::iterator it = av.begin(); it != av.end(); ++it)
            dv.push_back(*it);
        return dv;
    }

    virtual void set_many(DasObject* obj, const std::vector<shared_ptr<DasObject> >&dv) {
        ass_vec av;
        for (das_vec::const_iterator it = dv.begin(); it != dv.end(); ++it)
            av.push_back(std::tr1::dynamic_pointer_cast<Assoc_type>(*it));
        (dynamic_cast<Das_type*> (obj)->*set_method)(av);
    }

private:
    get_method_ptr get_method;
    set_method_ptr set_method;
};

class AssociationInfo {
public:

    AssociationInfo(const std::string& ass_type,
            const std::string& table_name,
            const std::string& ass_key,
            const std::string& obj_key,
            AssociationAccess* acc)
    : association_type(ass_type),
    association_table(table_name),
    association_key(ass_key),
    object_key(obj_key),
    access(acc) {
    }
    std::string association_type;
    std::string association_table;
    std::string association_key;
    std::string object_key;
    shared_ptr<AssociationAccess> access;

private:
    AssociationInfo();
};

class TypeCtor {
public:

    virtual ~TypeCtor() {
    }
    virtual shared_ptr<DasObject> create(const std::string& name, const std::string &db_alias) = 0;
};

template<class Das_type>
class TypeCtorImp : public TypeCtor {
public:

    virtual shared_ptr<DasObject> create(const std::string& name, const std::string &db_alias) {
        return Das_type::create(name, db_alias);
    }
};

class TypeInfo {
public:

    shared_ptr<DasObject>
    operator() (const std::string& name, const std::string &db_alias) const {
        return ctor_->create(name, db_alias);
    }

    const KeywordInfo&
    get_keyword_info(const std::string &keyword_name)
    const throw (std::out_of_range) {
        return keywords_.at(keyword_name);
    }

    const ColumnInfo&
    get_column_info(const std::string &column_name)
    const throw (std::out_of_range) {
        return columns_.at(column_name);
    }

    const ImageInfo&
    get_image_info()
    const throw (std::out_of_range) {
        if (image_)
            return *image_;
        else
            throw std::out_of_range("this type does not provide image data");
    }

    const AssociationInfo&
    get_association_info(const std::string &association_name)
    const throw (std::out_of_range) {
        return associations_.at(association_name);
    }
    
    
    const std::type_info*
    get_type() const{
        return type_;
    }
    
private:
    friend class DdlInfo;
    shared_ptr<TypeCtor> ctor_;
    shared_ptr<ImageInfo> image_;
    boost::unordered_map< std::string, AssociationInfo > associations_;
    boost::unordered_map< std::string, KeywordInfo > keywords_;
    boost::unordered_map< std::string, ColumnInfo > columns_;
    const std::type_info* type_;
};

class DdlInfo {
public:

    virtual
    const KeywordInfo&
    get_keyword_info(const std::string &type_name, const std::string &keyword_name)
    const throw (std::out_of_range) {
        return all_types_.at(type_name).get_keyword_info(keyword_name);
    }

    virtual
    const ColumnInfo&
    get_column_info(const std::string &type_name, const std::string &column_name)
    const throw (std::out_of_range) {
        return all_types_.at(type_name).get_column_info(column_name);
    }

    virtual
    const ImageInfo&
    get_image_info(const std::string &type_name)
    const throw (std::out_of_range) {
        return all_types_.at(type_name).get_image_info();
    }

    virtual
    const AssociationInfo&
    get_association_info(const std::string &type_name, const std::string &association_name)
    const throw (std::out_of_range) {
        return all_types_.at(type_name).get_association_info(association_name);
    }

    virtual
    const TypeInfo&
    get_type_info(const std::string &type_name)
    const throw (std::out_of_range) {
        return all_types_.at(type_name);
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


protected:
    static boost::unordered_map< std::string, TypeInfo > all_types_;

    DdlInfo() {
    };



private:
    void init();
    boost::unordered_map< std::string, DdlInfo* > ddl_map_;
    static DdlInfo* instance_;
};

#endif
