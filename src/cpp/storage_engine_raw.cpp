#include "../../include/internal/storage_engine_raw.hpp"
#include "../../include/exceptions.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h> 
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "../../include/ddl/info.hpp"
#include "../../include/das_object.hpp"
#include "../../include/internal/storage_engine.ipp"
#include "internal/log.hpp"
#include "../../include/internal/database_config.hpp"

using namespace std;

namespace das {
    namespace tpl {

        class RawStorageAccess_read_column : public boost::static_visitor<ssize_t> {
        public:

            RawStorageAccess_read_column(size_t offset, size_t count, /*size_t max_str,*/ const char* path)
            : o_(offset), c_(count), path_(path)/*, b_max_(max_str + 1)*/ {
            }

            ssize_t operator() (std::string* buff) const {
                char b[BUFF_SIZE];
                int fd = open(path_, O_RDONLY);
                if (fd == -1) throw io_exception();
                size_t terms_ = 0;
                size_t off = 0;
                ssize_t count = 0;

                count = read(fd, b, BUFF_SIZE);
                if (count == -1) {
                    close(fd);
                    //delete[] b;
                    throw io_exception();
                }
                while (terms_ < o_) {

                    off = 0;
                    while (terms_ < o_ && off < count)
                        if (b[off++] == '\0')
                            ++terms_;
                    if (off >= count) {
                        count = read(fd, b, BUFF_SIZE);
                        if (count == -1) {
                            close(fd);
                           // delete[] b;
                            throw io_exception();
                        }
                        off = 0;
                    }
                }

                for (size_t i = 0; i < c_; ++i) {
                    while (off < count) {
                        if (b[off++] == '\0')
                            break;
                        else
                            buff[i].push_back(b[off - 1]);
                    }
                    if (off >= count) {
                        count = read(fd, b, BUFF_SIZE);
                        if (count == -1) {
                            close(fd);
                            //delete[] b;
                            throw io_exception();
                        }
                        off = 0;
                    }
                }
                return c_;

            }

            template<typename T>
            ssize_t operator() (T* buff) const {
                int fd = open(path_, O_RDONLY);
                if (fd == -1) throw io_exception();
                off_t off = lseek(fd, o_ * sizeof (T), SEEK_SET);
                if (off == -1) {
                    close(fd);
                    throw io_exception();
                }
                ssize_t count = read(fd, buff, c_ * sizeof (T));
                if (count == -1) {
                    close(fd);
                    throw io_exception();
                }

                close(fd);
                count /= sizeof (T);
                return count;
            }

        private:
            size_t o_;
            size_t c_;
            //size_t b_max_;
            const char *path_;
            const static size_t BUFF_SIZE = 512;
        };

        class RawStorageAccess_append_column : public boost::static_visitor<size_t> {
        public:

            RawStorageAccess_append_column(int file_desc, size_t count)
            : c_(count), f_(file_desc) {
            }

            size_t operator() (std::string* buff) const {
                for (size_t i = 0; i < c_; ++i) {
                    size_t len = buff[i].length();
                    ssize_t count = write(f_, buff[i].c_str(), len + 1);
                    if (count == -1) throw io_exception();
                }
                return c_;
            }

            template<typename T>
            size_t operator() (T* buff) const {
                ssize_t count = write(f_, buff, c_ * sizeof (T));
                if (count == -1) throw io_exception();
                count /= sizeof (T);
                return count;
            }
        private:
            size_t c_;
            int f_;
        };

        size_t
        RawStorageAccess::read(const std::string &col_name, ColumnFromFile* c,
                column_buffer_ptr buffer, size_t offset, size_t count) {
            //size_t max_str_l = DdlInfo::get_instance()->get_column_info(type_name(obj_), col_name).max_string_length;
            if (c->temp_path() != "")
                return boost::apply_visitor(
                    RawStorageAccess_read_column(offset, count, /*max_str_l,*/ c->temp_path().c_str()),
                    buffer);
            else
                if (c->fname() != "") {
                std::stringstream ss;
                ss << c->fname() << c->id();

                return boost::apply_visitor(
                        RawStorageAccess_read_column(offset, count, /*max_str_l,*/ ss.str().c_str()),
                        buffer);
            }

            return 0;
        }

