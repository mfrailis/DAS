#ifndef DAS_COLUMN_HPP
#define DAS_COLUMN_HPP
#include <odb/core.hxx>
#include <vector>
#include <string>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>

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

    void
    type(const std::string &type) {
        type_ = type;
    }

protected:
    long long size_;
    std::string type_;

    Column() {
    }
private:
    friend class odb::access;

};

template<typename T> class DasDataIn;
template<typename T> class DasDataOut;

#pragma db object abstract

class ColumnFile : public Column {
public:

    ColumnFile(const long long &size,
            const std::string &type,
            const std::string &fname)
    : Column(size, type), fname_(fname), id_(0) {
    }

    ColumnFile(const std::string &type)
    : Column(type), id_(0) {
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

protected:

    ColumnFile() {
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

    void save();
    std::string fname_;
};

#pragma db object no_id

class ColumnBlob : public Column {
public:

    ColumnBlob(const long long &size,
            const std::string &type)
    : Column(size, type) {
    }

    ColumnBlob(const std::string &type)
    : Column(type) {
    }



private:
    friend class odb::access;

    ColumnBlob() {
    }

#pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
    std::vector<char> buffer_;

};
#endif
