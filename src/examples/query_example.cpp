#include <iostream>
#include <vector>
#include <typeinfo>

#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

int main(int argc, char** argv) {

    shared_ptr<D::Database> db = D::Database::create("test_level1");

    D::Transaction t(db->begin());
    
    D::Result<measure> r = db->query<measure>(
            "measure_session.session_campaign.name == 'campaign_1'",
            "startdate asc");   
    
    for(D::Result<measure>::const_iterator i = r.cbegin(); i!=r.cend(); i++)
    {
        cout << "name: " << i->name()  << endl;
    }
    
    t.commit();
    
    return 0;
}