        /*
        size_t
        RawStorageAccess::append(std::fstream &stream, column_buffer_ptr buffer, size_t count) {
            return boost::apply_visitor(
                    RawStorageAccess_append_column(stream, count),
                    buffer);

        }
         */
        RawStorageTransaction::RawStorageTransaction(TransactionBundle &tb)
        : tb_(tb) {
        }

        void
        RawStorageTransaction::add(DasObject *ptr) {

            objs_.push_back(ptr);
        }

        void
        RawStorageTransaction::add(DasObject *ptr, const Extension &e) {

            objs_.push_back(ptr);
        }

        class RawStorageAccess_FlushColumnBuffer : public boost::static_visitor<void> {
        public:

            RawStorageAccess_FlushColumnBuffer(
                    ColumnFromFile *c,
                    RawStorageAccess *s)
            : c_(c), s_(s) {

            }

            template<typename T>
            void operator() (T &native_type) const {
                typedef std::vector<std::pair<T*, size_t> > buckets_type;
                buckets_type bks = c_->buffer().buckets<T>();
                size_t size = 0;

                int file_des = open(c_->temp_path().c_str(), O_WRONLY | O_APPEND);
                if (file_des == -1)
                    throw io_exception();

                for (typename buckets_type::iterator it = bks.begin(); it != bks.end(); ++it) {

                    StorageAccess::column_buffer_ptr buffer = it->first;
                    size_t count = boost::apply_visitor(
                            RawStorageAccess_append_column(file_des, it->second),
                            buffer);
                    size += it->second;
                }

                close(file_des);

                size += c_->file_size();
                c_->file_size(size);
                c_->buffer().clear();
            }

        private:
            ColumnFromFile* c_;
            RawStorageAccess* s_;
        };

        void
        RawStorageTransaction::save(const std::string &path) {
            for (std::vector<DasObject*>::iterator obj_it = objs_.begin();
                    obj_it != objs_.end(); ++obj_it) {
                std::map<std::string, ColumnFromFile*> map;
                DasObject* obj = *obj_it;
                StorageTransaction::get_columns_from_file(obj, map);

                for (std::map<std::string, ColumnFromFile*>::iterator m_it = map.begin();
                        m_it != map.end(); ++m_it) {
                    std::string c_name = m_it->first;
                    ColumnFromFile* cff = m_it->second;

                    RawStorageAccess *rsa = dynamic_cast<RawStorageAccess*> (storage_access(obj));
                    if (cff == NULL) // empty column, skip
                        continue;

                    rsa->flush_buffer(c_name, cff);

                    std::string temp_path = cff->temp_path();
                    if (temp_path != "") {
                        std::stringstream ss;
                        if (path == "")
                            ss << rsa->get_default_path(true);
                        else
                            ss << rsa->get_custom_path(path, true);

                        ss << m_it->first << "_";

                        ColumnFromFile cffn(cff->file_size(), cff->get_type(), ss.str());
                        column_from_file(obj, c_name, cffn);

                        ColumnFromFile *cffnp = column_from_file(obj, c_name);
                        cffnp->persist(*(tb_.db()));

                        ss << cffnp->id();
                        if (rename(temp_path.c_str(), ss.str().c_str())) {

                            std::cout << strerror(errno) << std::endl;
                            throw das::io_exception();
                        }

                    }
                }

            }
        }

