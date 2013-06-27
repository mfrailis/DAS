#ifndef RESULT_HPP
#define	RESULT_HPP
#include <odb/database.hxx>
namespace das {
    namespace tpl {

        template<typename T>
        class Result : public odb::result<T> {
        public:

            Result(const odb::result<T> &r) : odb::result<T>(r) {
            }
        };
    }
}
#endif	/* RESULT_HPP */

