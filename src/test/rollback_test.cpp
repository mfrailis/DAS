#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"
#include <boost/thread/thread.hpp>

using namespace std;
namespace D = das::tpl;

using namespace std;
//boost::mutex mtx;

void thread_func(long long id) {
    //    mtx.lock();
    try {
        shared_ptr<D::Database> db = D::Database::create("test_level1");
        D::Transaction t = db->begin(das::serializable);
        shared_ptr<lfiHkDaeSlowVoltage> ptr = db->load<lfiHkDaeSlowVoltage>(id);
        cout << "TH read" << endl;
        ptr->apid(1);
        cout << "TH before commit" << endl;
        t.commit();
        cout << "TH committed" << endl;
    } catch (const std::exception &e) {
        cout << "TH exception" << endl;
    }
    //    mtx.unlock();
}

int main(int argc, char** argv) {
    //    mtx.lock();
    shared_ptr<D::Database> db = D::Database::create("test_level1");

    shared_ptr<lfiHkDaeSlowVoltage> ptr = lfiHkDaeSlowVoltage::create("test1", "test_level1");

    das::Array<long long> a;
    a.resize(10);
    a(0) = 25;
    ptr->append_column<long long>("sampleOBT", a);
    a(9) = 15;

    D::Transaction t = db->begin(das::serializable);
    db->persist(ptr);
    t.commit();
    boost::thread th(&thread_func, ptr->das_id());

    ptr->append_column<long long>("sampleOBT", a);

    try {
        D::Transaction t = db->begin();
        cout << "M begin" << endl;
        db->attach(ptr);
        boost::thread::yield();
        shared_ptr<lfiHkDaeSlowVoltage> ptr1 = db->load<lfiHkDaeSlowVoltage>(ptr->das_id());

        //    mtx.unlock();

        //    mtx.lock();


        ptr->apid(2);
        cout << "M before commit main" << endl;
        t.commit();
        cout << "M committed" << endl;
    } catch (const std::exception &e) {
        cout << "M exception" << endl;
    }
    //    mtx.unlock();
    das::Array<long long> b = ptr->get_column<long long>("sampleOBT", 0, 20);

    cout << b << endl;

    th.join();

    t = db->begin(das::serializable);
    db->attach(ptr);
    t.commit();

    return 0;
}