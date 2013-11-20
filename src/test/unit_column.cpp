#include "unit_tpl_all.hpp"

namespace D = das::tpl;

template<typename T>
void
append(das::Array<T>& array,
        const std::string col_name,
        shared_ptr<test_columns>& ptr) {
    BOOST_REQUIRE_NO_THROW(ptr->append_column(col_name, array));
}

void
save(shared_ptr<test_columns>& ptr, shared_ptr<D::Database>& db) {
    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->attach(ptr);
            t.commit();
            );
}

/*template<typename T>
das::Array<T>
load(const std::string col_name,
        long long id,
        shared_ptr<D::Database>& db) {

    shared_ptr<test_columns> ptr;
    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            ptr = db->load<test_columns>(id);
            t.commit();
            );

    long long s = 0;
    BOOST_REQUIRE_NO_THROW(s = ptr->get_column_size(col_name));
    das::Array<T> array;
    BOOST_REQUIRE_NO_THROW(array.reference(ptr->get_column(col_name, 0, s)));
    return array;
}*/

template<typename T>
das::Array<T>
get(const std::string col_name, shared_ptr<test_columns>& ptr) {
    long long s = 0;
    BOOST_REQUIRE_NO_THROW(s = ptr->get_column_size(col_name));
    das::Array<T> array;
    BOOST_REQUIRE_NO_THROW(array.reference(ptr->get_column<T>(col_name, 0, s)));
    return array;
}

struct ColumnFixture {

    ColumnFixture() : id(0) {
        BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));
        BOOST_REQUIRE_NO_THROW(ptr = test_columns::create("keywords_test_0", "test_level2"));
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
void check(das::Array<T>& a, das::Array<T>& b) {
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
}

template<typename T>
void
test_case(
        const std::string col_name,
        shared_ptr<test_columns>& ptr,
        shared_ptr<D::Database>& db,
        das::Array<T>& base,
        das::Array<T>& ext1
        ) {
    
    BOOST_REQUIRE_NO_THROW(ptr->append_column(col_name, base));

    das::Array<T> a = get<T>(col_name, ptr);

    check(base, a);

    save(ptr, db);

    das::Array<T> b = get<T>(col_name, ptr);

    check(base, b);
    
    BOOST_REQUIRE_NO_THROW(ptr->append_column(col_name, ext1));
    
    das::Array<T> c;
    BOOST_REQUIRE_NO_THROW(c.reference(ptr->get_column<T>(col_name, 3, 4)));
    
    check(ext1, c);
    
    save(ptr, db);
    
    das::Array<T> d;
    BOOST_REQUIRE_NO_THROW(d.reference(ptr->get_column<T>(col_name, 3, 4)));
    
    check(ext1, d);
    
    long long s = 0;
    BOOST_REQUIRE_NO_THROW(s = ptr->get_column_size(col_name));
    
    BOOST_CHECK_EQUAL(s,7);
}

BOOST_FIXTURE_TEST_SUITE(column_data_unit_tests, ColumnFixture)

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
    
    test_case("column_char",ptr,db,base,ext);
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
    
    test_case("column_int16",ptr,db,base,ext);
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
    
    test_case("column_int32",ptr,db,base,ext);
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
    
    test_case("column_int64",ptr,db,base,ext);
}

BOOST_AUTO_TEST_CASE(column_float) {
    das::Array<long long> base(das::shape(3));
    base(0) = -22.202;
    base(1) = 152.13;
    base(2) = 2.13317;

    das::Array<long long> ext(das::shape(4));
    ext(0) = 254.4;
    ext(1) = -33.350;
    ext(2) = 186.6;
    ext(3) = -121.11;
    
    test_case("column_float32",ptr,db,base,ext);
}

BOOST_AUTO_TEST_CASE(column_double) {
    das::Array<long long> base(das::shape(3));
    base(0) = -22.202332;
    base(1) = 152.12243;
    base(2) = 2.133561337;

    das::Array<long long> ext(das::shape(4));
    ext(0) = 254.4234;
    ext(1) = -33.3554560;
    ext(2) = 186.6232234;
    ext(3) = -121.1551;
    
    test_case("column_float64",ptr,db,base,ext);
}

BOOST_AUTO_TEST_CASE(column_boolean) {
    das::Array<bool> base(das::shape(3));
    base(0) = true;
    base(1) = false;
    base(2) = false;

    das::Array<bool> ext(das::shape(4));
    ext(0) = true;
    ext(1) = 2;
    ext(2) = 1;
    ext(3) = 0;
    
    test_case("column_boolean",ptr,db,base,ext);
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
    
    test_case("column_uint8",ptr,db,base,ext);
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
    
    test_case("column_uin16",ptr,db,base,ext);
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
    
    test_case("column_uint32",ptr,db,base,ext);
}

BOOST_AUTO_TEST_SUITE_END()