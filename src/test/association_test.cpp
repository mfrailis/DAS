#include <iostream>
#include <exception>
#include <das/tpl/database.hpp>
#include <das/transaction.hpp>
#include <das/ddl/types.hpp>

#include "tpl/database.hpp"

using namespace std;
namespace D = das::tpl;

long long create_one_shared(const shared_ptr<D::Database> &db,
        long long &assoc_id) {
    shared_ptr<test_association_one_shared> ptr =
            test_association_one_shared::create("test_0", "test_level2");

    shared_ptr<test_associated_one_shared> assoc =
            test_associated_one_shared::create("test_assoc_0", "test_level2");

    ptr->one_shared(assoc);
    long long id;
    try {
        D::Transaction t = db->begin();
        cout << "Persisting association one shared... ";
        id = db->persist(ptr);
        t.commit();
        cout << "done." << endl;

        shared_ptr<test_associated_one_shared> assoc2 =
                ptr->one_shared();

        if (assoc2->das_id() == 0) {
            cout << "Failed to persist associated object" << endl;
            return 0;
        }
        assoc_id = assoc->das_id();
        if (assoc != assoc2) {
            cout << "Failed: multiple copies of the associated object" << endl;
            return 0;
        }
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 0;
    }
    return id;
}

long long read_update_one_shared(const shared_ptr<D::Database> &db,
        long long id,
        long long assoc_id) {
    shared_ptr<test_association_one_shared> ptr;
    shared_ptr<test_associated_one_shared> assoc, assoc2, assoc_new;
    bool throws = false;
    try {
        D::Transaction t = db->begin();
        cout << "Retriving association object... ";
        ptr = db->load<test_association_one_shared>(id);
        t.commit();
        cout << "done." << endl;

    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 0;
    }

    try {
        cout << "Testing exception while retriving associated object... ";
        assoc = ptr->one_shared();
    } catch (const das::not_in_managed_context &e) {
        throws = true;
        cout << "done." << endl;
    }

    if (!throws) return 0;

    try {
        D::Transaction t = db->begin();
        cout << "Retriving associated object... ";
        assoc = ptr->one_shared();
        t.commit();
        cout << "done." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 0;
    }
    
    if(assoc->das_id() != assoc_id){
        cout << "Failed: associated id mismatch" << endl;
        return 0;  
    }

    try {
        cout << "Testing exception while re-retriving associated object... ";
        assoc2 = ptr->one_shared();
        cout << "done." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 0;
    }

    if (assoc != assoc2) {
        cout << "Failed: multiple copies of the associated object" << endl;
        return 0;
    }
    
    assoc_new = test_associated_one_shared::create("test_assoc_1", "test_level2");
    
    try{
        ptr->one_shared(assoc_new);
        D::Transaction t = db->begin();
        cout << "Updating association... ";
        db->attach(ptr);
        t.commit();
        cout << "done. " << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 0;
    }

    return assoc_new->das_id();
}

int check_update_one_shared(const shared_ptr<D::Database> &db,
        long long id,
        long long assoc_id,
        long long assoc_new_id){
    shared_ptr<test_association_one_shared> ptr;
    shared_ptr<test_associated_one_shared> assoc, assoc2, assoc_new;
    bool throws = false;
    try {
        D::Transaction t = db->begin();
        cout << "Retriving association object... ";
        ptr = db->load<test_association_one_shared>(id);
        t.commit();
        cout << "done." << endl;

    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 1;
    }

    try {
        cout << "Testing exception while retriving associated object... ";
        assoc = ptr->one_shared();
    } catch (const das::not_in_managed_context &e) {
        throws = true;
        cout << "done." << endl;
    }

    if (!throws) return 2;

    try {
        D::Transaction t = db->begin();
        cout << "Retriving associated object... ";
        assoc = ptr->one_shared();
        t.commit();
        cout << "done." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 3;
    }
    
    if(assoc->das_id() == assoc_id){
        cout << "Failed: old association retrived" << endl;
        return 7;
    }
    
    if(assoc->das_id() != assoc_new_id){
        cout << "Failed: wrong association retrived" << endl;
        return 8;
    }
    
    if(assoc->das_id() != assoc_id){
        cout << "Failed: associated id mismatch" << endl;
        return 4;  
    }

    try {
        cout << "Testing exception while re-retriving associated object... ";
        assoc2 = ptr->one_shared();
        cout << "done." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 5;
    }

    if (assoc != assoc2) {
        cout << "Failed: multiple copies of the associated object" << endl;
        return 6;
    }
    
    return 0;
        
}

int main() {
    shared_ptr<D::Database> db = D::Database::create("test_level2");
    long long assoc_id = 0;
    cout << "CREATING ASSOCIATION" << endl;
    long long id = create_one_shared(db, assoc_id);

    if (id == 0) return 1;
    cout << "READING/UPDATING ASSOCIATION" << endl;
    long long assoc_new_id = read_update_one_shared(db, id, assoc_id);
    if (assoc_new_id == 0) return 1;
    cout << assoc_new_id << endl;
    
    cout << "CHECKING UPDATED ASSOCIATION" << endl;
    return check_update_one_shared(db,id,assoc_id,assoc_new_id);
}