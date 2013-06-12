#ifndef AUX_QUERY_HPP
#define AUX_QUERY_HPP

#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>

#pragma db view
struct max_version
{
  short version;
};

#pragma db view
struct find_count
{
  unsigned long long count;
};

#endif // AUX_QUERY_HPP
