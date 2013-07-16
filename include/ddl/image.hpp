#ifndef DAS_IMAGE_HPP
#define DAS_IMAGE_HPP

#pragma db value
class Image
{

 public:
  Image(unsigned int size1, unsigned int size2, std::string pixel_type)
    : size1_(size1), size2_(size2), pixel_type_(pixel_type)
  {}

  Image(std::string pixel_type)
    : size1_(0), size2_(0), pixel_type_(pixel_type)
  {}

  const std::string&
  pixel_type() const
  {
    return pixel_type_;
  }

  void
  pixel_type(std::string type)
  {
    pixel_type_ = type;
  }

  unsigned long
  size1()
  {
    return size1_;
  }

  void
  size1(unsigned int size)
  {
    size1_ = size;
  }

  unsigned int
  size2()
  {
    return size2_;
  }

  void
  size2(unsigned int size)
  {
    size2_ = size;
  }

 protected:
  unsigned int size1_;
  unsigned int size2_;
  std::string pixel_type_;
  Image() {}
 private:
  friend class odb::access;
};

#pragma db value
class ImageFile: public Image
{
public:
  ImageFile(unsigned int size1, unsigned int size2, std::string pixel_type, std::string fname)
    : Image(size1, size2, pixel_type), fname_(fname)
  {}
  ImageFile(std::string type)
    : Image(type)
  {}
  const std::string&
  fname()
  {
    return fname_;
  }

  void
  fname(std::string fname)
  {
    fname_ = fname;
  }

 private:
  friend class odb::access;
  void save();
  ImageFile()  {}
  std::string fname_;
};

#pragma db value
class ImageBlob: public Image
{
public:
  ImageBlob(unsigned int size1, unsigned int size2, std::string pixel_type, std::string fname)
    : Image(size1, size2, pixel_type)
  {}
  ImageBlob(std::string type)
    : Image(type)
  {}

  //TODO methods for streaming in and out the buffer

 private:
  friend class odb::access;

  ImageBlob()  {}

  #pragma db mysql:type("MEDIUMBLOB") oracle:type("BLOB") pgsql:type("BYTEA") sqlite:type("BLOB") mssql:type("varbinary")
  std::vector<char> buffer_;
  //FIXME is this vector<char> suitable?
};
#endif
