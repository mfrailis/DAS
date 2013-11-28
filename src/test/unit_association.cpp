#include "unit_tpl_all.hpp"
using namespace std;
namespace D = das::tpl;

template<typename T>
bool persist_check(const shared_ptr<T> &obj) {
    return obj->das_id() != 0;
}

template<typename T>
bool persist_check(const vector<shared_ptr<T> > &vec) {
    for (typename vector < shared_ptr<T> >::const_iterator it = vec.begin(); it != vec.end(); ++it)
        if ((*it)->das_id() == 0)
            return false;

    return true;
}

template<typename T>
bool compare(const shared_ptr<T> &obj1, const shared_ptr<T> &obj2) {
    return obj1 == obj2;
}

template<typename T>
bool compare(const vector<shared_ptr<T> > &v1, const vector<shared_ptr<T> >&v2) {
    if (v1.size() != v2.size())
        return false;

    for (typename vector < shared_ptr<T> >::const_iterator it = v1.begin(); it != v1.end(); ++it)
        if (find(v2.begin(), v2.end(), *it) == v2.end())
            return false;

    return true;
}

template<typename T>
T
find_id(T first, T last, long long id) {
    T it = first;
    while (it != last) {
        if ((*it)->das_id() == id)
            break;
        else
            ++it;
    }
    return it;
}

template<typename T, typename U>
bool compare_id(const shared_ptr<T> &obj1, const shared_ptr<U> &obj2) {
    return obj1->das_id() == obj2->das_id() && obj1->type_name() ==  obj2->type_name();
}

template<typename T, typename U>
bool compare_id(const vector<shared_ptr<T> > &v1, const vector<shared_ptr<U> >&v2) {
    if (v1.size() != v2.size())
        return false;

    for (typename vector < shared_ptr<T> >::const_iterator it = v1.begin(); it != v1.end(); ++it)
        if (find_id(v2.begin(), v2.end(), (*it)->das_id()) == v2.end())
            return false;

    return true;
}

template<typename T>
shared_ptr<T>
retrive(const shared_ptr<D::Database> &db, long long id) {
    shared_ptr<T> ptr;
    BOOST_REQUIRE_NO_THROW({
        D::Transaction t = db->begin();
        ptr = db->load<T>(id);
        t.commit();
    });
    BOOST_REQUIRE(ptr);
    BOOST_REQUIRE_NE(0, ptr->das_id());
    return ptr;
}

template<typename T, typename Y>
void create_association(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr,
        Y &assoc) {

    ptr->association(assoc);
    BOOST_REQUIRE_NO_THROW({
        D::Transaction t = db->begin();
        db->persist(ptr);
        t.commit();
        Y assoc2 = ptr->association();
        BOOST_CHECK(persist_check(ptr));;
        BOOST_CHECK(persist_check(assoc));
        BOOST_CHECK(compare(assoc, assoc2));
    });
}

template<typename T, typename Y>
void read_update_association(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr,
        shared_ptr<Y> &old_assoc,
        shared_ptr<DasObject> new_assoc) {
    shared_ptr<Y> assoc;
    shared_ptr<DasObject> assoc2;

    BOOST_CHECK_THROW(assoc = ptr->association(), das::not_in_managed_context);
    BOOST_CHECK_THROW(assoc2 = ptr->get_associated_object("association"), das::not_in_managed_context); 
    BOOST_REQUIRE_NO_THROW({
        D::Transaction t = db->begin();
        assoc = ptr->association();
        t.commit();
    });
    BOOST_CHECK(compare_id(old_assoc, assoc));
    BOOST_REQUIRE_NO_THROW({
        assoc2 = ptr->get_associated_object("association");
    });
    BOOST_CHECK(compare_id(assoc, assoc2));
    BOOST_REQUIRE_NO_THROW({
        D::Transaction t = db->begin();
        db->attach(ptr);
        ptr->set_associated_object("association",new_assoc);
        t.commit();
    });
    BOOST_CHECK(persist_check(new_assoc));
}