        void
        RawStorageTransaction::save() {
            for (std::vector<DasObject*>::iterator obj_it = objs_.begin();
                    obj_it != objs_.end(); ++obj_it) {
                std::map<std::string, ColumnFromFile*> map;
                DasObject* obj = *obj_it;
                StorageTransaction::get_columns_from_file(obj, map);
                RawStorageAccess *rsa = dynamic_cast<RawStorageAccess*> (storage_access(obj));
                
                std::string storage_path;
                
                /*
                 * if some column was previously stored, use that path for storing
                 *  new and updated data
                 */
                for (std::map<std::string, ColumnFromFile*>::iterator m_it = map.begin();
                        m_it != map.end(); ++m_it) {
                    ColumnFromFile* cff = m_it->second;
                    
                    if(cff == NULL || cff->fname() == "")
                        continue;
                    
                    std::string str = cff->fname();
                    size_t found = str.find_last_of("/");
                    if(found < std::string::npos){
                        storage_path = str.substr(0,found);
                        break;
                    }
                    
                }
                
                /*
                 * if we didn't find any suitable path, than fall back on 
                 * default path
                 */
                if(storage_path == "")
                    storage_path = rsa->get_default_path(true);
                
                for (std::map<std::string, ColumnFromFile*>::iterator m_it = map.begin();
                        m_it != map.end(); ++m_it) {
                    std::string c_name = m_it->first;
                    ColumnFromFile* cff = m_it->second;
                    
                    if (cff == NULL) // empty column, skip
                        continue;

                    rsa->flush_buffer(c_name, cff);

                    std::string temp_path = cff->temp_path();
                    if (temp_path != "") {
                        std::stringstream ss;

                        ss << storage_path;
                        ss << m_it->first << "_";

                        ColumnFromFile cffn(cff->file_size(), cff->get_type(), ss.str());
                        column_from_file(obj, c_name, cffn);

                        ColumnFromFile *cffnp = column_from_file(obj, c_name);
                        cffnp->persist(*(tb_.db()));

                        ss << cffnp->id();
                        if (rename(temp_path.c_str(), ss.str().c_str())) {

                            std::cout << strerror(errno) << std::endl;
                            throw das::io_exception();
                        }

                    }
                }

            }
        }

        void
        RawStorageAccess::flush_buffer(const std::string &col_name, ColumnFromFile* col) {
            if (col->buffer().empty()) return;
            if (col->temp_path() == "") {
                int dst_fd;
                std::stringstream tmp;
                tmp << get_temp_path(true) << obj_->name() << "_" << col_name << "_XXXXXX";
                std::string str = tmp.str();
                size_t len = str.length();
                char *c_str = new char[len + 1];
                str.copy(c_str, len);
                c_str[len] = '\0';
                // generates unique filename and opens it exclusively
                dst_fd = mkstemp(c_str);
                if (dst_fd == -1) {
                    delete[] c_str;
                    throw io_exception();
                }


                col->temp_path(c_str);
                delete[] c_str;

                if (col->fname() != "") {

                    std::stringstream src;
                    src << col->fname() << col->id();

                    int src_fd;
                    struct stat stat_buf;
                    off_t offset = 0;

                    src_fd = open(src.str().c_str(), O_RDONLY);
                    fstat(src_fd, &stat_buf);
                    sendfile(dst_fd, src_fd, &offset, stat_buf.st_size);
                    close(src_fd);
                }
                close(dst_fd);
            }
            column_type col_t = DdlInfo::get_instance()->
                    get_column_info(type_name(obj_), col_name).type_var_;
            boost::apply_visitor(
                    RawStorageAccess_FlushColumnBuffer(col, this),
                    col_t);
        }

        inline
        RawStorageAccess::BasicToken::BasicToken(const std::string& str) : s_(str) {
        }

        inline
        void
        RawStorageAccess::BasicToken::expand(std::stringstream& ss) {

            ss << s_;
        }

        inline
        void
        RawStorageAccess::BasicToken::dbg(std::stringstream &ss) {

            ss << "basic: " << s_ << endl;
        }

        inline
        RawStorageAccess::TimeToken::TimeToken(const std::string& str) : s_(str) {
        }

        inline
        void
        RawStorageAccess::TimeToken::expand(std::stringstream& ss) {
            time_t rawtime;
            struct tm * timeinfo;
            char buffer [128];

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer, 128, s_.c_str(), timeinfo);

            size_t i = 0;
            while (i < 128 && buffer[i] != '\0') {
                switch (buffer[i]) {
                    case ':':
                        buffer[i] = '-';
                        break;
                    case '/':
                    case ' ':
                        buffer[i] = '_';

                        break;
                    default:
                        break;
                }
                ++i;
            }
            ss << buffer;
        }

        inline
        void
        RawStorageAccess::TimeToken::dbg(std::stringstream &ss) {

            ss << "time : " << s_ << endl;
        }

        inline
        RawStorageAccess::EnvToken::EnvToken(const std::string& str) : s_(str) {
        }

        inline
        void
        RawStorageAccess::EnvToken::expand(std::stringstream& ss) {
            char* var;
            var = getenv(s_.c_str());

            if (!var) return;
            ss << var;
        }

        inline
        void
        RawStorageAccess::EnvToken::dbg(std::stringstream &ss) {

            ss << "env  : " << s_ << endl;
        }

