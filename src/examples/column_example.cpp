#include <iostream>
#include <vector>

#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

int main(int argc, char** argv) {
    
    // first we create a ddl-object
    shared_ptr<lfiHkDaeSlowVoltage> lfi = lfiHkDaeSlowVoltage::create("lfiHkDaeSlowVoltage_1","test_level1");

    // we create a das::Array and we populate with some data.
    das::Array<long long> obt(das::shape(5));
    obt(0) = 125685654;
    obt(1) = 125686899; 
    obt(2) = 125765952; 
    obt(3) = 126847426;
    obt(4) = 127056466;
    
    /* we add the array to the column "sampleOBT".
     * Note that we store a reference, not the actual array
     */
    lfi->append_column("sampleOBT", obt);
    
    // now we can persist the ddl-object and its data
    shared_ptr<D::Database> db = D::Database::create("test_level1");
    D::Transaction t(db->begin());
    db->persist(lfi);
    t.commit();
        
    das::Array<int> eng(das::shape(3));
    obt(0) = 942;
    obt(1) = 1724; 
    obt(2) = 2347;
    
    /* we append an array of int to a column of type double:
     * the input array will be copied a casted to double before be appended
     */
    lfi->append_column("LM151322Eng",eng);
    
    // we can retrive the data, always as a copy array
    das::Array<long long> obt1 = lfi->get_column<long long>("sampleOBT");
    
    // even if not persistent yet
    das::Array<double> eng1 = lfi->get_column<double>("LM151322Eng");
    
    // we can also require a different type, if compatible
    das::Array<float> eng2 = lfi->get_column<float>("LM151322Eng");
    
    /* we can of course retrive only a part of the column specifing 
     * offset and length
     */
    das::Array<long long> obt2 = lfi->get_column<long long>("sampleOBT",1,3);
    
    // we persist the data added after last transaction
    t = db->begin();
    db->attach(lfi);
    t.commit();
    
    
    return 0;
}