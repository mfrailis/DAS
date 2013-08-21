#ifndef DAS_IMAGE_HPP
#define DAS_IMAGE_HPP
#include <odb/core.hxx>
#include <vector>
#include <string>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>
#include "../internal/image_buffer.hpp"
#include "../exceptions.hpp"
#include "../internal/array.hpp"

#pragma db object abstract

class ImageFromFile {
public:

    ImageFromFile(const std::string &pixel_type,
            const std::string &fname)
    : pixel_type_(pixel_type), fname_(fname), size0_(0), size1_(1), size2_(1),
    size3_(1), size4_(1), size5_(1), size6_(1), size7_(1), size8_(1),
    size9_(1), size10_(1), id_(0),
    buff_(pixel_type, size1_, size2_, size3_, size4_, size5_, size6_, size7_, size8_, size9_, size10_) {
    }

    ImageFromFile(const std::string &pixel_type)
    : pixel_type_(pixel_type), id_(0),
    size0_(0), size1_(1), size2_(1), size3_(1), size4_(1), size5_(1),
    size6_(1), size7_(1), size8_(1), size9_(1), size10_(1),
    buff_(pixel_type, size1_, size2_, size3_, size4_, size5_, size6_, size7_, size8_, size9_, size10_) {
    }

    const std::string&
    pixel_type() const {
        return pixel_type_;
    }

    unsigned int
    extent(size_t rank) {
        switch (rank) {
            case 0: return size0_ + buff_.extent(0);
            case 1: return size1_;
            case 2: return size2_;
            case 3: return size3_;
            case 4: return size4_;
            case 5: return size5_;
            case 6: return size6_;
            case 7: return size7_;
            case 8: return size8_;
            case 9: return size9_;
            case 10: return size10_;
            default:
                return 1;
        }
    }

    unsigned int
    num_elements() const {
        return (size0_ * size1_ * size2_ * size3_ * size4_ *
                size5_ * size6_ * size7_ * size8_ * size9_ * size10_) +
                buff_.num_elements();
    }

    const std::string&
    fname() {
        return fname_;
    }
    
    const std::string&
    temp_path() {
        return temp_path_;
    }

    void
    temp_path(const std::string &fname) {
        temp_path_ = fname;
    }
    
    ImageBuffer&
    buffer() {
        return buff_;
    }
    
protected:
    unsigned int size0_;
    unsigned int size1_;
    unsigned int size2_;
    unsigned int size3_;
    unsigned int size4_;
    unsigned int size5_;
    unsigned int size6_;
    unsigned int size7_;
    unsigned int size8_;
    unsigned int size9_;
    unsigned int size10_;

#pragma db access(pixel_type)
    std::string pixel_type_;
    std::string fname_;

#pragma db transient
    ImageBuffer buff_;

    // implemented for odb library pourposes

    ImageFromFile() : id_(0),
    buff_(size1_, size2_, size3_, size4_, size5_, size6_, size7_, size8_, size9_, size10_) {
    }

    void
    pixel_type(const std::string &pixel_type) {
        pixel_type_ = pixel_type;
        buff_.init(pixel_type);
    }
private:

    friend class odb::access;
#pragma db id auto
    long long id_;
#pragma db transient
    std::string temp_path_;

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