template<typename T, typename Y>
void read_update_association(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr,
        std::vector<shared_ptr<Y> > &old_assoc,
        std::vector<shared_ptr<Y> > &new_assoc) {
    std::vector<shared_ptr<Y> > assoc;
    std::vector<shared_ptr<DasObject> > assoc2;
    
    std::vector<shared_ptr<DasObject> > das_new_assoc;
    for(typename std::vector<shared_ptr<Y> >::iterator it = new_assoc.begin();
            it != new_assoc.end();
            ++it)
        das_new_assoc.push_back(*it);

    BOOST_CHECK_THROW(assoc = ptr->association(), das::not_in_managed_context);
    BOOST_CHECK_THROW(assoc2 = ptr->get_associated_objects("association"), das::not_in_managed_context); 
    BOOST_REQUIRE_NO_THROW({
        D::Transaction t = db->begin();
        assoc = ptr->association();
        t.commit();
    });
    BOOST_CHECK(compare_id(old_assoc, assoc));
    BOOST_REQUIRE_NO_THROW({
        assoc2 = ptr->get_associated_objects("association");
    });
    BOOST_CHECK(compare_id(assoc, assoc2));
    BOOST_REQUIRE_NO_THROW({
        D::Transaction t = db->begin();
        db->attach(ptr);
        ptr->set_associated_objects("association",das_new_assoc);
        t.commit();
    });
    BOOST_CHECK(persist_check(das_new_assoc));
}

template<typename T, typename Y>
void check_update(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr,
        Y &old_assoc,
        Y &new_assoc) {
    Y assoc, assoc2;
    BOOST_CHECK_THROW({assoc = ptr->association();}, das::not_in_managed_context);
    BOOST_REQUIRE_NO_THROW({
        D::Transaction t = db->begin();
        assoc = ptr->association();
        t.commit();
    });

    BOOST_CHECK(!compare_id(assoc, old_assoc));
    BOOST_CHECK(compare_id(assoc, new_assoc));
    BOOST_REQUIRE_NO_THROW({assoc2 = ptr->association();});
    BOOST_CHECK(compare(assoc, assoc2));
}

template<typename T, typename Y>
void restore_old(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr) {
    Y old_assoc, new_assoc;
    BOOST_REQUIRE_NO_THROW({
        old_assoc = ptr->association();
        D::Transaction t = db->begin();
        shared_ptr<T> ptr1 = db->load<T>(ptr->das_id());
        ptr1->association(old_assoc);
        t.commit();

        t = db->begin();
        shared_ptr<T> ptr2 = db->load<T>(ptr->das_id());
        new_assoc = ptr2->association();
        t.commit();
    });
    BOOST_CHECK(compare_id(old_assoc, new_assoc));
}

BOOST_AUTO_TEST_SUITE(associations_unit_tests)

BOOST_AUTO_TEST_CASE(one_shared) {
    shared_ptr<D::Database> db;
    shared_ptr<test_association_one_shared> ptr;
    shared_ptr<test_associated_one_shared> assoc, assoc2;

    BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));
    BOOST_REQUIRE_NO_THROW(
            ptr = test_association_one_shared::create("test_00", "test_level2"));
    BOOST_REQUIRE_NO_THROW(
            assoc = test_associated_one_shared::create("test_assoc_A", "test_level2"));
    BOOST_REQUIRE_NO_THROW(
            assoc2 = test_associated_one_shared::create("test_assoc_B", "test_level2"));

    create_association(db, ptr, assoc);
    shared_ptr<test_association_one_shared> ptr1 =
            retrive<test_association_one_shared>(db, ptr->das_id());

    read_update_association(db, ptr1, assoc, assoc2);

    shared_ptr<test_association_one_shared> ptr2 =
            retrive<test_association_one_shared>(db, ptr->das_id());

    check_update(db, ptr2, assoc, assoc2);
    restore_old<test_association_one_shared, shared_ptr<test_associated_one_shared> >(db, ptr);
}

