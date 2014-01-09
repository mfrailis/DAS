#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

using namespace std;
//boost::mutex mtx;
#define BOOST_REQUIRE_NO_THROW(s) s
#define BOOST_CHECK_NO_THROW(s) s
#define BOOST_REQUIRE_NE(x,y)
#define BOOST_CHECK_EQUAL(x,y)
#define BOOST_CHECK_THROW(x,e) try{ x; }catch(const e & exc){}




template<typename IT0, typename IT1>
void BOOST_CHECK_EQUAL_COLLECTIONS(IT0 L_begin, IT0 L_end, IT1 R_begin, IT1 R_end) {
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
save(shared_ptr<test_columns_blob>& ptr, shared_ptr<D::Database>& db) {
    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->attach(ptr);
            t.commit();
            );
}


shared_ptr<D::Database> db;
shared_ptr<test_columns_blob> ptr;
long long id;

void ColumnFixtureBlob() {
    id = 0;

    db = D::Database::create("test_level2");
    ptr = test_columns_blob::create("column_unit_test_0", "test_level2");

            D::Transaction t = db->begin(); 
            db->persist(ptr);
            t.commit();

    id = ptr->das_id();

}

void change() {
ptr = test_columns_blob::create("column_unit_test_1", "test_level2");

            D::Transaction t = db->begin();
            db->persist(ptr);
            t.commit();

    id = ptr->das_id();

}

template<typename T>
void
test_case_blob(
        const std::string col_name,
        shared_ptr<test_columns_blob>& ptr,
        shared_ptr<D::Database>& db,
        das::Array<T>& base,
        das::Array<T>& ext1
        ) {

   // BOOST_CHECK_THROW(ptr->get_column<T>(col_name, 0, 10), das::empty_column);

ptr->append_column(col_name, base);

    das::Array<T> a;
a.reference(ptr->get_column<T>(col_name));

    BOOST_CHECK_EQUAL_COLLECTIONS(base.begin(), base.end(), a.begin(), a.end());

    das::Array<T> a1;
    a1.reference(ptr->get_column<T>(col_name, 1, 2));

    typename das::Array<T>::iterator it = base.begin();
    ++it;
    BOOST_CHECK_EQUAL_COLLECTIONS(it, base.end(), a1.begin(), a1.end());

//    BOOST_CHECK_THROW(ptr->get_column<T>(col_name, 0, 10), das::io_exception);

    save(ptr, db);

//    BOOST_CHECK_THROW(ptr->get_column<T>(col_name, 0, 10), das::io_exception);

    das::Array<T> b;
    b.reference(ptr->get_column<T>(col_name));

    BOOST_CHECK_EQUAL_COLLECTIONS(base.begin(), base.end(), b.begin(), b.end());

    ptr->append_column(col_name, ext1);

    das::Array<T> c;
    BOOST_REQUIRE_NO_THROW(c.reference(ptr->get_column<T>(col_name, 3, 4)));

    BOOST_CHECK_EQUAL_COLLECTIONS(ext1.begin(), ext1.end(), c.begin(), c.end());

 //   BOOST_CHECK_THROW(ptr->get_column<T>(col_name, 0, 10), das::io_exception);

    save(ptr, db);

 //   BOOST_CHECK_THROW(ptr->get_column<T>(col_name, 0, 10), das::io_exception);

    das::Array<T> d;
    d.reference(ptr->get_column<T>(col_name, 3, 4));

    BOOST_CHECK_EQUAL_COLLECTIONS(ext1.begin(), ext1.end(), d.begin(), d.end());

    long long s = 0;
    s = ptr->get_column_size(col_name);



}

