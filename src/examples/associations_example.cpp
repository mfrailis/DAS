#include <iostream>
#include <vector>

#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

int main(int argc, char** argv) {
    
    // first we create some ddl-objects
    shared_ptr<session> ss = session::create("session_1","test_level1");

    shared_ptr<measure> m0 = measure::create("measure_0","test_level1");
    shared_ptr<measure> m1 = measure::create("measure_1","test_level1");
    shared_ptr<measure> m2 = measure::create("measure_2","test_level1");
    
    /* now we can create the following association tree using the appropriate 
     * setter methods
     *      m0  m1  m2
     *       |   |   |
     *       +---+---+
     *           |
     *          ss
     */
    
    m0->measure_session(ss);
    m1->measure_session(ss);
    m2->measure_session(ss);
    
    
    // now we can persist the ddl-objects
    shared_ptr<D::Database> db = D::Database::create("test_level1");
    D::Transaction t(db->begin());
    
    /* note that persist method recoursively calls himself on the associated
     * objects. This means that ss and then camp will also be pestisted
     * when persisting m0.
     */
    db->persist(m0);
    db->persist(m1);
    db->persist(m2);
    
    t.commit();
        
    return 0;
}

