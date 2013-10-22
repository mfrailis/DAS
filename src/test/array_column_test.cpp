#include "tpl/database.hpp"
#include "transaction.hpp"
#include "ddl/types.hpp"

using namespace std;
namespace D = das::tpl;

const int CHUNK_SIZE = 3;
const int N_CHUNK = 2;

int main() {
    typedef das::Array<int, 2> A_type;
    shared_ptr<D::Database> db = D::Database::create("test_level2");

    shared_ptr<test_columns_array> ptr = test_columns_array::create("prova", "test_level2");

    for (int j = 0; j < N_CHUNK; ++j) {
        A_type* ab = new A_type[CHUNK_SIZE];
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            int *b = new int[20];
            das::Array<int, 2> a(b, das::shape(10, 2), das::deleteDataWhenDone);
            ab[i] = a;
        }
        das::ColumnArray<int,2> ca(ab,CHUNK_SIZE,das::deleteDataWhenDone);
        ptr->append_column_array("column_int32",ca);
    }


    ptr->get_column_array<int, 2>("column_int32", 0, 5);
}