int main(){

    cout << "START" << endl;
    /*BOOST_AUTO_TEST_CASE(conversion_exceptions)*/
    {

        ColumnFixtureBlob();
        das::Array<int> base(das::shape(3));
        base(0) = -2220;
        base(1) = 1523;
        base(2) = 2117;
        BOOST_CHECK_THROW(ptr->append_column("column_string", base), das::bad_type);

        das::Array<std::string> str(das::shape(3));
        str(0) = "string1";
        str(1) = "string2";
        str(2) = "string3";
        BOOST_CHECK_THROW(ptr->append_column("column_float32", str), das::bad_type);

        das::ColumnArray<int> img;
        img.resize(2);
        img(0).resize(1);
        img(1).resize(1);
        BOOST_CHECK_THROW(ptr->append_column_array("column_float32", img), das::bad_array_shape);

        BOOST_CHECK_NO_THROW(ptr->append_column("column_int32", base));

        BOOST_CHECK_THROW(ptr->get_column<std::string>("column_int32", 0, 3), das::bad_type);
    }

    /*BOOST_AUTO_TEST_CASE(column_char)*/
    {
        ColumnFixtureBlob();
        das::Array<char> base(das::shape(3));
        base(0) = -22;
        base(1) = 15;
        base(2) = 27;

        das::Array<char> ext(das::shape(4));
        ext(0) = 56;
        ext(1) = -30;
        ext(2) = 18;
        ext(3) = -12;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_char", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_char", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_short)*/
    {
        ColumnFixtureBlob();
        das::Array<short> base(das::shape(3));
        base(0) = -222;
        base(1) = 15;
        base(2) = 27;

        das::Array<short> ext(das::shape(4));
        ext(0) = 254;
        ext(1) = -30;
        ext(2) = 18;
        ext(3) = -121;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_int16", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_int16", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_int)*/
    {
        ColumnFixtureBlob();
        das::Array<int> base(das::shape(3));
        base(0) = -2220;
        base(1) = 1523;
        base(2) = 2117;

        das::Array<int> ext(das::shape(4));
        ext(0) = 254;
        ext(1) = -3330;
        ext(2) = 18;
        ext(3) = -121;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_int32", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_int32", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_long_long)*/
    {
        ColumnFixtureBlob();
        das::Array<long long> base(das::shape(3));
        base(0) = -22202;
        base(1) = 15213;
        base(2) = 213317;

        das::Array<long long> ext(das::shape(4));
        ext(0) = 2544;
        ext(1) = -33350;
        ext(2) = 1866;
        ext(3) = -12111;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_int64", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_int64", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_float)*/
    {
        ColumnFixtureBlob();
        das::Array<float> base(das::shape(3));
        base(0) = -22.202;
        base(1) = 152.13;
        base(2) = 2.13317;

        das::Array<float> ext(das::shape(4));
        ext(0) = 254.4;
        ext(1) = -33.350;
        ext(2) = 186.6;
        ext(3) = -121.11;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_float32", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_float32", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_double)*/
    {
        ColumnFixtureBlob();
        das::Array<double> base(das::shape(3));
        base(0) = -22.202332;
        base(1) = 152.12243;
        base(2) = 2.133561337;

        das::Array<double> ext(das::shape(4));
        ext(0) = 254.4234;
        ext(1) = -33.3554560;
        ext(2) = 186.6232234;
        ext(3) = -121.1551;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_float64", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_float64", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_boolean)*/
    {
        ColumnFixtureBlob();
        das::Array<bool> base(das::shape(3));
        base(0) = true;
        base(1) = false;
        base(2) = false;

        das::Array<bool> ext(das::shape(4));
        ext(0) = true;
        ext(1) = true;
        ext(2) = false;
        ext(3) = false;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_boolean", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_boolean", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_uchar)*/
    {
        ColumnFixtureBlob();
        das::Array<unsigned char> base(das::shape(3));
        base(0) = 22;
        base(1) = 15;
        base(2) = 27;

        das::Array<unsigned char> ext(das::shape(4));
        ext(0) = 56;
        ext(1) = 30;
        ext(2) = 18;
        ext(3) = 12;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_uint8", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_uint8", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_ushort)*/
    {
        ColumnFixtureBlob();
        das::Array<unsigned short> base(das::shape(3));
        base(0) = 222;
        base(1) = 15;
        base(2) = 27;

        das::Array<unsigned short> ext(das::shape(4));
        ext(0) = 254;
        ext(1) = 30;
        ext(2) = 18;
        ext(3) = 121;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_uin16", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_uin16", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_uint)*/
    {
        ColumnFixtureBlob();
        das::Array<unsigned int> base(das::shape(3));
        base(0) = 2220;
        base(1) = 1523;
        base(2) = 2117;

        das::Array<unsigned int> ext(das::shape(4));
        ext(0) = 254;
        ext(1) = 3330;
        ext(2) = 18;
        ext(3) = 121;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_uint32", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_uint32", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_string)*/
    {
        ColumnFixtureBlob();
        das::Array<std::string> base(das::shape(3));
        base(0) = "string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01\
string01string01string01string01string01string01string01string01string01string01";
        base(1) = "string2";
        base(2) = "string3";

        das::Array<std::string> ext(das::shape(4));
        ext(0) = "string chunk 2.1";
        ext(1) = "string chunk 2.1";
        ext(2) = "string chunk 2.1";
        ext(3) = "string chunk 2.1";

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_string", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_string", ptr, db, base, ext);
    }

    /*BOOST_AUTO_TEST_CASE(column_float_conversion)*/
    {
        ColumnFixtureBlob();
        das::Array<int> base(das::shape(3));
        base(0) = -2220;
        base(1) = 1523;
        base(2) = 2117;

        das::Array<int> ext(das::shape(4));
        ext(0) = 254;
        ext(1) = -3330;
        ext(2) = 18;
        ext(3) = -121;

        das::DatabaseConfig::database("test_level2").buffered_data(true);
        test_case_blob("column_float32", ptr, db, base, ext);

        change();
        das::DatabaseConfig::database("test_level2").buffered_data(false);
        test_case_blob("column_float32", ptr, db, base, ext);
    }
    return 0;
}