#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"
#include <boost/thread/thread.hpp>

using namespace std;
namespace D = das::tpl;

using namespace std;
//boost::mutex mtx;
#define BOOST_REQUIRE_NO_THROW(s) s
#define BOOST_REQUIRE_NE(x,y)

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

template<typename IT0, typename IT1>
void CHECK_EQUAL_COLLECTIONS(IT0 L_begin, IT0 L_end, IT1 R_begin, IT1 R_end) {
    IT0 L_it = L_begin;
    IT1 R_it = R_begin;
    size_t i = 0;

    while (L_it != L_end && R_it != R_end) {
        if (*L_it != *R_it) {
            cout << "[" << i << "] L_it=" << *L_it << " R_it=" << *R_it << endl;
            return;
        }
        ++L_it;
        ++R_it;
        ++i;
    }

    if (L_it != L_end)
        cout << "[" << i << "] L_it has more elements" << endl;

    if (R_it != R_end)
        cout << "[" << i << "] R_it has more elements" << endl;
}

template<typename T, int Rank>
void CHECK_NESTED_COLLECTIONS(
        typename das::ColumnArray<T, Rank>::iterator L_begin,
        typename das::ColumnArray<T, Rank>::iterator L_end,
        typename das::ColumnArray<T, Rank>::iterator R_begin,
        typename das::ColumnArray<T, Rank>::iterator R_end) {

    using das::ColumnArray;

    typedef typename ColumnArray<T, Rank>::iterator array_iterator;

    size_t i = 0;
    array_iterator L_it = L_begin;
    array_iterator R_it = R_begin;
    while (L_it != L_end && R_it != R_end) {
        CHECK_EQUAL_COLLECTIONS(L_it->begin(), L_it->end(),
                R_it->begin(), R_it->end());
        ++L_it;
        ++R_it;
        ++i;
    }

    if (L_it != L_end)
        cout << "[" << i << "] L_it has more elements" << endl;

    if (R_it != R_end)
        cout << "[" << i << "] R_it has more elements" << endl;

}

void
save(shared_ptr<test_columns_array>& ptr, shared_ptr<D::Database>& db) {
    D::Transaction t = db->begin();
    db->attach(ptr);
    t.commit();
}

template<typename T, int Rank>
void
test_case(
        const std::string col_name,
        shared_ptr<test_columns_array>& ptr,
        shared_ptr<D::Database>& db,
        das::ColumnArray<T, Rank>& base,
        das::ColumnArray<T, Rank>& ext1
        ) {
    typedef typename das::ColumnArray<T, Rank>::iterator array_it;

    ptr->append_column_array(col_name, base);

    das::ColumnArray<T, Rank> a = ptr->get_column_array<T, Rank>(col_name);

    cout << a(0).shape() << endl;
    cout << a(1).shape() << endl;
    cout << a(2).shape() << endl;

    CHECK_NESTED_COLLECTIONS<T, Rank>(base.begin(), base.end(), a.begin(), a.end());

    cout << "CHECK_NESTED_COLLECTIONS 0" << endl;

    das::ColumnArray<T, Rank> a1 = ptr->get_column_array<T, Rank>(col_name, 1, 2);

    cout << a1(0).shape() << endl;
    cout << a1(1).shape() << endl;

    typename das::ColumnArray<T, Rank>::iterator it = base.begin();
    ++it;
    CHECK_NESTED_COLLECTIONS<T, Rank>(it, base.end(), a1.begin(), a1.end());

    cout << "CHECK_NESTED_COLLECTIONS 1" << endl;

    save(ptr, db);


    das::ColumnArray<T, Rank> b = ptr->get_column_array<T, Rank>(col_name);

    cout << b(0).shape() << endl;
    cout << b(1).shape() << endl;
    cout << b(2).shape() << endl;

    CHECK_NESTED_COLLECTIONS<T, Rank>(base.begin(), base.end(), b.begin(), b.end());
    cout << "CHECK_NESTED_COLLECTIONS 2" << endl;

    ptr->append_column_array(col_name, ext1);

    das::ColumnArray<T, Rank> c = ptr->get_column_array<T, Rank>(col_name, 3, 4);

    cout << c(0).shape() << endl;
    cout << c(1).shape() << endl;
    cout << c(2).shape() << endl;
    cout << c(3).shape() << endl;

    CHECK_NESTED_COLLECTIONS<T, Rank>(ext1.begin(), ext1.end(), c.begin(), c.end());
    cout << "CHECK_NESTED_COLLECTIONS 3" << endl;
    save(ptr, db);


    das::ColumnArray<T, Rank> d = ptr->get_column_array<T, Rank>(col_name, 3, 4);

    cout << d(0).shape() << endl;
    cout << d(1).shape() << endl;
    cout << d(2).shape() << endl;
    cout << d(3).shape() << endl;
    CHECK_NESTED_COLLECTIONS<T, Rank>(ext1.begin(), ext1.end(), d.begin(), d.end());
    cout << "CHECK_NESTED_COLLECTIONS 4" << endl;
    long long s = ptr->get_column_array_size(col_name);

    if (s != 7)
        cout << "dim error" << endl;

}
shared_ptr<D::Database> db;
shared_ptr<test_columns_array> ptr;
long long id;

void ColumnFixture() {

    BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));
    BOOST_REQUIRE_NO_THROW(ptr = test_columns_array::create("column_array_unit_test_0", "test_level2"));
    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->persist(ptr);
            t.commit();
            );
    id = ptr->das_id();
    BOOST_REQUIRE_NE(id, 0);
}

void change() {
    BOOST_REQUIRE_NO_THROW(ptr = test_columns_array::create("column__array_unit_test_1", "test_level2"));
    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->persist(ptr);
            t.commit();
            );
    id = ptr->das_id();
    BOOST_REQUIRE_NE(id, 0);
}

int main() {
    /*   typedef boost::posix_time::ptime ptime;
    
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
     */


    ColumnFixture();

    int b0[] = {0, 1, 2, 3, 4, 5};
    int b1[] = {7, 8, 9, 10, 11, 12};
    int b2[] = {13, 14, 15, 16, 17, 18};

    int e0[] = {19, 20, 21, 22, 23, 24};
    int e1[] = {25, 26, 27, 28, 29, 30};
    int e2[] = {31, 32, 33, 34, 35, 36};
    int e3[] = {37, 38, 39, 40, 41, 42};

    das::ColumnArray<int, 2> base(das::shape(3));
    base(0).reference(das::Array<int, 2>(b0, das::shape(2, 3), das::neverDeleteData));
    base(1).reference(das::Array<int, 2>(b1, das::shape(2, 3), das::neverDeleteData));
    base(2).reference(das::Array<int, 2>(b2, das::shape(2, 3), das::neverDeleteData));

    das::ColumnArray<int, 2> ext(4);
    ext(0).reference(das::Array<int, 2>(e0, das::shape(2, 3), das::neverDeleteData));
    ext(1).reference(das::Array<int, 2>(e1, das::shape(2, 3), das::neverDeleteData));
    ext(2).reference(das::Array<int, 2>(e2, das::shape(2, 3), das::neverDeleteData));
    ext(3).reference(das::Array<int, 2>(e3, das::shape(2, 3), das::neverDeleteData));

    das::DatabaseConfig::database("test_level2").buffered_data(true);
    test_case("column_byte", ptr, db, base, ext);
    /*
        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case("column_byte", ptr, db, base, ext);
     */

    return 0;
}


