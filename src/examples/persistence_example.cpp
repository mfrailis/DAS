#include <iostream>
#include <vector>

#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

int main(int argc, char** argv) {
    // create a ddl-object. His state is new
    shared_ptr<measure> m(measure::create("measure_name","test_level1"));
    // set a keyword
    m->startdate(1372319348);

    {
        // create a pu-object that handles 'test_level1' pu-instance.
        shared_ptr<D::Database> db = D::Database::create("test_level1");

        /* create a transaction from the pm-object. Since we didn't create a
         * session, the system will create one that covers the transaction
         */
        D::Transaction t(db->begin());
        // persist the object and attach him to the current session
        db->persist(m);
        /* commit the changes to the persistent data structures (database).
         * The session created by the system will end here
         */
        t.commit();

        /* m object still exists but in the detached status.
         * We can keep modifying his data
         */
        m->obs_id("m_4567");
        

    /* db goes out of scope and it will be destroyed allowing system resources
     * such as database connections to be released
     */
    }
    
    
    /* even now m is still valid and we can take advantage of his methods without
     * any concern about database resources
     */
    m->run_id(12345);


    {
        // create a new that handles the 'test_level1' pu-instance.
        shared_ptr<D::Database> db = D::Database::create("test_level1");
        D::Transaction t2(db->begin());

        // attach m to the session held by the current transaction.
        db->attach(m);

        // update and commit all the objects managed by the session, m in this case
        t2.commit();
    }
    return 0;
}
