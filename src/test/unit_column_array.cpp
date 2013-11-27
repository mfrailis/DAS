#include "unit_tpl_all.hpp"
#include <string>

namespace D = das::tpl;

typedef das::ColumnArray<char, 2>::iterator array_char_2_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_char_2_iterator);

typedef das::ColumnArray<char>::iterator array_char_1_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_char_1_iterator);

typedef das::ColumnArray<short, 2>::iterator array_short_2_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_short_2_iterator);

typedef das::ColumnArray<int, 2>::iterator array_int_2_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_int_2_iterator);

typedef das::ColumnArray<long long>::iterator array_ll_1_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_ll_1_iterator);

typedef das::ColumnArray<float>::iterator array_float_1_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_float_1_iterator);

typedef das::ColumnArray<float, 2>::iterator array_float_2_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_float_2_iterator);

typedef das::ColumnArray<std::string>::iterator array_str_1_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_str_1_iterator);

typedef das::ColumnArray<double>::iterator array_double_1_iterator;
BOOST_TEST_DONT_PRINT_LOG_VALUE(array_double_1_iterator);

template<typename T, int Rank>
void CHECK_NESTED_COLLECTIONS(
        typename das::ColumnArray<T, Rank>::iterator L_begin,
        typename das::ColumnArray<T, Rank>::iterator L_end,
        typename das::ColumnArray<T, Rank>::iterator R_begin,
        typename das::ColumnArray<T, Rank>::iterator R_end) {

    using das::ColumnArray;

    typedef typename ColumnArray<T, Rank>::iterator array_iterator;


    array_iterator L_it = L_begin;
    array_iterator R_it = R_begin;
    while (L_it != L_end && R_it != R_end) {
        BOOST_CHECK_EQUAL_COLLECTIONS(L_it->begin(), L_it->end(),
                R_it->begin(), R_it->end());
        ++L_it;
        ++R_it;
    }


    BOOST_CHECK_EQUAL(L_it, L_end);
    BOOST_CHECK_EQUAL(R_it, R_end);
}

void
save(shared_ptr<test_columns_array>& ptr, shared_ptr<D::Database>& db) {
    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->attach(ptr);
            t.commit();
            );
}

template<typename T, int Rank>
das::ColumnArray<T, Rank>
get(const std::string col_name, shared_ptr<test_columns_array>& ptr) {
    long long s = 0;
    BOOST_REQUIRE_NO_THROW(s = ptr->get_column_array_size(col_name));
    das::ColumnArray<T, Rank> array;
    BOOST_REQUIRE_NO_THROW(array.reference(ptr->get_column_array<T, Rank>(col_name, 0, s)));
    return array;
}

struct ColumnArrayFixture {

    ColumnArrayFixture() : id(0) {
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

    ~ColumnArrayFixture() {
    }

    shared_ptr<D::Database> db;
    shared_ptr<test_columns_array> ptr;
    long long id;
};

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
    BOOST_CHECK_THROW((ptr->get_column_array<T, Rank>(col_name, 0, 10)), das::empty_column);

    BOOST_REQUIRE_NO_THROW(ptr->append_column_array(col_name, base));

    das::ColumnArray<T, Rank> a = get<T, Rank>(col_name, ptr);

    CHECK_NESTED_COLLECTIONS<T, Rank>(base.begin(), base.end(), a.begin(), a.end());

    das::ColumnArray<T, Rank> a1;
    BOOST_REQUIRE_NO_THROW(a1.reference(ptr->get_column_array<T, Rank>(col_name, 1, 2)));

    typename das::ColumnArray<T, Rank>::iterator it = base.begin();
    ++it;
    CHECK_NESTED_COLLECTIONS<T, Rank>(it, base.end(), a1.begin(), a1.end());

    BOOST_CHECK_THROW((ptr->get_column_array<T, Rank>(col_name, 0, 10)), das::io_exception);

    save(ptr, db);

    BOOST_CHECK_THROW((ptr->get_column_array<T, Rank>(col_name, 0, 10)), das::io_exception);

    das::ColumnArray<T, Rank> b = get<T, Rank>(col_name, ptr);

    CHECK_NESTED_COLLECTIONS<T, Rank>(base.begin(), base.end(), b.begin(), b.end());

