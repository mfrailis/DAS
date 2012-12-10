
#ifndef DDL_INSTANCE_HXX
#define DDL_INSTANCE_HXX

// #if defined(DATABASE_MYSQL) || defined(DATABASE_SQLITE) || defined(DATABASE_PGSQL)
// #define CBLOB "TEXT"
// #elif defined(DATABASE_ORACLE)
// #define CBLOB "CLOB"
// #endif

#include <vector>
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <odb/core.hxx>


// Include TR1 <memory> header in a compiler-specific fashion. Fall back
// on the Boost implementation if the compiler does not support TR1.
//
#include <odb/tr1/memory.hxx>

#include <odb/tr1/lazy-ptr.hxx>

using std::tr1::shared_ptr;

using odb::tr1::lazy_shared_ptr;

#pragma db object abstract
class EssentialMetadata
{
 public:
  EssentialMetadata(std::string name, short version)
    : name_(name), version_(version)
  {}

  const std::string&
  name() const
  {
    return name_;
  }
  
  const short&
  version() const
  {
    return version_;
  }


  const std::string&
  dbUserId() const
  {
    return dbUserId_;
  }


  void dbUserId(std::string dbUserId)
  {
    dbUserId_ = dbUserId;
  }
  

  const boost::posix_time::ptime&
  creationDate() const
  {
    return creationDate_;
  }


  void creationDate(boost::posix_time::ptime creationDate)
  {
    creationDate_ = creationDate;
  }
    

 protected:
  friend class odb::access;
  EssentialMetadata() {}

 private:

  std::string name_;
  short version_;
  std::string dbUserId_;
  boost::posix_time::ptime creationDate_;

};



#pragma db value
class LfiDaeSlowVoltageColumn
{

 public:
  LfiDaeSlowVoltageColumn(long long size, std::string type, std::string fname)
    :size_(size), type_(type), fname_(fname)
  {}

  const long long&
  size() const
  {
    return size_;
  }

  void
  size(long long size)
  {
    size_ = size;
  }

  const std::string&
  type() const
  {
    return type_;
  }
  
  void
  type(std::string type)
  {
    type_ = type;
  }

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

  LfiDaeSlowVoltageColumn()  {}

  long long size_;
  std::string type_;
  std::string fname_;
  
};

#pragma db object
class LfiDaeSlowVoltage: public EssentialMetadata
{
 public:
  LfiDaeSlowVoltage(std::string name, short version):
    EssentialMetadata(name, version)
  {
  }

  const std::string&
  runId() const
  {
    return runId_;
  }

  void 
  runId(std::string runId)
  {
    runId_ = runId;
  }

  const long long&
  startTime() const
  {
    return startTime_;
  }

  void 
  startTime(long long startTime)
  {
    startTime_ = startTime;
  }

  const long long&
  endTime() const
  {
    return endTime_;
  }

  void 
  endTime(long long endTime)
  {
    endTime_ = endTime;
  }

  const short&
  apid() const
  {
    return apid_;
  }

  void 
  apid(short apid)
  {
    apid_ = apid;
  }

  const char&
  type() const
  {
    return type_;
  }

  void 
  type(char type)
  {
    type_ = type;
  }

  const char&
  subtype() const
  {
    return subtype_;
  }

  void 
  subtype(char subtype)
  {
    subtype_ = subtype;
  }

  const short&
  pi1Val() const
  {
    return pi1Val_;
  }

  void 
  pi1Val(short pi1Val)
  {
    pi1Val_ = pi1Val;
  }
  
  const short&
  pi2Val() const
  {
    return pi1Val_;
  }

  void 
  pi2Val(short pi2Val)
  {
    pi2Val_ = pi2Val;
  }

 protected:

  friend class odb::access;
  LfiDaeSlowVoltage() {}  

 private:

  #pragma db id auto
  long long das_id_;
  std::string relative_path_;
  std::map<std::string, LfiDaeSlowVoltageColumn> columns_;

  std::string runId_;
  long long startTime_;
  long long endTime_;
  short apid_;
  char type_;
  char subtype_;
  short pi1Val_;
  short pi2Val_;
  
};

class TestLog;

#pragma db object
class TestLogImage: public EssentialMetadata
{

 public:
  TestLogImage(std::string name, short version)
    : EssentialMetadata(name, version)
  {}

  const int&
  naxis1() const
  {
    return naxis1_;
  }
  
  void 
  naxis1(int naxis1)
  {
    naxis1_ = naxis1;
  }
  
  const int&
  naxis2() const
  {
    return naxis1_;
  }
  
  void 
  naxis2(int naxis2)
  {
    naxis2_ = naxis2;
  }
  
  const std::string&
  format() const
  {
    return format_;
  }

  void 
  format(std::string format)
  {
    format_ = format;
  }  
  
 protected:
  friend class odb::access;
  TestLogImage() {}
  
 private:

  #pragma db id auto
  long long das_id_;

  int naxis1_;
  int naxis2_;
  std::string format_;

};


#pragma db object
class TestLog: public EssentialMetadata
{

 public:
  TestLog(std::string name, short version)
    : EssentialMetadata(name, version)
  {}

  const std::string&
  runId() const
  {
    return runId_;
  }

  void 
  runId(std::string runId)
  {
    runId_ = runId;
  }

  const long long&
  startTime() const
  {
    return startTime_;
  }

  void 
  startTime(long long startTime)
  {
    startTime_ = startTime;
  }

  const long long&
  endTime() const
  {
    return endTime_;
  }

  void 
  endTime(long long endTime)
  {
    endTime_ = endTime;
  }

  const std::string&
  log() const
  {
    return log_;
  }

  void 
  log(std::string log)
  {
    log_ = log;
  }

  const std::vector<lazy_shared_ptr<TestLogImage> >&
  images() const;

  std::vector<lazy_shared_ptr<TestLogImage> >&
  images();
  
 protected:
  friend class odb::access;
  TestLog() {}
  
 private:

  #pragma db id auto
  long long das_id_;
  
  #pragma db value_not_null unordered
  std::vector<lazy_shared_ptr<TestLogImage> > images_;
  
  std::string runId_;
  long long startTime_;
  long long endTime_;

  #pragma db type("TEXT")
  std::string log_;
  
};


#endif // DDL_INSTANCE_HXX
