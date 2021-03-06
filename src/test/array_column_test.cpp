#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

const int CHUNK_SIZE = 3;
const int N_CHUNK = 2;

int main() {
    {
        shared_ptr<D::Database> db = D::Database::create("test_level2");
        shared_ptr<test_columns_array> ptr = test_columns_array::create("prova", "test_level2");

        for (int j = 0; j < N_CHUNK; ++j) {
            das::Array<int, 2>* ab = new das::Array<int, 2>[CHUNK_SIZE];
            for (int i = 0; i < CHUNK_SIZE; ++i) {
                int *b = new int[20];
                for (int j = 0; j < 20; ++j)
                    b[j] = (i + 1)*11111;
                das::Array<int, 2> a(b, das::shape(2, 10), das::deleteDataWhenDone);
                ab[i].reference(a);
            }
            das::ColumnArray<int, 2> ca(ab, CHUNK_SIZE, das::deleteDataWhenDone);
            ptr->append_column_array("column_int32", ca);
        }

        cout << "SIZE A " << ptr->get_column_array_size("column_int32") << endl;

        D::Transaction t = db->begin();
        db->persist(ptr);
        t.commit();

        cout << "SIZE B " << ptr->get_column_array_size("column_int32") << endl;

        for (int j = 0; j < N_CHUNK; ++j) {
            das::Array<int, 2>* ab = new das::Array<int, 2>[CHUNK_SIZE];
            for (int i = 0; i < CHUNK_SIZE; ++i) {
                int *b = new int[20];
                for (int j = 0; j < 20; ++j)
                    b[j] = (i + 5)*11111;
                das::Array<int, 2> a(b, das::shape(2, 10), das::deleteDataWhenDone);
                ab[i].reference(a);
            }
            das::ColumnArray<int, 2> ca(ab, CHUNK_SIZE, das::deleteDataWhenDone);
            ptr->append_column_array("column_int32", ca);
        }

        cout << "SIZE C " << ptr->get_column_array_size("column_int32") << endl;

        das::ColumnArray<int, 2> g = ptr->get_column_array<int, 2>("column_int32", 0, 8);
        cerr
                << g(0) << endl
                << g(1) << endl
                << g(2) << endl
                << g(3) << endl
                << g(4) << endl
                << g(5) << endl
                << g(6) << endl
                << g(7) << endl;

        t = db->begin();
        db->attach(ptr);
        t.commit();

        cout << "SIZE D " << ptr->get_column_array_size("column_int32") << endl;


    }


}