        inline
        RawStorageAccess::TypeToken::TypeToken(RawStorageAccess &sa, const char& c)
        : sa_(sa), c_(c) {
        }

        inline
        void
        RawStorageAccess::TypeToken::dbg(std::stringstream &ss) {

            ss << "type : %" << c_ << endl;
        }

        inline
        void
        RawStorageAccess::TypeToken::expand(std::stringstream& ss) {
            switch (c_) {
                case 't':
                    ss << type_name(sa_.obj_);
                    break;
                case 'n':
                    ss << sa_.obj_->name();

                    break;
                case 'v':
                    ss << sa_.obj_->version();
            }
        }

        inline
        RawStorageAccess::CustomToken::CustomToken(RawStorageAccess& sa)
        : sa_(sa) {
        }

        inline
        void
        RawStorageAccess::CustomToken::dbg(std::stringstream &ss) {

            ss << "cust.: %s" << endl;
        }

        inline
        void
        RawStorageAccess::CustomToken::expand(std::stringstream& ss) {

            ss << *sa_.cp_;
        }

        void
        RawStorageAccess::parse(const std::string& exp, std::vector<ResolveToken*>& vec) {
            stringstream *ss = new stringstream();

            size_t size = exp.length();
            if (size < 2) return;
            char c, la;
            c = exp[0];
            la = exp[1];
            size_t i = 2;
            bool skip = false;
            do {
                if (skip) {
                    c = la;
                    la = exp[i++];
                    skip = false;
                    continue;
                }
                switch (c) {
                    case '%':
                        switch (la) {
                            case '$':
                                if (ss->str() != "")
                                    vec.push_back(new BasicToken(ss->str()));
                                delete ss;
                                ss = new stringstream();
                                make_env(i, exp, vec);
                                delete ss;
                                ss = new stringstream();
                                break;
                            case '%':
                                (*ss) << '%';
                                skip = true;
                                break;
                            case 'a':
                            case 'A':
                            case 'b':
                            case 'B':
                            case 'C':
                            case 'c':
                            case 'd':
                            case 'F':
                            case 'g':
                            case 'G':
                            case 'H':
                            case 'I':
                            case 'j':
                            case 'm':
                            case 'M':
                            case 'p':
                            case 'r':
                            case 'R':
                            case 'S':
                            case 'T':
                            case 'u':
                            case 'U':
                            case 'V':
                            case 'w':
                            case 'W':
                            case 'x':
                            case 'X':
                            case 'y':
                            case 'Y':
                            case 'z':
                            case 'Z':
                            {
                                if (ss->str() != "")
                                    vec.push_back(new BasicToken(ss->str()));
                                delete ss;
                                ss = new stringstream();
                                std::string tk("%");
                                tk.append(1, la);
                                vec.push_back(new TimeToken(tk));
                                skip = true;
                                break;
                            }
                            case 't':
                            case 'n':
                            case 'v':
                                if (ss->str() != "")
                                    vec.push_back(new BasicToken(ss->str()));
                                delete ss;
                                ss = new stringstream();
                                vec.push_back(new TypeToken(*this, la));
                                skip = true;
                                break;
                            case 's':
                                if (ss->str() != "")
                                    vec.push_back(new BasicToken(ss->str()));
                                delete ss;
                                ss = new stringstream();
                                vec.push_back(new CustomToken(*this));
                                skip = true;
                                break;
                            default:
                                cout << "unknown expression: %" << la << endl;
                        }
                        break;
                    default:
                        (*ss) << c;
                }
                c = exp[i - 1];
                if (i < size)
                    la = exp[i];
                ++i;
            } while (i < size + 2);

            if (ss->str() != "")
                vec.push_back(new BasicToken(ss->str()));

            delete ss;
        }

        void
        RawStorageAccess::make_env(size_t &i, const string & exp, std::vector<ResolveToken*> &vec) {
            std::stringstream ss;
            while (i < exp.size()) {
                if (exp[i] != '$')
                    ss << exp[i];
                else {
                    vec.push_back(new EnvToken(ss.str()));
                    i += 2;

                    return;
                }
                ++i;
            }
        }

