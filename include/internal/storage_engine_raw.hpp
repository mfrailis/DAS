#ifndef STORAGE_ENGINE_RAW_HPP
#define	STORAGE_ENGINE_RAW_HPP
#include <list>
#include <fstream>
#include <sstream>
#include <exception>
#include "../storage_engine.hpp"


namespace das {

    class RawStorageTransaction : public StorageTransaction {
    public:

        virtual void add(DasObject *ptr);
        virtual void add(DasObject *ptr, const Extension &e);

        virtual void save(const std::string &path);

        virtual void save();

        virtual void commit(){
            
        }

        virtual void rollback();

        virtual ~RawStorageTransaction() {
        }

        RawStorageTransaction(TransactionBundle &tb);
    private:
        std::list<DasObject*> objs_;
        TransactionBundle& tb_;
    };

    class RawStorageAccess : public StorageAccess {
    public:

        RawStorageAccess(DasObject *obj, const DatabaseInfo &i) : StorageAccess(obj, i), cp_(0) {
            configure();
        }

        virtual ~RawStorageAccess() {
            for (token_vec::iterator it = tmp_path_.begin();
                    it != tmp_path_.end(); ++it)
                delete *it;

            for (token_vec::iterator it = std_path_.begin();
                    it != std_path_.end(); ++it)
                delete *it;

            for (token_vec::iterator it = cst_path_.begin();
                    it != cst_path_.end(); ++it)
                delete *it;
        }

        virtual size_t read_column(
                const std::string &col_name,
                Column* col,
                column_buffer_ptr buffer,
                size_t offset,
                size_t count);


        virtual size_t read_column_array(
                const std::string &col_name,
                Column* col,
                column_array_buffer_ptr &buffer,
                size_t offset,
                size_t count
                );

        virtual void flush_buffer(const std::string &col_name, Column* col);

        virtual size_t read_image(
                Image* img,
                image_buffer_ptr buffer,
                const das::TinyVector<int, 11> &offset,
                const das::TinyVector<int, 11> &count,
                const das::TinyVector<int, 11> &stride
                );
        
        virtual Column* create_column(
                const std::string &type,
                const std::string &array_size){
            return new ColumnFile(type,array_size);
        }  
        
        virtual Image* create_image(
                const std::string &pixel_type){
            return new ImageFile(pixel_type);
        }
        
        virtual void flush_buffer(Image* img);

        virtual bool buffered_only() {
            return false;
        }


        std::string
        get_default_path(const bool& mkdirs = false);

        std::string
        get_custom_path(const std::string &custom_path, const bool& mkdirs = false);

        std::string
        get_temp_path(const bool& mkdirs = false);

        
        virtual bool release(Column *c);
        
        virtual bool release(Image *i);

    private:

        class ResolveToken {
        public:
            virtual void expand(std::stringstream &ss) = 0;
            virtual void dbg(std::stringstream &ss) = 0;
        };

        void configure();
        void parse(const std::string &s, std::vector<ResolveToken*> &vec);
        void make_env(size_t &i, const string & exp, std::vector<ResolveToken*> &vec);

        class BasicToken : public ResolveToken {
        public:
            BasicToken(const std::string &str);
            void expand(std::stringstream &ss);
            void dbg(std::stringstream &ss);
        private:
            std::string s_;
        };

        class TimeToken : public ResolveToken {
        public:

            TimeToken(const std::string &str);
            void dbg(std::stringstream &ss);
            void expand(std::stringstream &ss);
        private:
            std::string s_;
        };

        class EnvToken : public ResolveToken {
        public:
            EnvToken(const std::string &str);
            void dbg(std::stringstream &ss);
            void expand(std::stringstream &ss);
        private:
            std::string s_;
        };

        class TypeToken : public ResolveToken {
        public:
            TypeToken(RawStorageAccess &sa, const char &c);
            void dbg(std::stringstream &ss);
            void expand(std::stringstream &ss);
        private:
            RawStorageAccess &sa_;
            char c_;
        };

        class CustomToken : public ResolveToken {
        public:
            CustomToken(RawStorageAccess &sa);
            void dbg(std::stringstream &ss);
            void expand(std::stringstream &ss);
        private:
            RawStorageAccess &sa_;
        };
        
        template<class T>
        bool drop(T* obj);

        typedef std::vector<ResolveToken*> token_vec;
        token_vec tmp_path_;
        token_vec std_path_;
        token_vec cst_path_;
        const std::string *cp_;
    };

    class bad_token : public std::exception {
    public:

        bad_token(char token) {
            msg_ = "unknown token : ";
            msg_.append(1, token);
        }

        virtual ~bad_token() throw () {
        }

        virtual const char*
        what() const throw () {
            return msg_.c_str();
        }
    private:
        std::string msg_;

    };

}


#endif	/* STORAGE_ENGINE_RAW_HPP */