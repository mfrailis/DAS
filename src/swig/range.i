%{
#include "internal/array.hpp"
  %}

namespace das{
  class Range {
  public:
  
    Range();
    Range(int slicePosition);
  
    Range(int first, int last, int stride = 1);
  
    size_t length() const ;
    static Range all();
  };
}
