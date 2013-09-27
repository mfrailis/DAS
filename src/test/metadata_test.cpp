#include <iostream>
#include <exception>
#include <das/tpl/database.hpp>
#include <das/transaction.hpp>
#include <das/ddl/types.hpp>

using namespace std;
namespace D = das::tpl;

signed char key_byte_ = 127;
short key_int16_ = 32767;
int key_int32_ = 2147483647;
long long key_int64_ = 8589934592;
float key_float32_ = 2.718281828;
double key_float64_ = 1.144729886;
bool key_boolean_ = true;
char key_char_ = '9';
std::string key_string_ = "this is a 'test' string";
std::string key_text_ = "this is a 'test' text";

long long create_metadata(const shared_ptr<D::Database> &db) {
    long long id = 0;

    shared_ptr<test_keywords> ptr = test_keywords::create("keywords_test_0", "test_level2");
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

    try {
        D::Transaction t = db->begin();
        cout << "Persisting metadata object... ";
        id = db->persist(ptr);
        t.commit();
        cout << "done." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
    }
    return id;
}

int read_update_metadata(const shared_ptr<D::Database> &db, long long id) {
    shared_ptr<test_keywords> ptr;
    try {
        D::Transaction t = db->begin();
        cout << "Retriving metadata object... ";
        ptr = db->load<test_keywords>(id);
        t.commit();
        cout << "done." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 1;
    }

    cout << "Checking keywords... ";
    if (ptr->key_byte() != key_byte_){
        cout << "byte keyword mismatch: " << ptr->key_byte() << " " << key_byte_<< endl;
        return 2;
    }
    if (ptr->key_int16() != key_int16_){
        cout << "int16 keyword mismatch: " << ptr->key_int16() << " " << key_int16_<< endl;
        return 2;
    }
    if (ptr->key_int32() != key_int32_){
        cout << "int32 keyword mismatch: " << ptr->key_int32() << " " << key_int32_<< endl;
        return 2;
    }
    if (ptr->key_int64() != key_int64_){
        cout << "int64 keyword mismatch: " << ptr->key_int64() << " " << key_int64_<< endl;
        return 2;
    }
    if (ptr->key_float32() != key_float32_){
        cout << "float32 keyword mismatch: " << ptr->key_float32() << " " << key_float32_<< endl;
        return 2;
    }
    if (ptr->key_float64() != key_float64_){
        cout << "float64 keyword mismatch: " << ptr->key_float64() << " " << key_float64_<< endl;
        return 2;
    }
    if (ptr->key_boolean() != key_boolean_){
        cout << "boolean keyword mismatch: " << ptr->key_boolean() << " " << key_boolean_<< endl;
        return 2;
    }
    if (ptr->key_char() != key_char_){
        cout << "char keyword mismatch: " << ptr->key_char() << " " << key_char_<< endl;
        return 2;
    }
    if (ptr->key_string() != key_string_){
        cout << "string keyword mismatch: " << ptr->key_string() << " " << key_string_<< endl;
        return 2;
    }
    if (ptr->key_text() != key_text_){
        cout << "text keyword mismatch: " << ptr->key_text() << " " << key_text_<< endl;
        return 2;
    }
    cout << "done." << endl;

    key_byte_  = 100;
    key_int16_ = 100;
    key_int32_ = 100;
    key_int64_ = 100;
    key_float32_ = 1.732050808;
    key_float64_ = 2.645751311;
    key_boolean_ = false;
    key_char_ = 'f';
    key_string_ = "this is an updated 'test' string";
    key_text_ = "this is a updated 'test' text";


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

    try {
        D::Transaction t = db->begin();
        cout << "Updating metadata object... ";
        db->attach(ptr);
        t.commit();
        cout << "done." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 3;
    }

    return 0;
}