BOOST_AUTO_TEST_CASE(one_exclusive) {
    shared_ptr<D::Database> db;
    shared_ptr<test_association_one_exclusive> ptr;
    shared_ptr<test_associated_one_exclusive> assoc, assoc2;

    BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));
    BOOST_REQUIRE_NO_THROW(
            ptr = test_association_one_exclusive::create("test_00", "test_level2"));
    BOOST_REQUIRE_NO_THROW(
            assoc = test_associated_one_exclusive::create("test_assoc_A", "test_level2"));
    BOOST_REQUIRE_NO_THROW(
            assoc2 = test_associated_one_exclusive::create("test_assoc_B", "test_level2"));
    create_association(db, ptr, assoc);
    shared_ptr<test_association_one_exclusive> ptr1 =
            retrive<test_association_one_exclusive>(db, ptr->das_id());

    read_update_association(db, ptr1, assoc, assoc2);

    shared_ptr<test_association_one_exclusive> ptr2 =
            retrive<test_association_one_exclusive>(db, ptr->das_id());

    check_update(db, ptr2, assoc, assoc2);
    restore_old<test_association_one_exclusive, shared_ptr<test_associated_one_exclusive> >(db, ptr);

}

BOOST_AUTO_TEST_CASE(many_shared) {
    shared_ptr<D::Database> db;
    shared_ptr<test_association_many_shared> ptr;
    vector< shared_ptr<test_associated_many_shared> > assoc;
    vector<shared_ptr<test_associated_many_shared> > assoc2;

    BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));
    BOOST_REQUIRE_NO_THROW(
            ptr = test_association_many_shared::create("test_00", "test_level2"));

    for (int i = 0; i < 5; ++i)
        BOOST_REQUIRE_NO_THROW(
            assoc.push_back(
            test_associated_many_shared::create("test_assoc_A", "test_level2")));

    for (int i = 0; i < 3; ++i)
        BOOST_REQUIRE_NO_THROW(
            assoc2.push_back(
            test_associated_many_shared::create("test_assoc_B", "test_level2")));

    create_association(db, ptr, assoc);

    shared_ptr<test_association_many_shared> ptr1 =
            retrive<test_association_many_shared>(db, ptr->das_id());

    read_update_association(db, ptr1, assoc, assoc2);

    shared_ptr<test_association_many_shared> ptr2 =
            retrive<test_association_many_shared>(db, ptr->das_id());

    check_update(db, ptr2, assoc, assoc2);

    restore_old<test_association_many_shared, vector<shared_ptr<test_associated_many_shared> > >(db, ptr);
}

BOOST_AUTO_TEST_CASE(many_exclusive) {
    shared_ptr<D::Database> db;
    shared_ptr<test_association_many_exclusive> ptr;
    vector< shared_ptr<test_associated_many_exclusive> > assoc;
    vector<shared_ptr<test_associated_many_exclusive> > assoc2;
  
    BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));
    BOOST_REQUIRE_NO_THROW(
            ptr = test_association_many_exclusive::create("test_00", "test_level2"));

    for (int i = 0; i < 5; ++i)
        BOOST_REQUIRE_NO_THROW(
            assoc.push_back(
            test_associated_many_exclusive::create("test_assoc_A", "test_level2")));
    for (int i = 0; i < 3; ++i)
        BOOST_REQUIRE_NO_THROW(
            assoc2.push_back(
            test_associated_many_exclusive::create("test_assoc_B", "test_level2")));

    create_association(db, ptr, assoc);

    shared_ptr<test_association_many_exclusive> ptr1 =
            retrive<test_association_many_exclusive>(db, ptr->das_id());

    read_update_association(db, ptr1, assoc, assoc2);

    shared_ptr<test_association_many_exclusive> ptr2 =
            retrive<test_association_many_exclusive>(db, ptr->das_id());

    check_update(db, ptr2, assoc, assoc2);

    restore_old<test_association_many_exclusive, vector<shared_ptr<test_associated_many_exclusive> > >(db, ptr);
}

BOOST_AUTO_TEST_SUITE_END()