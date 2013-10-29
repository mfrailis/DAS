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
    : pixel_type_(pixel_type), fname_(fname), id_(0),
    buff_(pixel_type, this) {
    }

    ImageFromFile(const std::string &pixel_type)
    : pixel_type_(pixel_type), id_(0),
    buff_(pixel_type, this) {
    }

    ImageFromFile(const ImageFromFile &rhs)
    :
    id_(0),
    pixel_type_(rhs.pixel_type_),
    buff_(rhs.pixel_type_, this),
    fname_(rhs.fname_),
    temp_path_(rhs.temp_path_) {
    }

    const std::string&
    pixel_type() const {
        return pixel_type_;
    }

    virtual
    unsigned int
    extent(size_t rank) const {
        return 0;
    }

    virtual
    unsigned int
    file_tiles() const {
        return 0;
    }
    
    virtual
    void
    file_tiles(const unsigned int& tiles){
    }
    
    virtual
    void
    extent(const size_t &rank, size_t value) {
    }

    virtual
    unsigned int
    rank() const {
        return 0;
    }

    virtual
    unsigned int
    num_elements() const {
        return 0;
    }

    const std::string&
    fname() const{
        return fname_;
    }
    
    void
    fname(const std::string &fname){
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

    ImageBuffer&
    buffer() {
        return buff_;
    }

    const long long&
    id() const{
        return id_;
    }
    
    virtual
    void
    persist(odb::database &db){
        throw das::abstract_das_object();
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


#pragma db access(pixel_type)
    std::string pixel_type_;
    std::string fname_;

#pragma db transient
    ImageBuffer buff_;

    // implemented for odb library pourposes

    ImageFromFile() : id_(0),
    buff_(this) {
    }

    void
    pixel_type(const std::string &pixel_type) {
        pixel_type_ = pixel_type;
        buff_.init(pixel_type);
    }
private:

    friend class odb::access;
    friend class ImageBuffer;
#pragma db id auto
    long long id_;
#pragma db transient
    std::string temp_path_;  
#pragma db transient
    std::string rollback_path_;    

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
