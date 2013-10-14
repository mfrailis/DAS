#include <iostream>
#include <sstream>
#include <exception>
#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"
#include <vector>
#include <algorithm>
#include "tpl/database.hpp"
#include "ddl/types/ddl_test_associated_one_exclusive.hpp"

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

template<typename T>
bool compare_id(const shared_ptr<T> &obj1, const shared_ptr<T> &obj2) {
    return obj1->das_id() == obj2->das_id();
}

template<typename T>
bool compare_id(const vector<shared_ptr<T> > &v1, const vector<shared_ptr<T> >&v2) {
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
    try {
        D::Transaction t = db->begin();
        ptr = db->load<T>(id);
        t.commit();
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        ptr.reset();
    }
    return ptr;
}

template<typename T, typename Y>
int create_association(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr,
        Y &assoc) {

    ptr->association(assoc);
    try {
        D::Transaction t = db->begin();
        cout << "Persisting association... ";
        db->persist(ptr);
        t.commit();
        cout << "ok." << endl;

        Y assoc2 = ptr->association();

        if (!persist_check(ptr)) return 1;
        if (!persist_check(assoc)) return 2;
        if (!compare(assoc, assoc2)) return 3;

    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 4;
    }
    return 0;
}

template<typename T, typename Y>
int read_update_association(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr,
        Y &old_assoc,
        Y &new_assoc) {
    Y assoc, assoc2;
    bool throws = false;

    try {
        cout << "Testing exception while retriving associated object... ";
        assoc = ptr->association();
    } catch (const das::not_in_managed_context &e) {
        throws = true;
        cout << "ok." << endl;
    }

    if (!throws) return 1;

    try {
        D::Transaction t = db->begin();
        cout << "Retriving associated object... ";
        assoc = ptr->association();
        t.commit();
        cout << "ok." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 2;
    }

    cout << "Comparing ids... ";
    if (!compare_id(old_assoc, assoc)) {
        cout << "fail." << endl;
        return 3;
    }
    cout << "ok." << endl;

    try {
        cout << "Testing exception while re-retriving associated object... ";
        assoc2 = ptr->association();
        cout << "ok." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 4;
    }

    cout << "Looking for multiple copies... ";
    if (!compare_id(assoc, assoc2)) {
        cout << "fail" << endl;
        return 5;
    }
    cout << "ok." << endl;

    try {
        D::Transaction t = db->begin();
        cout << "Updating association... ";
        db->attach(ptr);
        ptr->association(new_assoc);
        t.commit();
        cout << "ok. " << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 6;
    }

    cout << "Cheking new association persistance... ";

    if (!persist_check(new_assoc)) {
        cout << "fail." << endl;
        return 7;
    }
    cout << "ok." << endl;
    return 0;
}

template<typename T, typename Y>
int check_update(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr,
        Y &old_assoc,
        Y &new_assoc) {
    Y assoc, assoc2;
    bool throws = false;

    try {
        cout << "Testing exception while retriving associated object... ";
        assoc = ptr->association();
    } catch (const das::not_in_managed_context &e) {
        throws = true;
        cout << "ok." << endl;
    }

    if (!throws) {
        cout << "fail." << endl;
        return 2;
    }

    try {
        D::Transaction t = db->begin();
        cout << "Retriving associated object... ";
        assoc = ptr->association();
        t.commit();
        cout << "ok." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 3;
    }


    cout << "Comparing associated objects... ";
    if (compare_id(assoc, old_assoc)) {
        cout << "fail." << endl;
        return 7;
    }
    if (!compare_id(assoc, new_assoc)) {
        cout << "fail." << endl;
        return 8;
    }
    cout << "ok." << endl;

    try {
        cout << "Testing exception while re-retriving associated object... ";
        assoc2 = ptr->association();
        cout << "ok." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 9;
    }

    cout << "Comparing associated objects... ";
    if (!compare(assoc, assoc2)) {
        cout << "fail." << endl;
        return 10;
    }
    cout << "ok." << endl;
    return 0;
}

template<typename T, typename Y>
int restore_old(const shared_ptr<D::Database> &db,
        shared_ptr<T> &ptr) {
    Y old_assoc, new_assoc;
    try {
        D::Transaction t = db->begin();
        cout << "Restoring association... ";
        db->attach(ptr);
        t.commit();
        cout << "ok." << endl;
        old_assoc = ptr->association();

        t = db->begin();
        cout << "Reloading association... ";
        shared_ptr<T> ptr1 = db->load<T>(ptr->das_id());
        new_assoc = ptr1->association();
        t.commit();
        cout << "ok." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 9;
    }

    cout << "Comparing associated objects... ";
    if (!compare_id(old_assoc, new_assoc)) {
        cout << "fail." << endl;
        return 7;
    }
    cout << "ok." << endl;

    return 0;
}

template<typename T>
bool same_n(size_t size, shared_ptr<T> &assoc) {
    return size == 1;
}

template<typename T>
bool same_n(size_t size, vector<shared_ptr<T> > &assoc) {
    return size == assoc.size();
}

template<typename T, typename Y>
bool reverse_check(const shared_ptr<D::Database> &db, shared_ptr<T> &ptr, Y &assoc) {
    cout << "Checking reverse association... ";
    stringstream query;
    query << "association.das_id > 0 && das_id == " << ptr->das_id();
    D::Transaction t = db->begin();
    D::Result<T> r = db->query<T>(query.str());
    size_t size = r.size();
    t.commit();

    bool res = same_n(size, assoc);
    if (res)
        cout << "ok." << endl;
    else
        cout << "fail." << endl;
    return res;
}

