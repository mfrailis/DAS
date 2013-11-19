#include "unit_tpl_all.hpp"
#include "internal/array.hpp"

namespace D = das::tpl;

template<typename T>
void
append(das::Array<T>& array,
        const std::string col_name,
        shared_ptr<test_columns>& ptr,
        shared_ptr<D::Database>& db) {
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
    BOOST_REQUIRE_NO_THROW(array.reference(ptr->get_column(col_name, 0, s)));
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

BOOST_AUTO_TEST_SUITE(column_data_unit_tests)

BOOST_AUTO_TEST_CASE(column_char) {
    das::Array<char> base;

}

BOOST_AUTO_TEST_SUITE_END()