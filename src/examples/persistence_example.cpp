#include <iostream>
#include <vector>

#include "tpl/database.hpp"
#include "tpl/transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

int main(int argc, char** argv) {
    // create a ddl-object. His state is new
    shared_ptr<measure> m(measure::create("measure_name"));
    m->startdate(1372319348);

    {
        /* create a pu-object that handles 'test_level1' pu-instance.
         * Now it begins the persistent context handled by this object.
         */
        shared_ptr<D::Database> db = D::Database::create("test_level1");

        // create a transaction from the pu-object
        D::Transaction t(db->begin());
        // persist the object and attach him to db (pu-object)
        db->persist(m);
        // commit the changes to the persistent data structures (database)
        t.commit();

        // modify the object
        m->run_id(12345);
        
        // save changes of the attached objects
        db->flush();
        
        
    /* db goes out of scope and it will be destroyed:
     *      * system resources such as database connections will be released
     *      * managed context of the db object will end.
     */   
    }
    
    /* m object still exists but in the detached status.
     * We can keep modifying his data
     */
    m->obs_id("m_4567");
    
    /* create a new pu-object that handles the same pu-instance as before. */    
    shared_ptr<D::Database> db1 = D::Database::create("test_level1");
    
    /* attach m to the new pu-object which will eventually update the 
     * persistent counterpart of the object.
     */
    db1->attach(m);
    
    // save changes of the attached objects
    db1->flush();

    return 0;
}
