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

//#pragma db value abstract

class Column {
public:

    Column(const std::string &type, const std::string &array_size)
    : buff_(type, array_size) {
    }

    virtual
    long long
    size() const = 0;

    virtual
    const std::string&
    get_array_size() const = 0;


    virtual
    const std::string&
    get_type() const = 0;
    
    virtual
    const long long&
    store_size() const = 0;
    
    virtual
    void
    store_size(const long long &size)=0;

    ColumnBuffer&
    buffer() {
        return buff_;
    }
        
protected:

    virtual
    void
    set_type(const std::string &type) = 0;

    virtual
    void
    set_array_size(const std::string &array_size) = 0;

    Column() {
    }


    ColumnBuffer buff_;
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
    : Column(type, array_size), size_(size), type_(type), array_size_(array_size), fname_(fname), id_(0) {
    }

    ColumnFromFile(const std::string &type, const std::string& array_size)
    : Column(type, array_size), size_(0), type_(type), array_size_(array_size), id_(0) {
    }

    const std::string&
    fname() const {
        return fname_;
    }

    const std::string&
    temp_path() const {
        return temp_path_;
    }

    void
    temp_path(const std::string &fname) {
        temp_path_ = fname;
    }



    bool
    is_new() {
        return id_ == 0;
    }

    virtual
    void
    persist(odb::database &db) {
        throw das::abstract_das_object();
    }

    const
    long long&
    id() const {
        return id_;
    }

    virtual
    const std::string&
    get_array_size() const {
        return array_size_;
    }

    virtual
    const std::string&
    get_type() const {
        return type_;
    }

    virtual
    long long
    size() const {
        return size_ + buff_.size();
    }

    virtual
    const long long&
    store_size() const {
        return size_;
    }

    virtual
    void
    store_size(const long long &size) {
        size_ = size;
    }

    const std::string&
    rollback_path() {
        return rollback_path_;
    }

    void
    rollback_path(const std::string& path) {
        rollback_path_ = path;
    }
protected:
    long long size_;
#pragma db get(get_type) set(set_type)
    std::string type_;

#pragma db get(get_array_size) set(set_array_size)    
    std::string array_size_;

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


    // implemented for odb library pourposes

    ColumnFromFile() : id_(0) {
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

#pragma db value abstract

class ColumnFromBlob : public Column {
public:
    typedef std::vector<char> blob_type;
    ColumnFromBlob(const long long &size,
            const std::string &type,
            const std::string &array_size)
    : Column(type, array_size), size_(size), type_(type), array_size_(array_size) {
    }

    ColumnFromBlob(const std::string &type, const std::string& array_size)
    : Column(type, array_size),size_(0), type_(type), array_size_(array_size) {
    }

    ColumnFromBlob() {
    }

    virtual
    const std::string&
    get_array_size() const {
        return array_size_;
    }

    virtual
    const std::string&
    get_type() const {
        return type_;
    }

    virtual
    void
    persist(odb::database &db) {
    }

    virtual
    long long
    size() const {
        return size_ + buff_.size();
    }

    virtual
    const long long&
    store_size() const {
        return size_;
    }

    virtual
    void
    store_size(const long long &size) {
        size_ = size;
    }
    blob_type&
    blob(){
        return buffer_;
    }
    
protected:
    
    long long size_;
#pragma db get(get_type) set(set_type)
    std::string type_;

#pragma db get(get_array_size) set(set_array_size)    
    std::string array_size_;
    
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
    
    friend class odb::access;


#pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
    blob_type buffer_;

};
#endif
