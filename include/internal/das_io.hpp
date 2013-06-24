#ifndef DAS_IO_HPP
#define	DAS_IO_HPP
#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <blitz/array.h>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>
#include <fstream>
#include "../ddl/column.hpp"
#include "../ddl/info.hpp"
#include "../exceptions.hpp"
using std::tr1::shared_ptr;


void das_io_read(const char* path, size_t offset, size_t count, void *buffer);

template<typename T>
class DasDataIn : public boost::static_visitor< blitz::Array<T, 1> > {
public:

    DasDataIn(shared_ptr<ColumnFile> ptr)
    : col_(ptr) {
    }

    template<typename X>
    void
    read_file(X *buf, const X &native_type) const {
        const char *path = col_->temp_path_ == ""? col_->fname_.c_str() : col_->temp_path_.c_str();
        das_io_read(path, start_ * sizeof (X), length_ * sizeof (X), buf);
    }

    template<typename X>
    void
    read_file(X *buf, const std::string &native_type) const {
        std::cout << "string column I/O not implemented yet" << std::endl;
    }

    template<typename X>
    void
    read_file(std::string *buf, const X &native_type) const {
        std::cout << "string column I/O not implemented yet" << std::endl;
    }

    template<typename X, typename Y>
    void
    read_file(X *buf, const Y &native_type) const {
        const char *path = col_->temp_path_ == ""? col_->fname_.c_str() : col_->temp_path_.c_str();
            
        Y *buffer = new Y[length_];
        try {
            das_io_read(path, start_ * sizeof (Y), length_ * sizeof (Y), buffer);
        }catch (std::exception &e) {
            delete[] buffer;
            throw;
        }

        for (size_t i = 0; i < length_; i++)
            buf[i] = buffer[i]; //casting types

        delete[] buffer;
    }


    //apply_visitor method

    template<typename Y>
    blitz::Array<T, 1>
    operator()(const Y &native_type) const {
        T *buffer = new T[length_];
        try{
            read_file(buffer, native_type);
        }catch(std::exception &e){
            delete[] buffer;
            throw;
        }
        return blitz::Array<T, 1>(buffer, blitz::shape(length_), blitz::deleteDataWhenDone);
    }

    blitz::Array<T, 1>
    operator()(const std::string &string_type) const {
        std::cout << "string data io not implemented yet" << std::cout;
        throw das::bad_type();
    }

    blitz::Array<T, 1>
    get(const ColumnInfo &info, const size_t &start, const size_t &length) {
        start_ = start;
        length_ = length;
        return boost::apply_visitor(*this, info.type_var_);
    }
private:
    size_t start_;
    size_t length_;

    shared_ptr<ColumnFile> col_;
    //    size_t write(const char *path, size_t offset, size_t count, void *buffer);
    //    size_t append(const char *path, size_t count, void *buffer);

};

template<typename T>
class DasDataOut : public boost::static_visitor< blitz::Array<T, 1> > {
public:

    DasDataOut(shared_ptr<ColumnFile> ptr)
    : col_(ptr) {
    }

    template<typename X>
    void
    read_file(X *buf, const X &native_type) const {
        std::string &path = col_->temp_path_ == "" ? col_->fname_ : col_->temp_path_;

        std::ifstream is(path.c_str(), std::ifstream::binary);
        if (is) {
            is.seekg(start_ * sizeof (X), is.beg);

            char * buffer = reinterpret_cast<char*> (buf);

            std::cout << "Reading " << length_ * sizeof (X) << " bytes... " << std::endl;
            // read data as a block:
            is.read(buffer, length_ * sizeof (X));

            if (is)
                std::cout << "all characters read successfully." << std::endl;
            else
                std::cout << "error: only " << is.gcount() << " could be read" << std::endl;
            is.close();
        }
    }

    template<typename X>
    void
    read_file(X *buf, const std::string &native_type) const {
        std::cout << "string column I/O not implemented yet" << std::endl;
    }

    template<typename X>
    void
    read_file(std::string *buf, const X &native_type) const {
        std::cout << "string column I/O not implemented yet" << std::endl;
    }

    template<typename X, typename Y>
    void
    read_file(X *buf, const Y &native_type) const {
        std::string &path = col_->temp_path_ == "" ? col_->fname_ : col_->temp_path_;
        std::ifstream is(path.c_str(), std::ifstream::binary);
        if (is) {
            is.seekg(start_ * sizeof (Y), is.beg);

            char * buffer = new char[length_ * sizeof (Y)];

            std::cout << "Reading " << length_ * sizeof (Y) << " bytes... " << std::endl;
            // read data as a block:
            is.read(buffer, length_ * sizeof (Y));

            if (is)
                std::cout << "all characters read successfully." << std::endl;
            else
                std::cout << "error: only " << is.gcount() << " could be read" << std::endl;
            is.close();

            Y *buf_y = reinterpret_cast<Y*> (buffer);
            for (size_t i = 0; i < length_; i++)
                buf[i] = buf_y[i]; //casting types

            delete[] buffer;
        }
    }


    //apply_visitor method

    template<typename Y>
    blitz::Array<T, 1>
    operator()(const Y &native_type) const {
        T *buffer = new T[length_];
        read_file(buffer, native_type);
        return blitz::Array<T, 1>(buffer, blitz::shape(length_), blitz::deleteDataWhenDone);
    }

    blitz::Array<T, 1>
    operator()(const std::string &string_type) const {
        std::cout << "string data io not implemented" << std::cout;
        throw das::bad_type();
    }

    blitz::Array<T, 1>
    get(const ColumnInfo &info, const long long &start, const long long &length) {
        start_ = start;
        length_ = length;
        return boost::apply_visitor(*this, info.type_var_);
    }
private:
    //    long long size_;
    long long start_;
    long long length_;
    //    std::string path_;
    shared_ptr<ColumnFile> col_;
    size_t write(const char *path, size_t offset, size_t count, void *buffer);
    size_t append(const char *path, size_t count, void *buffer);
};

#endif	/* DAS_IO_HPP */