        void
        RawStorageAccess::configure() {
            std::string root_dir = info.storage_engine.get<std::string> ("root_dir");
            if (root_dir[root_dir.length() - 1] != '/')
                root_dir.append("/");
            std::string default_path(root_dir);
            std::string custom_path(root_dir);
            default_path += info.storage_engine.get<std::string> ("default_path");
            custom_path += info.storage_engine.get<std::string> ("custom_path");
            std::string temp_path = info.storage_engine.get<std::string> ("temp_path");

            if (default_path[default_path.length() - 1] != '/')
                default_path.append("/");
            if (custom_path[custom_path.length() - 1] != '/')
                custom_path.append("/");

            if (temp_path[temp_path.length() - 1] != '/')
                temp_path.append("/");


            parse(default_path, std_path_);
            parse(custom_path, cst_path_);
            parse(temp_path, tmp_path_);
        }

        std::string
        RawStorageAccess::get_default_path(const bool& m) {
            std::stringstream ss;
            for (token_vec::iterator it = std_path_.begin();
                    it != std_path_.end(); ++it) {
                (*it)->expand(ss);
            }
            std::string s = ss.str();

            if (s[s.length() - 1] != '/')
                s.append("/");
            if (s.find("/../") != std::string::npos)
                throw bad_path();
            if (s.find("//") != std::string::npos)
                throw bad_path();

            if (m) make_dirs(s);

            return s;
        }

        std::string
        RawStorageAccess::get_custom_path(const std::string &c, const bool& m) {
            cp_ = &c;
            std::stringstream ss;
            for (token_vec::iterator it = cst_path_.begin();
                    it != cst_path_.end(); ++it) {
                (*it)->expand(ss);
            }
            cp_ = NULL;
            std::string s = ss.str();

            if (s[s.length() - 1] != '/')
                s.append("/");
            if (s.find("/../") != std::string::npos)
                throw bad_path();
            if (s.find("//") != std::string::npos)
                throw bad_path();

            if (m) make_dirs(s);

            return s;
        }

        std::string
        RawStorageAccess::get_temp_path(const bool& m) {
            std::stringstream ss;
            for (token_vec::iterator it = tmp_path_.begin();
                    it != tmp_path_.end(); ++it) {
                (*it)->expand(ss);
            }
            std::string s = ss.str();

            if (s[s.length() - 1] != '/')
                s.append("/");
            if (s.find("/../") != std::string::npos)
                throw bad_path();
            if (s.find("//") != std::string::npos)
                throw bad_path();

            if (m) make_dirs(s);

            return s;
        }

        void
        RawStorageAccess::make_dirs(const std::string &s) {
            struct stat stt;
            size_t offset = 0;
            size_t len = s.length() - 1;
            while (offset < len) {
                ++offset;
                offset = s.find_first_of('/', offset);
                if (offset < std::string::npos) {
                    std::string path(s.c_str(), offset);
                    if (stat(path.c_str(), &stt))
                        if (errno == ENOENT)
                            mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //TODO: handle errors and permisisons
                }
            }
        }

        /*
                class RawStorageAccess_write_column : public boost::static_visitor<size_t> {
                public:

                    RawStorageAccess_write_column(size_t offset, size_t count, const char* path)
                    : o_(offset), c_(count), path_(path) {
                    }

                    size_t operator() (std::string* buff) const {
                        throw not_implemented();
                    }

                    template<typename T>
                    size_t operator() (T* buff) const {
                        fstream s;
                        s.open(path_, std::ios_base::out);
                        s << std::setprecision(std::numeric_limits<T>::digits);
                        T skip;
                        while (s.good() && o_-- > 0)
                            s >> skip;

                        size_t count = 0;
                        while (s.good() && c_-- > 0)
                            s << buff[count++] << endl;

                        s.close();
                        return count;
                    }
                    mutable size_t o_;
                    mutable size_t c_;
                    const char *path_;
                };

                size_t
                RawStorageAccess::write(ColumnFromFile* c, column_buffer_ptr buffer, size_t offset, size_t count) {
                    if (c->temp_path() != "")
                        return boost::apply_visitor(
                            RawStorageAccess_write_column(offset, count, c->temp_path().c_str()),
                            buffer);
                    else
                        if (c->fname() != "")
                        return boost::apply_visitor(
                            RawStorageAccess_write_column(offset, count, c->fname().c_str()),
                            buffer);
                    else {
                        throw std::exception();
                    }
                }
         */
    }

}