int main() {
    shared_ptr<D::Database> db = D::Database::create("test_level2");
    cout << endl << "ONE SHARED" << endl;
    {
        shared_ptr<test_association_one_shared> ptr =
                test_association_one_shared::create("test_00", "test_level2");

        shared_ptr<test_associated_one_shared> assoc =
                test_associated_one_shared::create("test_assoc_A", "test_level2");

        shared_ptr<test_associated_one_shared> assoc2 =
                test_associated_one_shared::create("test_assoc_B", "test_level2");

        if (create_association(db, ptr, assoc))
            return 1;

        shared_ptr<test_association_one_shared> ptr1 = retrive<test_association_one_shared>(db, ptr->das_id());

        if (!ptr1) return 2;
        if (read_update_association(db, ptr1, assoc, assoc2))
            return 3;

        shared_ptr<test_association_one_shared> ptr2 = retrive<test_association_one_shared>(db, ptr->das_id());
        if (!ptr2) return 2;
        if (check_update(db, ptr2, assoc, assoc2))
            return 4;

        if (restore_old<test_association_one_shared, shared_ptr<test_associated_one_shared> >(db, ptr))
            return 5;

        if (!reverse_check(db, ptr, assoc))
            return 6;

    }
    cout << endl << "ONE EXCLUSIVE" << endl;
    {
        shared_ptr<test_association_one_exclusive> ptr =
                test_association_one_exclusive::create("test_00", "test_level2");

        shared_ptr<test_associated_one_exclusive> assoc =
                test_associated_one_exclusive::create("test_assoc_A", "test_level2");

        shared_ptr<test_associated_one_exclusive> assoc2 =
                test_associated_one_exclusive::create("test_assoc_B", "test_level2");

        if (create_association(db, ptr, assoc))
            return 1;

        shared_ptr<test_association_one_exclusive> ptr1 = retrive<test_association_one_exclusive>(db, ptr->das_id());
        if (!ptr1) return 2;
        if (read_update_association(db, ptr1, assoc, assoc2))
            return 3;

        shared_ptr<test_association_one_exclusive> ptr2 = retrive<test_association_one_exclusive>(db, ptr->das_id());
        if (!ptr2) return 2;
        if (check_update(db, ptr2, assoc, assoc2))
            return 4;

        if (restore_old<test_association_one_exclusive, shared_ptr<test_associated_one_exclusive> >(db, ptr))
            return 5;

        if (!reverse_check(db, ptr, assoc))
            return 6;

    }
    cout << endl << "MANY SHARED" << endl;
    {
        shared_ptr<test_association_many_shared> ptr =
                test_association_many_shared::create("test_00", "test_level2");

        vector< shared_ptr<test_associated_many_shared> > assoc;
        for (int i = 0; i < 5; ++i)
            assoc.push_back(test_associated_many_shared::create("test_assoc_A", "test_level2"));

        vector<shared_ptr<test_associated_many_shared> > assoc2;
        for (int i = 0; i < 3; ++i)
            assoc2.push_back(test_associated_many_shared::create("test_assoc_B", "test_level2"));

        if (create_association(db, ptr, assoc))
            return 1;

        shared_ptr<test_association_many_shared> ptr1 = retrive<test_association_many_shared>(db, ptr->das_id());
        if (!ptr1) return 2;
        if (read_update_association(db, ptr1, assoc, assoc2))
            return 3;

        shared_ptr<test_association_many_shared> ptr2 = retrive<test_association_many_shared>(db, ptr->das_id());
        if (!ptr2) return 2;
        if (check_update(db, ptr2, assoc, assoc2))
            return 4;

        if (restore_old<test_association_many_shared, vector<shared_ptr<test_associated_many_shared> > >(db, ptr))
            return 5;

        if (!reverse_check(db, ptr, assoc))
            return 6;
    }

    cout << endl << "MANY EXCLUSIVE" << endl;
    {
        shared_ptr<test_association_many_exclusive> ptr =
                test_association_many_exclusive::create("test_00", "test_level2");

        vector< shared_ptr<test_associated_many_exclusive> > assoc;
        for (int i = 0; i < 5; ++i)
            assoc.push_back(test_associated_many_exclusive::create("test_assoc_A", "test_level2"));

        vector<shared_ptr<test_associated_many_exclusive> > assoc2;
        for (int i = 0; i < 3; ++i)
            assoc2.push_back(test_associated_many_exclusive::create("test_assoc_B", "test_level2"));

        if (create_association(db, ptr, assoc))
            return 1;

        shared_ptr<test_association_many_exclusive> ptr1 = retrive<test_association_many_exclusive>(db, ptr->das_id());
        if (!ptr1) return 2;
        if (read_update_association(db, ptr1, assoc, assoc2))
            return 3;

        shared_ptr<test_association_many_exclusive> ptr2 = retrive<test_association_many_exclusive>(db, ptr->das_id());
        if (!ptr2) return 2;
        if (check_update(db, ptr2, assoc, assoc2))
            return 4;
        {
            D::Transaction t = db->begin();
            db->attach(ptr);
            t.commit();
        }
        if (restore_old<test_association_many_exclusive, vector<shared_ptr<test_associated_many_exclusive> > >(db, ptr))
            return 5;

        if (!reverse_check(db, ptr, assoc))
            return 6;

    }

    return 0;
}
