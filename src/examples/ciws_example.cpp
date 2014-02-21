#include <iostream>

#include <das/tpl/database.hpp>
#include <das/transaction.hpp>
#include <das/ddl/types.hpp>

namespace D = das::tpl;

int main(){
  shared_ptr<s21> ptr = s21::create("sample","ciws");
  
  // dummy data to be stored
  int en_data[] = {1,1,1,2,3,3,4,5,6,12};
  // create a das::Array from existent C array
  das::Array<int> en(en_data,10,das::neverDeleteData);

  /* create a das::ColumnArray for storing 1-dimensional
   * short das::Array and reserve space for 5 elements
   */
  das::ColumnArray<short> ca(5);
  /* for each element create a das::Array that holds a buffer of
   * 64 short
   */
  ca(0).reference(das::Array<short>(64));
  ca(1).reference(das::Array<short>(64));
  ca(2).reference(das::Array<short>(64));
  ca(3).reference(das::Array<short>(64));
  ca(4).reference(das::Array<short>(64));

  // fill the ColumnArray with dummy data
  for(size_t i=0; i<5; ++i)
    for(size_t j=0; j<64; ++j)
      ca(i)(j) = i*100+j;

  // append the arrays to the columns
  ptr->append_column("configuration",en);
  ptr->append_column_array("PDM1_HighGain",ca); 

  // create a database instance for storing the object
  shared_ptr<D::Database> db = D::Database::create("ciws");

  // begin a transaction
  D::Transaction t = db->begin();
  // persist the object
  db->persist(ptr);
  // commit the transaction
  t.commit();

  // start a new transaction
  t = db->begin();
  /* query for objects called 'sample', ordering result by id ascending
   * the last parameter means that each version of the object must be 
   * returned.
   */
  D::Result<s21> r = db->query<s21>("name=='sample'","das_id asc",true);
  for(D::Result<s21>::iterator it=r.begin(); it!=r.end(); ++it){
    // load the object from the iterator
    shared_ptr<s21> raw = it.load();
    // print some essential metadata
    std::cout << raw->name() <<
      " id:" << raw->das_id() <<
      " version:" << raw->version() <<
      " insert-time:" << raw->creationDate() <<
      std::endl;
    // retrive all "configuration" column data
    das::Array<int> array = raw->get_column<int>("configuration");
    std::cout << "configuration" << std::endl << array << std::endl;

    // retrive 3 array-elements from column "PDM1_HighGain" given an offset of 2
    das::ColumnArray<short> carray = raw->get_column_array<short,1>("PDM1_HighGain", 2, 3);
    std::cout << "PDM1_HighGain" << std::endl << carray << std::endl;
  }
  t.commit();
}
