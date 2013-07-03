#include <iostream>
#include <vector>

#include "tpl/database.hpp"
#include "tpl/transaction.hpp"
#include "ddl/types.hpp"



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
    {
        shared_ptr<D::Database> db = D::Database::create("test_level1");

        shared_ptr<site> s = site::create("site_name");
        shared_ptr<testlevel> tl = testlevel::create("test_level_name");
        shared_ptr<campaign> camp = campaign::create("campaign_name");
        shared_ptr<session> ss = session::create("session_name");

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

    ms.push_back(measure::create("measure_1"));
    ms.push_back(measure::create("measure_2"));
    ms.push_back(measure::create("measure_3"));
    for (std::vector<shared_ptr<measure> >::iterator i = ms.begin(); i < ms.end(); i++) {
        std::string log_name("log for measure ");
        log_name += (*i)->name();
        measure::log_vector logs;
        logs.push_back(measurelogs::create(log_name));
        (*i)->log(logs);
    }



    {
        shared_ptr<D::Database> db = D::Database::create("test_level1");
        shared_ptr<session> ss = db->load<session>(ss_id);

        D::Transaction t(db->begin());
        for (std::vector<shared_ptr<measure> >::iterator i = ms.begin(); i < ms.end(); i++) {
            (*i)->measure_session(ss);
            db->persist(*i);
        }
        t.commit();

        D::Transaction t2(db->begin());

        D::Result<measure> r = db->query<measure>(
                "measure_session.session_campaign.name == 'campaign_1'",
                "startdate asc");
        
        for (D::Result<measure>::const_iterator i = r.cbegin(); i != r.cend(); ++i) {
            cout << i->name() << " " << i->version() << endl;
        }
        
        t2.commit();
    }
    return 0;
}
