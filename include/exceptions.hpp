#ifndef DAS_EXCEPTIONS_HPP
#define DAS_EXCEPTIONS_HPP
#include <odb/exceptions.hxx>

namespace das
{
    typedef odb::object_not_persistent object_not_persistent;

    class wrong_database : public std::exception
    {
    public :
        virtual const char*
        what() const throw()
        {
            return "wrong database";
        }

    }; //TODO

}

#endif
