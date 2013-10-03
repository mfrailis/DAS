#include <iostream>
#include <exception>
#include <das/tpl/database.hpp>
#include <das/transaction.hpp>
#include <das/ddl/types.hpp>
#include "internal/array.hpp"
#include "tpl/database.hpp"

#include <limits>
#include <sstream>
#include <typeinfo>
#include <boost/thread/thread.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/random_device.hpp>

#define N_ITERATIONS 2
#define N_POOLS 1

using namespace std;
namespace D = das::tpl;

boost::mutex log_mtx;
boost::mutex tr_mtx;
boost::lagged_fibonacci3217 r;


#define LOG(s) {log_mtx.lock(); cout << s; log_mtx.unlock();}

template<typename T>
T gen_rand() {
    return r() * numeric_limits<T>::max()* 2 - numeric_limits<T>::min();
}

template<>
float gen_rand<float>() {
    return r();
}

template<>
double gen_rand<double>() {
    return r();
}

template<>
bool gen_rand<bool>() {
    return r() > 0.5;
}

template<>
string gen_rand<string>() {
    stringstream ss;
    string chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "1234567890"
            "-_-+");
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
    boost::random::uniform_int_distribution<> dim(0, 256);
    int len = dim(rng);
    for (int i = 0; i < len; ++i) {
        ss << chars[index_dist(rng)];
    }
    return ss.str();
}

template<typename T>
das::Array<T>
gen_rand_array() {
    das::Array<T> array;
    boost::random::uniform_int_distribution<> dim(1, 50);
    boost::random::random_device rng;
    array.resize(dim(rng));

    for (size_t i = 0; i < array.size(); ++i) {
        array(i) = gen_rand<T>();
    }

    return array;
}

template<typename T>
void append_column(das::Array<T> &array, shared_ptr<test_columns> &ptr) {
}

template<>
void append_column(das::Array<string> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_string", array);
}

template<>
void append_column(das::Array<char> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_char", array);
    long long sig = sum(array);
    ptr->sig_char(ptr->sig_char() + sig);
    ptr->dim_char(ptr->dim_char() + array.size());
}

template<>
void append_column(das::Array<short> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_int16", array);
    long long sig = sum(array);
    ptr->sig_int16(ptr->sig_int16() + sig);
    ptr->dim_int16(ptr->dim_int16() + array.size());
}

template<>
void append_column(das::Array<int> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_int32", array);
    long long sig = sum(array);
    ptr->sig_int32(ptr->sig_int32() + sig);
    ptr->dim_int32(ptr->dim_int32() + array.size());
}

template<>
void append_column(das::Array<long long> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_int64", array);
    long long sig = sum(array);
    ptr->sig_int64(ptr->sig_int64() + sig);
    ptr->dim_int64(ptr->dim_int64() + array.size());
}

template<>
void append_column(das::Array<float> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_float32", array);
    double sig = sum(array);
    ptr->sig_float32(ptr->sig_float32() + sig);
    ptr->dim_float32(ptr->dim_float32() + array.size());
}

template<>
void append_column(das::Array<double> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_float64", array);
    double sig = sum(array);
    ptr->sig_float64(ptr->sig_float64() + sig);
    ptr->dim_float64(ptr->dim_float64() + array.size());
}

template<>
void append_column(das::Array<bool> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_boolean", array);
    long long sig = sum(array);
    ptr->sig_boolean(ptr->sig_boolean() + sig);
    ptr->dim_boolean(ptr->dim_boolean() + array.size());
}

template<>
void append_column(das::Array<unsigned char> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_uint8", array);
    long long sig = sum(array);
    ptr->sig_uint8(ptr->sig_uint8() + sig);
    ptr->dim_uint8(ptr->dim_uint8() + array.size());
}

template<>
void append_column(das::Array<unsigned short> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_uin16", array);
    long long sig = sum(array);
    ptr->sig_uint16(ptr->sig_uint16() + sig);
    ptr->dim_uint16(ptr->dim_uint16() + array.size());
}

template<>
void append_column(das::Array<unsigned int> &array, shared_ptr<test_columns> &ptr) {
    ptr->append_column("column_uint32", array);
    long long sig = sum(array);
    ptr->sig_uint32(ptr->sig_uint32() + sig);
    ptr->dim_uint32(ptr->dim_uint32() + array.size());
}

string print_stats(const shared_ptr<test_columns> &ptr) {
    stringstream ss;
    //ss.width(10);
    int pad = 4;

    ss << " | ";
    /*ss << "column_string";
    ss.width(pad);
    ss << ptr->get_column_size("column_string")
            << " | " << "column_byte";
    ss.width(pad);
    ss << ptr->get_column_size("column_byte")
            << " | " << "column_int16";
    ss.width(pad);*/
    ss << ptr->get_column_size("column_int16")
            << " | " << "column_int32";
    ss.width(pad);
    ss << ptr->get_column_size("column_int32")
            << " | " << "column_int64";
    ss.width(pad);
    ss << ptr->get_column_size("column_int64")
            << " | " << "column_float32";
    ss.width(pad);
    ss << ptr->get_column_size("column_float32")
            << " | " << "column_float64";
    ss.width(pad);
    ss << ptr->get_column_size("column_float64")
            << " | " << "column_boolean";
    ss.width(pad);
    ss << ptr->get_column_size("column_boolean")
            << " | " << "column_char";
    ss.width(pad);
    ss << ptr->get_column_size("column_char")
            << " | " << "column_uint8";
    ss.width(pad);
    ss << ptr->get_column_size("column_uint8")
            << " | " << "column_uin16";
    ss.width(pad);
    ss << ptr->get_column_size("column_uin16")
            << " | " << "column_uint32";
    ss.width(pad);
    ss << ptr->get_column_size("column_uint32")
            << " | ";

    return ss.str();

}

