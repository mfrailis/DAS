#ifndef DAS_IMAGE_HPP
#define DAS_IMAGE_HPP
#include <odb/core.hxx>
#include <vector>
#include <string>

#pragma db value
class Image
{

 public:
  Image(const unsigned int &size1,const unsigned int &size2,const std::string &pixel_type)
    : size1_(size1), size2_(size2), pixel_type_(pixel_type)
  {}

  Image(const std::string &pixel_type)
    : size1_(0), size2_(0), pixel_type_(pixel_type)
  {}

  const std::string&
  pixel_type() const
  {
    return pixel_type_;
  }

  void
  pixel_type(const std::string &type)
  {
    pixel_type_ = type;
  }

  unsigned long
  size1()
  {
    return size1_;
  }

  void
  size1(const unsigned int &size)
  {
    size1_ = size;
  }

  unsigned int
  size2()
  {
    return size2_;
  }

  void
  size2(const unsigned int& size)
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
  ImageFile(const unsigned int &size1,
	    const unsigned int &size2,
	    const std::string &pixel_type,
	    const std::string &fname)
    : Image(size1, size2, pixel_type), fname_(fname)
  {}
  ImageFile(const std::string &type)
    : Image(type)
  {}
  const std::string&
  fname()
  {
    return fname_;
  }

  void
  fname(const std::string &fname)
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
  ImageBlob(const unsigned int &size1,
	    const unsigned int &size2,
	    const std::string &pixel_type,
	    const std::string &fname)
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
