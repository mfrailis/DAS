#include "unit_tpl_all.hpp"

namespace D = das::tpl;

struct MetadataFixture {

    MetadataFixture() {
        key_byte_ = 127;
        key_int16_ = 32767;
        key_int32_ = 2147483647;
        key_int64_ = 8589934592;
        key_float32_ = 2.718281828;
        key_float64_ = 1.144729886;
        key_boolean_ = true;
        key_char_ = '9';
        key_string_ = "this is a 'test' string";
        key_text_ = "this is a 'test' text";
        BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));
        BOOST_REQUIRE_NO_THROW(ptr = test_keywords::create("keywords_test_0", "test_level2"));
    }

    ~MetadataFixture() {
    }

    void update() {
        key_byte_ = 100;
        key_int16_ = 100;
        key_int32_ = 100;
        key_int64_ = 100;
        key_float32_ = 1.732050808;
        key_float64_ = 2.645751311;
        key_boolean_ = false;
        key_char_ = 'f';
        key_string_ = "this is an updated 'test' string";
        key_text_ = "this is a updated 'test' text";
    }
    
    void restore(){
        key_byte_ = 127;
        key_int16_ = 32767;
        key_int32_ = 2147483647;
        key_int64_ = 8589934592;
        key_float32_ = 2.718281828;
        key_float64_ = 1.144729886;
        key_boolean_ = true;
        key_char_ = '9';
        key_string_ = "this is a 'test' string";
        key_text_ = "this is a 'test' text";      
    }

    signed char key_byte_;
    short key_int16_;
    int key_int32_;
    long long key_int64_;
    float key_float32_;
    double key_float64_;
    bool key_boolean_;
    char key_char_;
    std::string key_string_;
    std::string key_text_;

    shared_ptr<D::Database> db;
    shared_ptr<test_keywords> ptr;
};

BOOST_FIXTURE_TEST_SUITE(metadata, MetadataFixture)

BOOST_AUTO_TEST_CASE(create_metadata) {
    long long id = 0;

    ptr->key_byte(key_byte_);
    ptr->key_int16(key_int16_);
    ptr->key_int32(key_int32_);
    ptr->key_int64(key_int64_);
    ptr->key_float32(key_float32_);
    ptr->key_float64(key_float64_);
    ptr->key_boolean(key_boolean_);
    ptr->key_char(key_char_);
    ptr->key_string(key_string_);
    ptr->key_text(key_text_);

    BOOST_CHECK_NO_THROW({
        D::Transaction t = db->begin();
        id = db->persist(ptr);
        t.commit();
    });

    BOOST_CHECK_NE(id, 0);
}

BOOST_AUTO_TEST_CASE(read_metadata) {
    BOOST_CHECK_NO_THROW({
        D::Transaction t = db->begin();
        ptr = db->load<test_keywords>("keywords_test_0");
        t.commit();
    });

    BOOST_CHECK_EQUAL(ptr->key_byte(), key_byte_);
    BOOST_CHECK_EQUAL(ptr->key_int16(), key_int16_);
    BOOST_CHECK_EQUAL(ptr->key_int32(), key_int32_);
    BOOST_CHECK_EQUAL(ptr->key_int64(), key_int64_);
    BOOST_CHECK_EQUAL(ptr->key_float32(), key_float32_);
    BOOST_CHECK_EQUAL(ptr->key_float64(), key_float64_);
    BOOST_CHECK_EQUAL(ptr->key_boolean(), key_boolean_);
    BOOST_CHECK_EQUAL(ptr->key_char(), key_char_);
    BOOST_CHECK_EQUAL(ptr->key_string(), key_string_);
    BOOST_CHECK_EQUAL(ptr->key_text(), key_text_);

}

BOOST_AUTO_TEST_CASE(update_metadata) {
    BOOST_CHECK_NO_THROW({
        D::Transaction t = db->begin();
        ptr = db->load<test_keywords>("keywords_test_0");
        t.commit();
    });

    update();

    ptr->key_byte(key_byte_);
    ptr->key_int16(key_int16_);
    ptr->key_int32(key_int32_);
    ptr->key_int64(key_int64_);
    ptr->key_float32(key_float32_);
    ptr->key_float64(key_float64_);
    ptr->key_boolean(key_boolean_);
    ptr->key_char(key_char_);
    ptr->key_string(key_string_);
    ptr->key_text(key_text_);

    BOOST_CHECK_NO_THROW({
        D::Transaction t = db->begin();
        db->attach(ptr);
        t.commit();
    });

}