int check_metadata(const shared_ptr<D::Database> &db, long long id) {
    shared_ptr<test_keywords> ptr;
    try {
        D::Transaction t = db->begin();
        cout << "Retriving metadata object... ";
        ptr = db->load<test_keywords>(id);
        t.commit();
        cout << "done." << endl;
    } catch (const exception &e) {
        cout << "exception:" << endl << e.what() << endl;
        return 1;
    }

    cout << "Checking keywords... ";
    if (ptr->key_byte() != key_byte_){
        cout << "byte keyword mismatch: " << ptr->key_byte() << " " << key_byte_<< endl;
        return 2;
    }
    if (ptr->key_int16() != key_int16_){
        cout << "int16 keyword mismatch: " << ptr->key_int16() << " " << key_int16_<< endl;
        return 2;
    }
    if (ptr->key_int32() != key_int32_){
        cout << "int32 keyword mismatch: " << ptr->key_int32() << " " << key_int32_<< endl;
        return 2;
    }
    if (ptr->key_int64() != key_int64_){
        cout << "int64 keyword mismatch: " << ptr->key_int64() << " " << key_int64_<< endl;
        return 2;
    }
    if (ptr->key_float32() != key_float32_){
        cout << "float32 keyword mismatch: " << ptr->key_float32() << " " << key_float32_<< endl;
        return 2;
    }
    if (ptr->key_float64() != key_float64_){
        cout << "float64 keyword mismatch: " << ptr->key_float64() << " " << key_float64_<< endl;
        return 2;
    }
    if (ptr->key_boolean() != key_boolean_){
        cout << "boolean keyword mismatch: " << ptr->key_boolean() << " " << key_boolean_<< endl;
        return 2;
    }
    if (ptr->key_char() != key_char_){
        cout << "char keyword mismatch: " << ptr->key_char() << " " << key_char_<< endl;
        return 2;
    }
    if (ptr->key_string() != key_string_){
        cout << "string keyword mismatch: " << ptr->key_string() << " " << key_string_<< endl;
        return 2;
    }
    if (ptr->key_text() != key_text_){
        cout << "text keyword mismatch: " << ptr->key_text() << " " << key_text_<< endl;
        return 2;
    }
    cout << "done." << endl;

    cout << "Checking polimorphic interface... ";
    if (ptr->key_byte() != ptr->get_key<signed char>("key_byte")){
        cout << "byte keyword mismatch: " << ptr->key_byte() << " " << ptr->get_key<signed char>("key_byte") << endl;
        return 2;
    }
    if (ptr->key_int16() != ptr->get_key<short>("key_int16")){
        cout << "int16 keyword mismatch: " << ptr->key_int16() << " " << ptr->get_key<short>("key_int16") << endl;
        return 2;
    }
    if (ptr->key_int32() != ptr->get_key<int>("key_int32")){
        cout << "int32 keyword mismatch: " << ptr->key_int32() << " " << ptr->get_key<int>("key_int32") << endl;
        return 2;
    }
    if (ptr->key_int64() != ptr->get_key<long long>("key_int64")){
        cout << "int64 keyword mismatch: " << ptr->key_int64() << " " << ptr->get_key<long long>("key_int64") << endl;
        return 2;
    }
    if (ptr->key_float32() != ptr->get_key<float>("key_float32")){
        cout << "float32 keyword mismatch: " << ptr->key_float32() << " " << ptr->get_key<float>("key_float32") << endl;
        return 2;
    }
    if (ptr->key_float64() != ptr->get_key<double>("key_float64")){
        cout << "float64 keyword mismatch: " << ptr->key_float64() << " " << ptr->get_key<double>("key_float64") << endl;
        return 2;
    }
    if (ptr->key_boolean() != ptr->get_key<bool>("key_boolean")){
        cout << "boolean keyword mismatch: " << ptr->key_boolean() << " " << ptr->get_key<bool>("key_boolean") << endl;
        return 2;
    }
    if (ptr->key_char() != ptr->get_key<char>("key_char")){
        cout << "char keyword mismatch: " << ptr->key_char() << " " << ptr->get_key<char>("key_char") << endl;
        return 2;
    }
    if (ptr->key_string() != ptr->get_key<string>("key_string")){
        cout << "string keyword mismatch: " << ptr->key_string() << " " << ptr->get_key<string>("key_string") << endl;
        return 2;
    }
    if (ptr->key_text() != ptr->get_key<string>("key_text")){
        cout << "text keyword mismatch: " << ptr->key_text() << " " << ptr->get_key<string>("key_text") << endl;
        return 2;
    }
    cout << "done." << endl;
    
    return 0;
}


int main(){
    shared_ptr<D::Database> db = D::Database::create("test_level2");
    
    long long id = create_metadata(db);
    if(id == 0) return 1;
    int res = read_update_metadata(db,id);
    if(res != 0) return res;
    res = check_metadata(db,id);
    return res;
}