/*
char,
short,
int,
long long,
float,
double,
bool,
unsigned char,
unsigned short,
unsigned int,
std::string
 */



template<typename T>
void iteraction(const shared_ptr<D::Database> &db, long long id) {
    shared_ptr<test_columns> ptr;

    tr_mtx.lock();
    try {
        D::Transaction t = db->begin();
        ptr = db->load<test_columns>(id);
        t.commit();
        LOG(boost::this_thread::get_id() << " " << typeid (T).name() << ":L" << print_stats(ptr) << endl);
    } catch (const exception &e) {
        LOG(boost::this_thread::get_id() << " " << typeid (T).name() << ": exception while loading object: "
                << e.what() << endl);
        tr_mtx.unlock();
        return;
    }
    tr_mtx.unlock();

    das::Array<T> array = gen_rand_array<T>();
    append_column(array, ptr);

    tr_mtx.lock();
    LOG(boost::this_thread::get_id() << " " << typeid (T).name() << ":M" << print_stats(ptr) << endl);
    try {
        D::Transaction t = db->begin();
        db->attach(ptr);
        t.commit();
        LOG(boost::this_thread::get_id() << " " << typeid (T).name() << ":S" << print_stats(ptr) << endl);
    } catch (const exception &e) {
        LOG(boost::this_thread::get_id() << " " << typeid (T).name() << ": exception while saving object: "
                << e.what() << endl);
    }
    tr_mtx.unlock();
}

template<typename T>
int DataWorker(long long id) {
    shared_ptr<D::Database> db = D::Database::create("test_level2");
    for (size_t i = 0; i < N_ITERATIONS; ++i)
        iteraction<T>(db, id);

    LOG(boost::this_thread::get_id() << ": terminates" << endl);
    return 0;
};

template<typename T>
long long sum(das::Array<T, 1> &array) {
    long long sum = 0;
    for (typename das::Array<T, 1>::iterator it = array.begin(); it != array.end(); ++it)
        sum += *it;
    return sum;
}

double sum(das::Array<float, 1> &array) {
    double sum = 0;
    for (typename das::Array<float, 1>::iterator it = array.begin(); it != array.end(); ++it)
        sum += *it;
    return sum;
}

double sum(das::Array<double, 1> &array) {
    double sum = 0;
    for (typename das::Array<double, 1>::iterator it = array.begin(); it != array.end(); ++it)
        sum += *it;
    return sum;
}

class Pool {
private:
    boost::thread t0, t1, t2, t3, t4, t5, t6, t7, t8, t9/*,t10*/;
public:

    Pool(long long id) :
    t0(&DataWorker<char>, id),
    t1(&DataWorker<short>, id),
    t2(&DataWorker<int>, id),
    t3(&DataWorker<long long>, id),
    t4(&DataWorker<float>, id),
    t5(&DataWorker<double>, id),
    t6(&DataWorker<bool>, id),
    t7(&DataWorker<unsigned char>, id),
    t8(&DataWorker<unsigned short>, id),
    t9(&DataWorker<unsigned int>, id)/*,
    t10(&DataWorker<std::string>, id) */ {
    }

    void join() {
        t0.join();
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        t6.join();
        t7.join();
        t8.join();
        t9.join();
        //    t10.join();
    }
};

int main(int argc, char** argv) {
    cout << "char           :" << typeid (char).name() << endl;
    cout << "short          :" << typeid (short).name() << endl;
    cout << "int            :" << typeid (int).name() << endl;
    cout << "long long      :" << typeid (long long).name() << endl;
    cout << "float          :" << typeid (float).name() << endl;
    cout << "double         :" << typeid (double).name() << endl;
    cout << "bool           :" << typeid (bool).name() << endl;
    cout << "unsigned char  :" << typeid (unsigned char).name() << endl;
    cout << "unsigned short :" << typeid (unsigned short).name() << endl;
    cout << "unsigned int   :" << typeid (unsigned int).name() << endl;
    cout << "std::string    :" << typeid (std::string).name() << endl;



    shared_ptr<D::Database> db = D::Database::create("test_level2");
    shared_ptr<test_columns> ptr = test_columns::create("column_test_C", "test_level2");
    D::Transaction t = db->begin();
    long long id = db->persist(ptr);
    t.commit();
    vector<Pool*> pool;

    for (int i = 0; i < N_POOLS; ++i) {
        pool.push_back(new Pool(id));
    }
    for (int i = 0; i < N_POOLS; ++i)
        pool[i]->join();

    return 0;
}