    BOOST_REQUIRE_NO_THROW(ptr->append_column_array(col_name, ext1));

    das::ColumnArray<T, Rank> c;
    BOOST_REQUIRE_NO_THROW(c.reference(ptr->get_column_array<T, Rank>(col_name, 3, 4)));

    CHECK_NESTED_COLLECTIONS<T, Rank>(ext1.begin(), ext1.end(), c.begin(), c.end());

    BOOST_CHECK_THROW((ptr->get_column_array<T, Rank>(col_name, 0, 10)), das::io_exception);

    save(ptr, db);

    BOOST_CHECK_THROW((ptr->get_column_array<T, Rank>(col_name, 0, 10)), das::io_exception);

    das::ColumnArray<T, Rank> d;
    BOOST_REQUIRE_NO_THROW(d.reference(ptr->get_column_array<T, Rank>(col_name, 3, 4)));

    CHECK_NESTED_COLLECTIONS<T, Rank>(ext1.begin(), ext1.end(), d.begin(), d.end());

    long long s = 0;
    BOOST_REQUIRE_NO_THROW(s = ptr->get_column_array_size(col_name));

    BOOST_CHECK_EQUAL(s, 7);

}

BOOST_FIXTURE_TEST_SUITE(column_array_data_unit_tests, ColumnArrayFixture)

BOOST_AUTO_TEST_CASE(conversion_exceptions) {
    das::ColumnArray<int> a(das::shape(3));
    a(0).resize(3);
    a(1).resize(3);
    a(2).resize(3);
    BOOST_CHECK_THROW(ptr->append_column_array("column_int32", a), das::bad_array_shape);

    das::ColumnArray<int, 2> b(das::shape(3));
    b(0).resize(2, 3);
    b(1).resize(2, 3);
    b(2).resize(2, 3);
    BOOST_CHECK_THROW(ptr->append_column_array("column_int32", b), das::bad_array_shape);

    das::ColumnArray<std::string, 2> str(das::shape(2));
    str(0).resize(2,2);
    str(1).resize(2,2);
    BOOST_CHECK_THROW(ptr->append_column_array("column_int32", str), das::bad_type);
    BOOST_CHECK_THROW(ptr->append_column_array("column_string", a), das::bad_type);

    char b0[] = {0, 1, 2, 3, 4, 5};
    das::ColumnArray<char, 2> base(das::shape(3));
    base(0).reference(das::Array<char, 2>(b0, das::shape(2, 3), das::neverDeleteData));
    base(1).reference(das::Array<char, 2>(b0, das::shape(2, 3), das::neverDeleteData));
    base(2).reference(das::Array<char, 2>(b0, das::shape(2, 3), das::neverDeleteData));
    BOOST_CHECK_NO_THROW(ptr->append_column_array("column_byte", base));

    BOOST_CHECK_NO_THROW((ptr->get_column_array<char, 2 >("column_byte")));

    BOOST_CHECK_THROW((ptr->get_column_array<char, 1>("column_byte")), das::bad_array_size);
    BOOST_CHECK_THROW((ptr->get_column_array<std::string, 2>("column_byte")), das::bad_type);
}

