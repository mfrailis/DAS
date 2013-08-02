#ifndef DAS_IMAGE_HPP
#define DAS_IMAGE_HPP
#include <odb/core.hxx>
#include <vector>
#include <string>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>

#pragma db object abstract

class ImageFromFile {
public:

    ImageFromFile(const unsigned int &size1,
            const unsigned int &size2,
            const std::string &pixel_type,
            const std::string &fname)
    : size1_(size1), size2_(size2), pixel_type_(pixel_type), fname_(fname) {
    }

    ImageFromFile(const std::string &pixel_type)
    : size1_(0), size2_(0), pixel_type_(pixel_type) {
    }

    const std::string&
    pixel_type() const {
        return pixel_type_;
    }

    void
    pixel_type(const std::string &type) {
        pixel_type_ = type;
    }

    unsigned long
    size1() {
        return size1_;
    }

    void
    size1(const unsigned int &size) {
        size1_ = size;
    }

    unsigned int
    size2() {
        return size2_;
    }

    void
    size2(const unsigned int& size) {
        size2_ = size;
    }

    const std::string&
    fname() {
        return fname_;
    }

    void
    fname(const std::string &fname) {
        fname_ = fname;
    }


protected:
    unsigned int size1_;
    unsigned int size2_;
    std::string pixel_type_;
    std::string fname_;

    ImageFromFile() {
    }
private:
    friend class odb::access;
#pragma db id auto
    long long id_;


};

#pragma db value

class ImageBlob {
public:

    ImageBlob(const unsigned int &size1,
            const unsigned int &size2,
            const std::string &pixel_type)
    : size1_(size1), size2_(size2), pixel_type_(pixel_type) {
    }

    ImageBlob(const std::string &pixel_type)
    : size1_(0), size2_(0), pixel_type_(pixel_type) {
    }

    //FIXME: this should be private, pixel_type_ needs to be initialized
    ImageBlob() {
    }

    const std::string&
    pixel_type() const {
        return pixel_type_;
    }

    void
    pixel_type(const std::string &type) {
        pixel_type_ = type;
    }

    unsigned long
    size1() {
        return size1_;
    }

    void
    size1(const unsigned int &size) {
        size1_ = size;
    }

    unsigned int
    size2() {
        return size2_;
    }

    void
    size2(const unsigned int& size) {
        size2_ = size;
    }

private:
    friend class odb::access;


    unsigned int size1_;
    unsigned int size2_;
    std::string pixel_type_;

#pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
    std::vector<char> buffer_;
    //FIXME is this vector<char> suitable?
};
#endif
