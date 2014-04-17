#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

int main(){
  blitz::Array<int,1> bucket(512);

  blitz::Array<int,1> slice = bucket(blitz::Range(50,70));
  
  bucket(55) = 34;
  bucket(0) = 1;

  slice.reference(bucket(blitz::Range(55,70)));

  std::cout << slice << std::endl << bucket(blitz::Range(50,70)) << std::endl;

  bucket.resize(5000);

  int * data1 = bucket.data();

  int * data2 = slice.data();

  std::cout << *data1 << " " << *data2 << std::endl;
  std::cout << bucket.isStorageContiguous() << " " << slice.isStorageContiguous() << std::endl;
}
