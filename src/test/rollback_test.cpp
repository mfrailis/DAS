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

/*int main(int argc, char** argv) {
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
}*/


int main(){
    typedef boost::posix_time::ptime ptime;
    
    shared_ptr<test_keywords> p = test_keywords::create("test","test_level2");
    
    das::optional<signed char> a0 = p->get_key<signed char>("key_byte");
    das::optional<short> a1 = p->get_key<short>("key_int16"); 
    das::optional<int> a2 = p->get_key<int>("key_int32"); 
    das::optional<long long> a3 = p->get_key<long long>("key_int64");
    das::optional<float> a4 = p->get_key<float>("key_float32");
    das::optional<double> a5 = p->get_key<double>("key_float64");
    das::optional<bool> a6 = p->get_key<bool>("key_boolean");
    das::optional<char> a7 = p->get_key<char>("key_char");
    das::optional<std::string> a8 = p->get_key<std::string>("key_string");
    das::optional<std::string> a9 = p->get_key<std::string>("key_text");
    
    long long b = p->get_key<long long>("das_id");                   // das_id
    std::string b1 = p->get_key<std::string>("name");                 // name
    short b2 = p->get_key<short>("version");                       // version
    ptime ct =  p->get_key<ptime>("creationDate");  // creationDate
    
    return 0;
}


