#include "unit_tpl_all.hpp"

namespace D = das::tpl;

void
save(shared_ptr<test_columns>& ptr, shared_ptr<D::Database>& db) {
    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->attach(ptr);
            t.commit();
            );
}

struct ColumnFixture {

    ColumnFixture() : id(0) {
        BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));
        BOOST_REQUIRE_NO_THROW(ptr = test_columns::create("column_unit_test_0", "test_level2"));
        BOOST_REQUIRE_NO_THROW(
                D::Transaction t = db->begin();
                db->persist(ptr);
                t.commit();
                );
        id = ptr->das_id();
        BOOST_REQUIRE_NE(id, 0);
    }

    void change() {
        BOOST_REQUIRE_NO_THROW(ptr = test_columns::create("column_unit_test_1", "test_level2"));
        BOOST_REQUIRE_NO_THROW(
                D::Transaction t = db->begin();
                db->persist(ptr);
                t.commit();
                );
        id = ptr->das_id();
        BOOST_REQUIRE_NE(id, 0);
    }

    ~ColumnFixture() {
    }

    shared_ptr<D::Database> db;
    shared_ptr<test_columns> ptr;
    long long id;
};

template<typename T>
void
test_case(
        const std::string col_name,
        shared_ptr<test_columns>& ptr,
        shared_ptr<D::Database>& db,
        das::Array<T>& base,
        das::Array<T>& ext1
        ) {
    
    BOOST_CHECK_THROW(ptr->get_column<T>(col_name,0,10),das::empty_column);

    BOOST_REQUIRE_NO_THROW(ptr->append_column(col_name, base));

    das::Array<T> a;
    BOOST_REQUIRE_NO_THROW(a.reference(ptr->get_column<T>(col_name)));

    BOOST_CHECK_EQUAL_COLLECTIONS(base.begin(), base.end(), a.begin(), a.end());

    das::Array<T> a1;
    BOOST_REQUIRE_NO_THROW(a1.reference(ptr->get_column<T>(col_name, 1, 2)));

    typename das::Array<T>::iterator it = base.begin();
    ++it;
    BOOST_CHECK_EQUAL_COLLECTIONS(it, base.end(), a1.begin(), a1.end());

    BOOST_CHECK_THROW(ptr->get_column<T>(col_name,0,10),das::io_exception);
    
    save(ptr, db);

    BOOST_CHECK_THROW(ptr->get_column<T>(col_name,0,10),das::io_exception);
    
    das::Array<T> b;
    BOOST_REQUIRE_NO_THROW(b.reference(ptr->get_column<T>(col_name)));

    BOOST_CHECK_EQUAL_COLLECTIONS(base.begin(), base.end(), b.begin(), b.end());

    BOOST_REQUIRE_NO_THROW(ptr->append_column(col_name, ext1));

    das::Array<T> c;
    BOOST_REQUIRE_NO_THROW(c.reference(ptr->get_column<T>(col_name, 3, 4)));

    BOOST_CHECK_EQUAL_COLLECTIONS(ext1.begin(), ext1.end(), c.begin(), c.end());

    BOOST_CHECK_THROW(ptr->get_column<T>(col_name,0,10),das::io_exception);
    
    save(ptr, db);
    
    BOOST_CHECK_THROW(ptr->get_column<T>(col_name,0,10),das::io_exception);

    das::Array<T> d;
    BOOST_REQUIRE_NO_THROW(d.reference(ptr->get_column<T>(col_name, 3, 4)));

    BOOST_CHECK_EQUAL_COLLECTIONS(ext1.begin(), ext1.end(), d.begin(), d.end());

    long long s = 0;
    BOOST_REQUIRE_NO_THROW(s = ptr->get_column_size(col_name));

    BOOST_CHECK_EQUAL(s, 7);
    
}

BOOST_FIXTURE_TEST_SUITE(column_data_unit_tests, ColumnFixture)

BOOST_AUTO_TEST_CASE(conversion_exceptions) {
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

BOOST_AUTO_TEST_CASE(column_char) {
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
    test_case("column_char", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_char", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_short) {
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
    test_case("column_int16", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_int16", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_int) {
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
    test_case("column_int32", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_int32", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_long_long) {
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
    test_case("column_int64", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_int64", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_float) {
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
    test_case("column_float32", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_float32", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_double) {
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
    test_case("column_float64", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_float64", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_boolean) {
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
    test_case("column_boolean", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_boolean", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_uchar) {
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
    test_case("column_uint8", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_uint8", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_ushort) {
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
    test_case("column_uin16", ptr, db, base, ext);
    
    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_uin16", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_uint) {
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
    test_case("column_uint32", ptr, db, base, ext);
    
    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_uint32", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_string) {
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
    test_case("column_string", ptr, db, base, ext);
    
    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_string", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_float_conversion) {
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
    test_case("column_float32", ptr, db, base, ext);
     
    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);   
    test_case("column_float32", ptr, db, base, ext);  
}





BOOST_AUTO_TEST_SUITE_END()