BOOST_AUTO_TEST_CASE(column_byte) {
    char b0[] = {0, 1, 2, 3, 4, 5};
    char b1[] = {7, 8, 9, 10, 11, 12};
    char b2[] = {13, 14, 15, 16, 17, 18};

    char e0[] = {19, 20, 21, 22, 23, 24};
    char e1[] = {25, 26, 27, 28, 29, 30};
    char e2[] = {31, 32, 33, 34, 35, 36};
    char e3[] = {37, 38, 39, 40, 41, 42};

    das::ColumnArray<char, 2> base(das::shape(3));
    base(0).reference(das::Array<char, 2>(b0, das::shape(2, 3), das::neverDeleteData));
    base(1).reference(das::Array<char, 2>(b1, das::shape(2, 3), das::neverDeleteData));
    base(2).reference(das::Array<char, 2>(b2, das::shape(2, 3), das::neverDeleteData));

    das::ColumnArray<char, 2> ext(4);
    ext(0).reference(das::Array<char, 2>(e0, das::shape(2, 3), das::neverDeleteData));
    ext(1).reference(das::Array<char, 2>(e1, das::shape(2, 3), das::neverDeleteData));
    ext(2).reference(das::Array<char, 2>(e2, das::shape(2, 3), das::neverDeleteData));
    ext(3).reference(das::Array<char, 2>(e3, das::shape(2, 3), das::neverDeleteData));

    das::DatabaseConfig::database("test_level2").buffered_data(true);
    test_case("column_byte", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_byte", ptr, db, base, ext);


}

BOOST_AUTO_TEST_CASE(column_short) {
    short b0[] = {0, 1, 2, 3, 4, 5};
    short b1[] = {7, 8};
    short b2[] = {11, 12, 13, 14, 15, 16, 17, 18};

    short e0[] = {19, 20, 21, 22, 23, 24};
    short e1[] = {25, 26, 27, 28, 29, 30};
    short e2[] = {31, 32, 33, 34, 35, 36, 132, 133, 134, 135};
    short e3[] = {37, 38, 39, 40, 41, 42};

    das::ColumnArray<short, 2> base(das::shape(3));
    base(0).reference(das::Array<short, 2>(b0, das::shape(3, 2), das::neverDeleteData));
    base(1).reference(das::Array<short, 2>(b1, das::shape(1, 2), das::neverDeleteData));
    base(2).reference(das::Array<short, 2>(b2, das::shape(4, 2), das::neverDeleteData));

    das::ColumnArray<short, 2> ext(4);
    ext(0).reference(das::Array<short, 2>(e0, das::shape(3, 2), das::neverDeleteData));
    ext(1).reference(das::Array<short, 2>(e1, das::shape(3, 2), das::neverDeleteData));
    ext(2).reference(das::Array<short, 2>(e2, das::shape(5, 2), das::neverDeleteData));
    ext(3).reference(das::Array<short, 2>(e3, das::shape(3, 2), das::neverDeleteData));

    das::DatabaseConfig::database("test_level2").buffered_data(true);
    test_case("column_int16", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_int16", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_int) {
    int b0[] = {0, 1, 2, 3, 4, 5};
    int b1[] = {7, 8};
    int b2[] = {11, 12, 13, 14, 15, 16, 17, 18};

    int e0[] = {19, 20, 21, 22, 23, 24};
    int e1[] = {25, 26, 27, 28, 29, 30};
    int e2[] = {31, 32, 33, 34, 35, 36, 132, 133, 134, 135};
    int e3[] = {37, 38, 39, 40, 41, 42};

    das::ColumnArray<int, 2> base(das::shape(3));
    base(0).reference(das::Array<int, 2>(b0, das::shape(3, 2), das::neverDeleteData));
    base(1).reference(das::Array<int, 2>(b1, das::shape(1, 2), das::neverDeleteData));
    base(2).reference(das::Array<int, 2>(b2, das::shape(4, 2), das::neverDeleteData));

    das::ColumnArray<int, 2> ext(4);
    ext(0).reference(das::Array<int, 2>(e0, das::shape(3, 2), das::neverDeleteData));
    ext(1).reference(das::Array<int, 2>(e1, das::shape(3, 2), das::neverDeleteData));
    ext(2).reference(das::Array<int, 2>(e2, das::shape(5, 2), das::neverDeleteData));
    ext(3).reference(das::Array<int, 2>(e3, das::shape(3, 2), das::neverDeleteData));

    das::DatabaseConfig::database("test_level2").buffered_data(true);
    test_case("column_int32", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_int32", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_long_long) {
    long long b0[] = {0, 1, 2, 3, 4, 5};
    long long b1[] = {7, 8};
    long long b2[] = {11, 12, 13, 14, 15, 16, 17, 18};

    long long e0[] = {19, 20, 21, 22, 23, 24};
    long long e1[] = {25, 26, 27, 28, 29, 30};
    long long e2[] = {31, 32, 33, 34, 35, 36, 132, 133, 134, 135};
    long long e3[] = {37, 38, 39, 40, 41, 42};

    das::ColumnArray<long long> base(das::shape(3));
    base(0).reference(das::Array<long long>(b0, das::shape(6), das::neverDeleteData));
    base(1).reference(das::Array<long long>(b1, das::shape(2), das::neverDeleteData));
    base(2).reference(das::Array<long long>(b2, das::shape(8), das::neverDeleteData));

    das::ColumnArray<long long> ext(4);
    ext(0).reference(das::Array<long long>(e0, das::shape(6), das::neverDeleteData));
    ext(1).reference(das::Array<long long>(e1, das::shape(6), das::neverDeleteData));
    ext(2).reference(das::Array<long long>(e2, das::shape(10), das::neverDeleteData));
    ext(3).reference(das::Array<long long>(e3, das::shape(6), das::neverDeleteData));

    das::DatabaseConfig::database("test_level2").buffered_data(true);
    test_case("column_int64", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_int64", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_float) {
    float b0[] = {0.12, 3.45};
    float b1[] = {7.56, 80.66};
    float b2[] = {11.121314, 15.161718};

    float e0[] = {1920.21, 2223.24};
    float e1[] = {2526, 2728.2930};
    float e2[] = {3143.5, 13233.4135};
    float e3[] = {378.39, 1.42};

    das::ColumnArray<float> base(das::shape(3));
    base(0).reference(das::Array<float>(b0, das::shape(2), das::neverDeleteData));
    base(1).reference(das::Array<float>(b1, das::shape(2), das::neverDeleteData));
    base(2).reference(das::Array<float>(b2, das::shape(2), das::neverDeleteData));

    das::ColumnArray<float> ext(4);
    ext(0).reference(das::Array<float>(e0, das::shape(2), das::neverDeleteData));
    ext(1).reference(das::Array<float>(e1, das::shape(2), das::neverDeleteData));
    ext(2).reference(das::Array<float>(e2, das::shape(2), das::neverDeleteData));
    ext(3).reference(das::Array<float>(e3, das::shape(2), das::neverDeleteData));

    das::DatabaseConfig::database("test_level2").buffered_data(true);
    test_case("column_float32", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_float32", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_string) {
    das::ColumnArray<std::string> base(das::shape(3));
    base(0).resize(3);
    base(1).resize(3);
    base(2).resize(3);

    base(0)(0) = "string01string01string01string01string01string01string01string01\
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
    base(0)(1) = "string2";
    base(0)(2) = "string3";



    das::ColumnArray<std::string> ext(das::shape(4));
    ext(0).resize(3);
    ext(1).resize(3);
    ext(2).resize(3);
    ext(3).resize(3);

    ext(1)(0) = "string chunk 2.1";
    ext(1)(1) = "string chunk 2.1";
    ext(1)(2) = "string chunk 2.1";
    ext(3)(1) = "string chunk 2.1";

    das::DatabaseConfig::database("test_level2").buffered_data(true);
    test_case("column_string", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_string", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_byte_int) {
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

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_byte", ptr, db, base, ext);
}

BOOST_AUTO_TEST_CASE(column_float_byte) {
    char b0[] = {0, 3};
    char b1[] = {7, 66};
    char b2[] = {14, 18};

    char e0[] = {121, 24};
    char e1[] = {2, 20};
    char e2[] = {5, 15};
    char e3[] = {39, 42};

    das::ColumnArray<char> base(das::shape(3));
    base(0).reference(das::Array<char>(b0, das::shape(2), das::neverDeleteData));
    base(1).reference(das::Array<char>(b1, das::shape(2), das::neverDeleteData));
    base(2).reference(das::Array<char>(b2, das::shape(2), das::neverDeleteData));

    das::ColumnArray<char> ext(4);
    ext(0).reference(das::Array<char>(e0, das::shape(2), das::neverDeleteData));
    ext(1).reference(das::Array<char>(e1, das::shape(2), das::neverDeleteData));
    ext(2).reference(das::Array<char>(e2, das::shape(2), das::neverDeleteData));
    ext(3).reference(das::Array<char>(e3, das::shape(2), das::neverDeleteData));

    das::DatabaseConfig::database("test_level2").buffered_data(true);
    test_case("column_float32", ptr, db, base, ext);

    change();
    das::DatabaseConfig::database("test_level2").buffered_data(false);
    test_case("column_float32", ptr, db, base, ext);
}

BOOST_AUTO_TEST_SUITE_END()