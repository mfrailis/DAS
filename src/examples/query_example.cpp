#include <iostream>
#include <vector>
#include <typeinfo>

#include "tpl/database.hpp"
#include "tpl/transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

int main(int argc, char** argv) {

    shared_ptr<D::Database> db = D::Database::create("test_level1");

    D::Transaction t(db->begin());
    
    D::Result<measure> r = db->query<measure>(
            "measure_session.session_campaign.name == 'campaign_1'",
            "startdate asc");   
    
    for(D::Result<measure>::iterator i = r.begin(); i!=r.end(); i++)
    {
        //cout << "name: " << i->name()  << endl;
        
        cout << "result it type: "<< typeid(*i).name() << endl;
        cout << "result it type: "<< typeid(i.load()).name() << endl; 
        shared_ptr<measure> p1 = i.load();
        shared_ptr<measure> p2 = i.load();
        
        bool eq = p1 == p2;
        cout << "same pointer? " << eq << endl;
        
    }
    
    
    shared_ptr<measure> sp;
    cout << "shared_ptr type: " << typeid(sp).name() << endl;
    
    
    t.commit();
    
    return 0;
}
