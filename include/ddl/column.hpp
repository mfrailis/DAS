#ifndef DAS_COLUMN_HPP
#define DAS_COLUMN_HPP
#include <odb/core.hxx>
#include <odb/database.hxx>
#include <vector>
#include <string>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>
#include "../internal/column_buffer.hpp"
#include "../exceptions.hpp"

#pragma db object abstract
class Column {
public:

    Column(const long long& size,
            const std::string &type,
            const std::string &array_size)
    : size_(size), type_(type), array_size_(array_size) {
    }

    Column(const std::string &type, const std::string &array_size)
    : size_(0), type_(type), array_size_(array_size) {
    }

    virtual
    long long
    size() const {
        return size_;
    }
    
    const std::string&
    get_array_size() const{
        return array_size_;
    }
    
    const std::string&
    get_type() const {
        return type_;
    }

protected:
        
    virtual
    void
    set_type(const std::string &type) {
        type_ = type;
    }
    
    virtual
    void
    set_array_size(const std::string &array_size) {
        array_size_ = array_size;
    }
    
    long long size_;
#pragma db get(get_type) set(set_type)
    std::string type_;
    
#pragma db get(get_array_size) set(set_array_size)    
    std::string array_size_;

    Column() {
    }
private:
    friend class odb::access;

};

#pragma db object abstract
class ColumnFromFile : public Column {
public:

    ColumnFromFile(const long long &size,
            const std::string &type,
            const std::string &array_size,
            const std::string &fname)
    : Column(size, type,array_size), fname_(fname), id_(0), buff_(type,array_size) {
    }

    ColumnFromFile(const std::string &type, const std::string& array_size)
    : Column(type,array_size), id_(0), buff_(type,array_size) {
    }
    
    const std::string&
    fname() const{
        return fname_;
    }
    const std::string&
    temp_path() const{
        return temp_path_;
    }

    void
    temp_path(const std::string &fname) {
        temp_path_ = fname;
    }

    ColumnBuffer&
    buffer() {
        return buff_;
    }
    
    bool
    is_new(){return id_ == 0;}
    
    virtual
    void
    persist(odb::database &db){
        throw das::abstract_das_object();
    }

    const
    long long&
    id() const{
        return id_;
    }
    
    virtual
    long long
    size() const {
        return size_ + buff_.size();
    }
    
    const long long&
    file_size() const {
        return size_;
    }


    void
    file_size(const long long &size) {
        size_ = size;
    }
    
    const std::string&
    rollback_path(){
        return rollback_path_;
    }
    
    void
    rollback_path(const std::string& path){
       rollback_path_ = path; 
    }
protected:
    virtual
    void
    set_type(const std::string &type) {
        type_ = type;
        buff_.init_type(type);
    }
    
    virtual
    void
    set_array_size(const std::string &array_size) {
        array_size_ = array_size;
        buff_.init_shape(array_size);
    }

#pragma db transient
    ColumnBuffer buff_;

    // implemented for odb library pourposes
    ColumnFromFile() : id_(0){
    }
private:

#pragma db id auto
    long long id_;
#pragma db transient
    std::string temp_path_;
#pragma db transient
    std::string rollback_path_;
    
    friend class odb::access;

    std::string fname_;
};

#pragma db object no_id
class ColumnFromBlob : public Column {
public:

    ColumnFromBlob(const long long &size,
            const std::string &type,
            const std::string &array_size)
    : Column(size, type, array_size) {
    }

    ColumnFromBlob(const std::string &type, const std::string& array_size)
    : Column(type,array_size) {
    }



private:
    friend class odb::access;

    ColumnFromBlob() {
    }

#pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
    std::vector<char> buffer_;

};
#endif
