#include <iostream>
#include <vector>

#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"
#include "ddl/types/ddl_measure.hpp"



using namespace std;
namespace D = das::tpl;

void print(const shared_ptr<lfiHkDaeSlowVoltage>& hk) {
    cout << "Run ID: " << hk->run_id() << endl
            << "  Start Time: " << hk->startTime() << endl
            << "  End Time  : " << hk->endTime() << endl
            << "  APID      : " << hk->apid() << endl;
}

int main(int argc, char * argv[]) {
    long long int ss_id;
    
    std::string db_alias("test_level1");
    
    {
        shared_ptr<D::Database> db = D::Database::create("test_level1");

        shared_ptr<site> s = site::create("site_name",db_alias);
        shared_ptr<testlevel> tl = testlevel::create("test_level_name",db_alias);
        shared_ptr<campaign> camp = campaign::create("campaign_2",db_alias);
        shared_ptr<session> ss = session::create("session_2",db_alias);

        camp->campaign_site(s);
        camp->campaign_test(tl);

        ss->session_campaign(camp);

        D::Transaction t(db->begin());
        ss_id = db->persist(ss); // save id for later use
        db->persist(camp); // unnecessary: camp is already made persistent by the association with session
        db->persist(tl); // unnecessary: as above 
        db->persist(s); // unnecessary: as above
        t.commit();

    }


    std::vector<shared_ptr<measure> > ms;

    ms.push_back(measure::create("measure_1",db_alias));
    ms.push_back(measure::create("measure_2",db_alias));
    ms.push_back(measure::create("measure_3",db_alias));
    for (std::vector<shared_ptr<measure> >::iterator i = ms.begin(); i < ms.end(); i++) {
        std::string log_name("log for measure ");
        log_name += (*i)->name();
        measure::log_vector logs;
        logs.push_back(measurelogs::create(log_name,db_alias));
        (*i)->log(logs);
    }



    {
        shared_ptr<D::Database> db = D::Database::create("test_level1");
        db->begin_session();
        D::Transaction t(db->begin());
        shared_ptr<session> ss = db->load<session>(ss_id);
        for (std::vector<shared_ptr<measure> >::iterator i = ms.begin(); i < ms.end(); i++) {
            (*i)->measure_session(ss);
            db->persist(*i);
        }
        t.commit();

        D::Transaction t2(db->begin());
        cout << "QUERY 0" << endl;
        D::Result<measure> r0 = db->query<measure>(
                "measure_session.session_campaign.name == 'campaign_1'",
                "startdate asc",false);
        
        cout << "QUERY 1" << endl;
        
        D::Result<measure> r = db->query<measure>(
                "measure_session.session_campaign.name == 'campaign_1'",
                "startdate asc",true);
        
        cout << "QUERY RESULT" << endl;
        
        const measure *row_ptr;
        for (D::Result<measure>::const_iterator i = r.cbegin(); i != r.cend(); ++i) {
            row_ptr = i.operator ->();
            cout << row_ptr->name() << " " << row_ptr->version() << endl;
            
        }
        t2.commit();
        db->end_session();
        
        D::Transaction t3(db->begin());
        D::Result<measure> r2 = db->query<measure>(
                "measure_session.session_campaign.name == 'campaign_1'",
                "startdate asc",true);
        
        cout << "QUERY RESULT" << endl;
        
        for (D::Result<measure>::iterator i = r2.begin(); i != r2.end(); ++i) {
            cout << i->name() << " " << i->version() << endl;
            
        }
        
        cout << "QUERY2 RESULT" << endl;
        r2 = db->query<measure>("das_id > 0", "startdate asc");
                for (D::Result<measure>::iterator i = r2.begin(); i != r2.end(); ++i) {
            cout << i->name() << " " << i->version() << endl;
            
        }
        t3.commit();
        
    }
       
    
    return 0;
}
