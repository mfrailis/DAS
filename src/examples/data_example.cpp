#include <iostream>

#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

long long write_data(const shared_ptr<D::Database> &db){
    /* first we create a standard c array with some values */
    float data[] = { 0.22, 1.55, 2.99, 3.44, 4.11};

    /* then we assign it to a das::Array object.
     * The third argument establishes whether or not to delete the data array
     * while deleting the das::Array object.
     */
    das::Array<float> array(data,5,das::neverDeleteData);
    
    /* we create an object which contains a binary table*/
    shared_ptr<test_columns> ptr = test_columns::create("test", "test_level2");

    /* then we append the array to the column identified by the string passed as
     * first argument
     */
    ptr->append_column("column_float32",array);
    
    /* now we just have to persist the object which implies the data persistance
     * also
     */
    D::Transaction t = db->begin();
    long long id = db->persist(ptr);
    t.commit();
    
    return id;
}

void read_data(const shared_ptr<D::Database> &db, long long id){
    /* we load the object givend its id*/
    D::Transaction t = db->begin();
    shared_ptr<test_columns> ptr = db->load<test_columns>(id);
    t.commit();
    
    /* then we retrive the column data giving the column name, an offset and
     * the number of the objects to load
     */
    das::Array<float> array = ptr->get_column<float>("column_float32",0,5);
    
    std::cout << array << std::endl;
}

int main(){
    shared_ptr<D::Database> db = D::Database::create("test_level2");
    
    long long id = write_data(db);
    read_data(db,id);
}