BOOST_AUTO_TEST_CASE(check_metadata) {
    BOOST_CHECK_NO_THROW({
        D::Transaction t = db->begin();
        ptr = db->load<test_keywords>("keywords_test_0");
        t.commit();
    });

    update();

    BOOST_CHECK_EQUAL(ptr->key_byte(), key_byte_);
    BOOST_CHECK_EQUAL(ptr->key_int16(), key_int16_);
    BOOST_CHECK_EQUAL(ptr->key_int32(), key_int32_);
    BOOST_CHECK_EQUAL(ptr->key_int64(), key_int64_);
    BOOST_CHECK_EQUAL(ptr->key_float32(), key_float32_);
    BOOST_CHECK_EQUAL(ptr->key_float64(), key_float64_);
    BOOST_CHECK_EQUAL(ptr->key_boolean(), key_boolean_);
    BOOST_CHECK_EQUAL(ptr->key_char(), key_char_);
    BOOST_CHECK_EQUAL(ptr->key_string(), key_string_);
    BOOST_CHECK_EQUAL(ptr->key_text(), key_text_);
}

BOOST_AUTO_TEST_CASE(check_metadata_polimorphic_interface) {
    BOOST_CHECK_NO_THROW({
        D::Transaction t = db->begin();
        ptr = db->load<test_keywords>("keywords_test_0");
        t.commit();
    });

    update();

    BOOST_CHECK_EQUAL(ptr->key_byte(), ptr->get_key<signed char>("key_byte"));
    BOOST_CHECK_EQUAL(ptr->key_int16(), ptr->get_key<short>("key_int16"));
    BOOST_CHECK_EQUAL(ptr->key_int32(), ptr->get_key<int>("key_int32"));
    BOOST_CHECK_EQUAL(ptr->key_int64(), ptr->get_key<long long>("key_int64"));
    BOOST_CHECK_EQUAL(ptr->key_float32(), ptr->get_key<float>("key_float32"));
    BOOST_CHECK_EQUAL(ptr->key_float64(), ptr->get_key<double>("key_float64"));
    BOOST_CHECK_EQUAL(ptr->key_boolean(), ptr->get_key<bool>("key_boolean"));
    BOOST_CHECK_EQUAL(ptr->key_char(), ptr->get_key<char>("key_char"));
    BOOST_CHECK_EQUAL(ptr->key_string(), ptr->get_key<string>("key_string"));
    BOOST_CHECK_EQUAL(ptr->key_text(), ptr->get_key<string>("key_text"));
    
    restore();
    
    BOOST_CHECK_NO_THROW(ptr->set_key<signed char>("key_byte",key_byte_));
    BOOST_CHECK_NO_THROW(ptr->set_key<short>("key_int16", key_int16_));
    BOOST_CHECK_NO_THROW(ptr->set_key<int>("key_int32" , key_int32_));
    BOOST_CHECK_NO_THROW(ptr->set_key<long long>("key_int64", key_int64_));      
    BOOST_CHECK_NO_THROW(ptr->set_key<float>("key_float32", key_float32_));
    BOOST_CHECK_NO_THROW(ptr->set_key<double>("key_float64", key_float64_));
    BOOST_CHECK_NO_THROW(ptr->set_key<bool>("key_boolean", key_boolean_));
    BOOST_CHECK_NO_THROW(ptr->set_key<char>("key_char", key_char_));
    BOOST_CHECK_NO_THROW(ptr->set_key<string>("key_string", key_string_));
    BOOST_CHECK_NO_THROW(ptr->set_key<string>("key_text", key_text_));
    
    BOOST_CHECK_NO_THROW({
        D::Transaction t = db->begin();
        db->attach(ptr);
        t.commit();
    });
    
    ptr.reset();
    
    BOOST_CHECK_NO_THROW({
        D::Transaction t = db->begin();
        ptr = db->load<test_keywords>("keywords_test_0");
        t.commit();
    });
    
    BOOST_CHECK_EQUAL(ptr->key_byte(), key_byte_);
    BOOST_CHECK_EQUAL(ptr->key_int16(), key_int16_);
    BOOST_CHECK_EQUAL(ptr->key_int32(), key_int32_);
    BOOST_CHECK_EQUAL(ptr->key_int64(), key_int64_);
    BOOST_CHECK_EQUAL(ptr->key_float32(), key_float32_);
    BOOST_CHECK_EQUAL(ptr->key_float64(), key_float64_);
    BOOST_CHECK_EQUAL(ptr->key_boolean(), key_boolean_);
    BOOST_CHECK_EQUAL(ptr->key_char(), key_char_);
    BOOST_CHECK_EQUAL(ptr->key_string(), key_string_);
    BOOST_CHECK_EQUAL(ptr->key_text(), key_text_);
}

BOOST_AUTO_TEST_SUITE_END()


