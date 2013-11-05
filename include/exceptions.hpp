#ifndef DAS_EXCEPTIONS_HPP
#define DAS_EXCEPTIONS_HPP
#include <odb/exceptions.hxx>
#include <string.h>
#include <errno.h>
namespace das {



    typedef odb::object_not_persistent object_not_persistent;

    class incompatible_array_shape : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "incompatible array shapes";
        }

    };

    class read_only_keyword : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "cannot modify a read-only keyword";
        }

    };
    typedef odb::object_not_persistent object_not_persistent;

    class bad_array_slice : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "bad offset, count or stride parameter while slicing array";
        }

    };

    class not_implemented : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "functionality not implemented yet";
        }

    };

    class abstract_das_object : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "virtual function not implemented in this object";
        }

    };

    class object_corrupt : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "the object contains corrupted data";
        }

    };

    class wrong_database : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "wrong database provided";
        }

    }; //TODO

    class no_credentials : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "database access denied";
        }

    }; //TODO

    class no_database : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "no database provided";
        }

    }; //TODO

    class keyword_not_present : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "the keyword requested is not present";
        }

    }; //TODO
    
        class bad_keyword_type : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "object type and keyword type incompatible";
        }

    }; //TODO

    class column_not_present : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "the column requested is not present";
        }

    }; //TODO

    class empty_column : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "the column requested is empty";
        }

    }; //TODO

    class wrong_size : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "requested wrong number of arrays in column array";
        }

    }; //TODO   
    
    class bad_array_size : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "the column requested has incopatible array size";
        }

    }; //TODO
    
    class bad_array_shape : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "requested array with incompatible shape";
        }

    }; //TODO
    class empty_image : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "the image requested is empty";
        }

    }; //TODO

    class association_not_present : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "the association requested is not present";
        }

    }; //TODO

    class unknown_kewyword_type : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "unknown type";
        }

    }; //TODO

    class non_compatible_types : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "types non compatible";
        }

    }; //TODO

    class bad_type : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "type non compatible with the expression";
        }

    }; //TODO

    class incomplete_statement : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "incomplete query expression";
        }

    }; //TODO

    class bad_ordering_clause : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "bad ordering clause";
        }

    }; //TODO

    class not_in_managed_context : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "operation forbidden outside a session or transaction";
        }

    }; //TODO

    class not_in_transaction : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "operation forbidden outside a transaction";
        }

    }; //TODO  

    class new_object : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "can't perform this operation on new object";
        }

    }; //TODO  

    class object_is_corrupt : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "corrupt object";
        }

    }; //TODO  

    class object_not_unique : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "session holds another object whit the same key";
        }

    }; //TODO  

    class bad_path : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "cannot access file";
        }

    }; //TODO

    class bad_param : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "wrong parameter";
        }

    }; //TODO


    //IO exceptions

    class io_exception : public std::exception {
    public:
        io_exception() : std::exception(),err_(0){}
        io_exception(int err): std::exception(),err_(err){}
        virtual const char*
        what() const throw () {
            if(err_)
                return strerror(err_);
            else
                return "io exception";
        }
        int err_;

    }; //TODO

    class data_corrupted : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "array size not compatible with array shape: possible data corruption";
        }


    }; //TODO 
    
    
    class das_io_exception : public std::exception {
    public:

        das_io_exception(char *str) : errstr_(str) {
        }

        virtual const char*
        what() const throw () {
            return errstr_;
        }
    private:
        char * errstr_;

    }; //TODO 

    class already_in_session : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "another session is active onto this database";
        }
    private:
        char * errstr_;

    }; //TODO 

    class already_in_transaction : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "another transaction is active onto this database";
        }
    private:
        char * errstr_;

    }; //TODO 

    class not_in_session : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "no session or transaction active onto this database";
        }
    private:
        char * errstr_;

    }; //TODO 

    class invalid_transaction : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "transaction corrupted. Perhaps already committed";
        }
    private:
        char * errstr_;

    }; //TODO    

    class no_external_data : public std::exception {
    public:

        virtual const char*
        what() const throw () {
            return "the object does not provide non-database data access";
        }
    private:
        char * errstr_;

    }; //TODO  
}

#endif
