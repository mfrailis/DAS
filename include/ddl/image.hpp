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

class Image {
public:


    Image(const std::string &pixel_type)
    : buff_(pixel_type, this) {
    }

    virtual
    const std::string&
    pixel_type() const = 0;

    virtual
    unsigned int
    extent(size_t rank) const = 0;

    virtual
    unsigned int
    file_tiles() const  = 0;
    
    virtual
    void
    file_tiles(const unsigned int& tiles) = 0;
    
    virtual
    void
    extent(const size_t &rank, size_t value)  = 0;

    virtual
    unsigned int
    rank() const  = 0;

    virtual
    unsigned int
    num_elements() const  = 0;

    virtual
    void
    reset(const Image *i) = 0;
    
    ImageBuffer&
    buffer() {
        return buff_;
    }
    
    virtual
    void
    persist(odb::database &db) = 0;
        
protected:

    ImageBuffer buff_;

    // implemented for odb library pourposes

    Image() :
    buff_(this) {
    }

private:

    friend class odb::access;
    friend class ImageBuffer;
};

#pragma db object abstract
class ImageFile : public Image {
public:

    ImageFile(const std::string &pixel_type,
            const std::string &fname)
    : Image(pixel_type), pixel_type_(pixel_type), fname_(fname), id_(0)
     {
    }

    ImageFile(const std::string &pixel_type)
    : Image(pixel_type), pixel_type_(pixel_type), id_(0)
    {
    }

    ImageFile(const ImageFile &rhs)
    :
    Image(rhs.pixel_type_),
    id_(0),
    pixel_type_(rhs.pixel_type_),
    fname_(rhs.fname_),
    temp_path_(rhs.temp_path_) {
    }

    virtual
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
    
    virtual
    void
    reset(const Image *i){
        const ImageFile * img = dynamic_cast<const ImageFile*>(i);
        fname(img->fname());
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

    // implemented for odb library pourposes

    ImageFile() : id_(0){
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

#pragma db value abstract

class ImageBlob : public Image{
public:
    typedef std::vector<char> blob_type;

    ImageBlob(const std::string &pixel_type)
    : Image(pixel_type), pixel_type_(pixel_type)
     {
    }


    ImageBlob(const ImageBlob &rhs)
    :
    Image(rhs.pixel_type_),
    pixel_type_(rhs.pixel_type_){
    }

    virtual
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
    
    virtual
    void
    reset(const Image *i){
    }
    

    virtual
    void
    persist(odb::database &db){
        throw das::abstract_das_object();
    }
    
    // implemented for odb library pourposes
    ImageBlob(){
    }
    
    blob_type&
    store(){
        return buffer_;
    }
    
    
    const
    blob_type&
    store() const{
        return buffer_;
    }
protected:


#pragma db access(pixel_type)
    std::string pixel_type_;



    void
    pixel_type(const std::string &pixel_type) {
        pixel_type_ = pixel_type;
        buff_.init(pixel_type);
    }
    
#pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
    blob_type buffer_;
private:
    friend class odb::access;
    friend class ImageBuffer;
};
#endif
