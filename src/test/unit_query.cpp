#include "unit_tpl_all.hpp"
#include <vector>
namespace T = das::tpl;

BOOST_AUTO_TEST_SUITE(query_unit_tests)

BOOST_AUTO_TEST_CASE(query_exceptions_T) {
    shared_ptr<T::Database> db;
    BOOST_REQUIRE_NO_THROW(db = T::Database::create("unit_test"));

    T::Transaction t = db->begin();

    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("association > 0"), das::keyword_not_present);

    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("associaton.das_id > 0"), das::association_not_present);

    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id == 'ciao' "), das::bad_type);

    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("association.key_string == 5 "), das::non_compatible_types);


    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id = 'ciao'"), das::incomplete_statement);

    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id = 5"), das::incomplete_statement);

    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id + version"), das::incomplete_statement);

    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id + version"), das::incomplete_statement);

    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id + version > association.key_int32 == association.key_int32 "), das::incomplete_statement);

    BOOST_CHECK_NO_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id + version > association.key_int32 * association.key_int32 "));


    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id + version > association.key_int32 && association.key_int32 "), das::incomplete_statement);


    BOOST_CHECK_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("association.key_boolean"), das::incomplete_statement);


    BOOST_CHECK_NO_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id + version > association.key_int32 * association.key_int32 || association.key_boolean == true"));

    BOOST_CHECK_NO_THROW(
            T::Result<test_association_one_shared> res
            = db->query<test_association_one_shared>("das_id + version > association.key_int32 * association.key_int32 && association.key_boolean == false"));



    t.commit();
}

BOOST_AUTO_TEST_CASE(query_T) {
    shared_ptr<T::Database> db;
    BOOST_REQUIRE_NO_THROW(db = T::Database::create("unit_test"));

    std::string name = "query_test";

    /*
        typedef T::Result<test_association_many_exclusive> Rtype;
        typedef test_association_many_exclusive Atype;
        typedef Atype::association_vector Vec;


        T::Transaction t = db->begin();

        Rtype res = db->query<test_association_many_exclusive>("name == '" + name + "'");
        for (Rtype::iterator it = res.begin(); it != res.end(); ++it) {
            Vec v = it->association();
            for (Vec::iterator vt = v.begin(); vt != v.end(); ++vt)
                db->erase(*vt);
            db->erase(it.load());
        }
    
     */

    typedef std::vector<shared_ptr<test_keywords> > MyVec;

    T::Transaction t = db->begin();

    T::Result<test_keywords> res = db->query<test_keywords>("name.startsWith('" + name + "')");
    for (T::Result<test_keywords>::iterator it = res.begin(); it != res.end(); ++it) {
        db->erase(it.load());
    }
    
    t.commit();

    MyVec vec;
    vec.push_back(test_keywords::create(name+"0","unit_test"));
    vec.push_back(test_keywords::create(name+"1","unit_test"));
    vec.push_back(test_keywords::create(name+"2","unit_test"));
    vec.push_back(test_keywords::create(name+"3","unit_test"));
    vec.push_back(test_keywords::create(name+"4","unit_test"));
    
    vec[0]->key_int32(10);    vec[0]->key_float32(5);       vec[0]->key_float64(5);
    vec[1]->key_int32(10);    vec[1]->key_float32(5);       vec[1]->key_float64(5);
    vec[2]->key_int32(10);    vec[2]->key_float32(5);       vec[2]->key_float64(5);
    vec[3]->key_int32(10);    vec[3]->key_float32(5.1);     vec[3]->key_float64(8.55);
    vec[4]->key_int32(10);    vec[4]->key_float32(5.0001);  vec[4]->key_float64(8.55);
    
    t = db->begin();
    
    for(MyVec::iterator it=vec.begin(); it != vec.end(); ++it)
        db->persist(*it);

    T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00");
    BOOST_CHECK_EQUAL(res2.size(),3);

    T::Result<test_keywords> res3 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 * key_float64  == 35.00");
    BOOST_CHECK_EQUAL(res3.size(),3);
 
    T::Result<test_keywords> res4 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32  == 25.00 || key_float64 == 8.5500");
    BOOST_CHECK_EQUAL(res4.size(),2);
    
    vec[2]->key_float64(8.55);
    
    T::Result<test_keywords> res5 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32  == 25.00 || key_float64 == 8.5500");
    BOOST_CHECK_EQUAL(res5.size(),3);
    
    t.commit();
}

BOOST_AUTO_TEST_CASE(query_ord_T) {
    typedef std::vector<shared_ptr<test_keywords> > MyVec;
    shared_ptr<T::Database> db;
    BOOST_REQUIRE_NO_THROW(db = T::Database::create("unit_test"));

    std::string name = "query_testB";


    typedef std::vector<shared_ptr<test_keywords> > MyVec;

    T::Transaction t = db->begin();

    BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id"),
            das::bad_ordering_clause);
    
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id ascendente"),
            das::bad_ordering_clause ); 
        
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id, key_int32"),
            das::bad_ordering_clause );
        
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id , key_int32"),
            das::bad_ordering_clause );
        
         
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id ,"),
            das::bad_ordering_clause );       
         
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id,"),
            das::bad_ordering_clause ); 
   
          
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id asc,key_INT32 des"),
            das::bad_ordering_clause ); 
           
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","id asc, key_int32 des, key_int64 asc, key_float64 desc"),
            das::bad_ordering_clause ); 
            
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id asc, key_int32 des, key_int64 asc, float64 desc"),
            das::bad_ordering_clause ); 
             
        BOOST_CHECK_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("name.startsWith('" + name + "') && key_int32 + key_float32 == 15.00","das_id asc, key_int32 des, key asc, key_float64 desc"),
            das::bad_ordering_clause );
        
        BOOST_CHECK_NO_THROW(
            T::Result<test_keywords> res2 = db->query<test_keywords>("das_id > 0","das_id asc, key_int32 des, key_int64 asc, key_float64 desc")
                );
        
        
    t.commit();
}

BOOST_AUTO_TEST_SUITE_END()
