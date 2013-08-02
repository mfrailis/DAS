#ifndef DAS_COLUMN_HPP
#define DAS_COLUMN_HPP
#include <odb/core.hxx>
#include <vector>
#include <string>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>
#include "../internal/column_buffer.hpp"

#pragma db object abstract
class Column {
public:

    Column(const long long& size,
            const std::string &type)
    : size_(size), type_(type) {
    }

    Column(const std::string &type)
    : size_(0), type_(type) {
    }

    const long long&
    size() const {
        return size_;
    }

    void
    size(const long long &size) {
        size_ = size;
    }

    const std::string&
    type() const {
        return type_;
    }

protected:

    virtual
    void
    type(const std::string &type) {
        type_ = type;
    }
    
    long long size_;
#pragma db access(type)
    std::string type_;

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
            const std::string &fname)
    : Column(size, type), fname_(fname), id_(0), buff_(type) {
    }

    ColumnFromFile(const std::string &type)
    : Column(type), id_(0), buff_(type) {
    }
    
    const std::string&
    fname() {
        return fname_;
    }

    void
    fname(const std::string &fname) {
        fname_ = fname;
    }

    const std::string&
    temp_path() {
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


protected:
    virtual
    void
    type(const std::string &type) {
        type_ = type;
        buff_.init(type);
    }

#pragma db transient
    ColumnBuffer buff_;

    // implemented for odb library pourposes
    ColumnFromFile() {
    }
private:

    template<typename T>
    friend class DasDataIn;

    template<typename T>
    friend class DasDataOut;

#pragma db id auto
    long long id_;
#pragma db transient
    std::string temp_path_;
    friend class odb::access;

    std::string fname_;
};

#pragma db object no_id
class ColumnFromBlob : public Column {
public:

    ColumnFromBlob(const long long &size,
            const std::string &type)
    : Column(size, type) {
    }

    ColumnFromBlob(const std::string &type)
    : Column(type) {
    }



private:
    friend class odb::access;

    ColumnFromBlob() {
    }

#pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
    std::vector<char> buffer_;

};
#endif
