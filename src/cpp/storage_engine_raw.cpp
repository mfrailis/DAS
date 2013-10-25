#include "../../include/internal/storage_engine_raw.hpp"
#include "../../include/exceptions.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
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

typedef unsigned long long chunk_t;

/* class for auto managing file descriptors.
 * Avoids dangling opened files
 */
class AutoFile {
public:

    AutoFile(int fd) : fd_(fd) {
    }

    operator int&() {
        return fd_;
    }

    ~AutoFile() {
        if (fd_ > 0) {
            close(fd_);
        }
    }

private:
    AutoFile();
    AutoFile(const AutoFile& rhs);
    const AutoFile& operator=(const AutoFile& rhs);

    int fd_;
};


namespace das {

    class RawStorageAccess_read_column : public boost::static_visitor<ssize_t> {
    public:

        RawStorageAccess_read_column(size_t offset, size_t count, /*size_t max_str,*/ const char* path)
        : o_(offset), c_(count), path_(path)/*, b_max_(max_str + 1)*/ {
        }

        ssize_t operator() (std::string* buff) const {
            char b[BUFF_SIZE];
            errno = 0;
            AutoFile fd(open(path_, O_RDONLY));
            if (fd == -1)
                throw io_exception(errno);
            size_t terms_ = 0;
            size_t off = 0;
            ssize_t count = 0;
            errno = 0;
            count = read(fd, b, BUFF_SIZE);
            if (count == -1) {
                int err = errno;
                throw io_exception(err);
            }
            while (terms_ < o_) {

                off = 0;
                while (terms_ < o_ && off < count)
                    if (b[off++] == '\0')
                        ++terms_;
                if (off >= count) {
                    errno = 0;
                    count = read(fd, b, BUFF_SIZE);
                    if (count == -1) {
                        int err = errno;
                        throw io_exception(err);
                    }
                    off = 0;
                }
            }
            size_t i = 0;
            while (i < c_) {
                while (off < count) {
                    if (b[off] == '\0') {
                        ++off;
                        ++i;
                        break;
                    } else
                        buff[i].push_back(b[off++]);
                }
                if (off >= count) {
                    errno = 0;
                    count = read(fd, b, BUFF_SIZE);
                    if (count == -1) {
                        int err = errno;
                        throw io_exception(err);
                    }
                    off = 0;
                }
            }
            return c_;

        }

        template<typename T>
        ssize_t operator() (T* buff) const {
            errno = 0;
            AutoFile fd(open(path_, O_RDONLY));
            if (fd == -1)
                throw io_exception(errno);
            errno = 0;
            off_t off = lseek(fd, o_ * sizeof (T), SEEK_SET);
            if (off == -1) {
                int err = errno;
                throw io_exception(err);
            }
            errno = 0;
            ssize_t count = read(fd, buff, c_ * sizeof (T));
            if (count == -1) {
                int err = errno;
                throw io_exception(err);
            }

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

    class RawStorageAccess_read_column_array : public boost::static_visitor<ssize_t> {
    public:

        RawStorageAccess_read_column_array(size_t offset, size_t count, const std::string &array_size, const char* path)
        : o_(offset), c_(count), array_size_(array_size), path_(path) {
        }

        template<typename T>
        ssize_t operator() (ColumnArrayBuffer<T> &buff) const {
            using boost::interprocess::unique_ptr;

            std::vector<int> shape = ColumnInfo::array_extent(array_size_);

            errno = 0;
            AutoFile fd(open(path_, O_RDONLY));
            if (fd == -1)
                throw io_exception(errno);
            
            size_t elems = 1;
            if (shape[shape.size() - 1] == -1) { // last extent variable

                for (int i = 0; i < shape.size()-1; ++i)
                    elems *= shape[i];
                

                for (size_t i = 0; i < o_; ++i) {
                    chunk_t c_size = 0;
                    errno = 0;
                    ssize_t count = read(fd, &c_size, sizeof (chunk_t));
                    if (count != sizeof (chunk_t))
                        throw io_exception(errno);

                    if (c_size % elems != 0)
                        throw das::data_corrupted();

                    off_t off = lseek(fd, c_size * sizeof (T), SEEK_CUR);
                    if (off == -1)
                        throw io_exception(errno);
                }

                for (size_t i = 0; i < c_; ++i) {
                    chunk_t c_size = 0;
                    errno = 0;
                    ssize_t count = read(fd, &c_size, sizeof (chunk_t));
                    if (count != sizeof (chunk_t))
                        throw io_exception(errno);

                    if (c_size % elems != 0)
                        throw das::data_corrupted();

                    unique_ptr<T, ArrayDeleter<T> > buffer(new T[c_size]);
                    count = read(fd, buffer.get(), c_size * sizeof (T));
                    if (count != c_size * sizeof (T))
                        throw io_exception(errno);

                    buff.add(buffer.release(), c_size);
                }

            } else {
                for (int i = 0; i < shape.size(); ++i)
                    elems *= shape[i];

                errno = 0;
                off_t off = lseek(fd, o_ * elems * sizeof (T), SEEK_SET);
                if (off == -1) {
                    throw io_exception(errno);
                }

                for (size_t i = 0; i < c_; ++i) {
                    unique_ptr<T, ArrayDeleter<T> > buffer(new T[elems]);
                    errno = 0;
                    ssize_t count = read(fd, buffer.get(), elems * sizeof (T));
                    if (count != elems * sizeof (T))
                        throw io_exception(errno);

                    buff.add(buffer.release(), elems);
                }
            }
            return buff.size();
        }

        //TODO

        ssize_t operator() (ColumnArrayBuffer<std::string> &buff) const {
            return 0;
        }

    private:
        size_t o_;
        size_t c_;
        std::string array_size_;
        const char *path_;
        const static size_t BUFF_SIZE = 512;
    };

    class RawStorageAccess_append_column : public boost::static_visitor<size_t> {
    public:

        RawStorageAccess_append_column(int file_desc, size_t count, bool include_size)
        : c_(count), f_(file_desc), include_size_(include_size) {
        }

        size_t operator() (std::string* buff) const {
            if (include_size_) {
                chunk_t size = c_;
                errno = 0;
                ssize_t count = write(f_, &size, sizeof (chunk_t));
                if (count == -1)
                    throw io_exception(errno);
            }
            for (size_t i = 0; i < c_; ++i) {
                size_t len = buff[i].length();
                errno = 0;
                ssize_t count = write(f_, buff[i].c_str(), len + 1);
                if (count == -1)
                    throw io_exception(errno);
            }
            return c_;
        }

        template<typename T>
        size_t operator() (T* buff) const {
            if (include_size_) {
                chunk_t size = c_;
                errno = 0;
                ssize_t count = write(f_, &size, sizeof (chunk_t));
                if (count == -1)
                    throw io_exception(errno);
            }
            errno = 0;
            ssize_t count = write(f_, buff, c_ * sizeof (T));
            if (count == -1)
                throw io_exception(errno);
            count /= sizeof (T);
            return count;
        }
    private:
        size_t c_;
        int f_;
        bool include_size_;
    };

    class RawStorageAccess_append_image : public boost::static_visitor<size_t> {
    public:

        RawStorageAccess_append_image(int file_desc, size_t count)
        : c_(count), f_(file_desc) {
        }

        template<typename T>
        size_t operator() (const T* buff) const {
            errno = 0;
            ssize_t count = write(f_, buff, c_ * sizeof (T));
            if (count == -1)
                throw io_exception(errno);
            count /= sizeof (T);
            return count;
        }
    private:
        size_t c_;
        int f_;
    };

    size_t
    RawStorageAccess::read_column(const std::string &col_name, ColumnFromFile* c,
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

    size_t
    RawStorageAccess::read_column_array(const std::string &col_name, ColumnFromFile* c,
            column_array_buffer_ptr &buffer, size_t offset, size_t count) {

        using boost::interprocess::unique_ptr;

        if (c->temp_path() != "")
            return boost::apply_visitor(
                RawStorageAccess_read_column_array(offset, count, c->get_array_size(), c->temp_path().c_str()),
                buffer);
        else
            if (c->fname() != "") {
            std::stringstream ss;
            ss << c->fname() << c->id();

            return boost::apply_visitor(
                    RawStorageAccess_read_column_array(offset, count, c->get_array_size(), ss.str().c_str()),
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

            errno = 0;
            AutoFile file_des(open(c_->temp_path().c_str(), O_WRONLY | O_APPEND));
            if (file_des == -1)
                throw io_exception(errno);

            bool is_variable = false;
            bool is_array_column = true;
            std::vector<int> sp  = ColumnInfo::array_extent(c_->get_array_size());
            if (sp[sp.size()-1] == -1)
                is_variable = true;
            if(sp.size() == 1 && sp[0] == 1)
                is_array_column = false;

            for (typename buckets_type::iterator it = bks.begin(); it != bks.end(); ++it) {

                StorageAccess::column_buffer_ptr buffer = it->first;
                size_t count = boost::apply_visitor(
                        RawStorageAccess_append_column(file_des, it->second, is_variable),
                        buffer);
                if(is_array_column)
                    size += 1; // we count the arrays we store
                else
                    size += it->second;
            }

            size += c_->file_size();
            c_->file_size(size);
            c_->buffer().clear();
        }

    private:
        ColumnFromFile* c_;
        RawStorageAccess* s_;
    };

    class RawStorageAccess_FlushImageBuffer : public boost::static_visitor<void> {
    public:

        RawStorageAccess_FlushImageBuffer(
                ImageFromFile *i,
                RawStorageAccess *s)
        : i_(i), s_(s) {

        }

        template<typename T>
        void operator() (T &native_type) const {
            typedef std::vector<ImageBufferEntry> buckets_type;
            const buckets_type &bks = i_->buffer().buckets();
            size_t tiles = 0;

            errno = 0;
            AutoFile file_des(open(i_->temp_path().c_str(), O_WRONLY | O_APPEND));
            if (file_des == -1)
                throw io_exception(errno);

            for (typename buckets_type::const_iterator it = bks.begin(); it != bks.end(); ++it) {

                StorageAccess::image_buffer_ptr buffer = it->data<T>();
                size_t count = boost::apply_visitor(
                        RawStorageAccess_append_image(file_des, it->num_elements()),
                        buffer);

                if (count != it->num_elements())
                    throw io_exception();

                tiles += it->shape()[0];
            }

            tiles += i_->file_tiles();
            i_->file_tiles(tiles);
            i_->buffer().clear();
        }

    private:
        ImageFromFile* i_;
        RawStorageAccess* s_;
    };

    void
    RawStorageTransaction::save(const std::string &path) {
        for (std::vector<DasObject*>::iterator obj_it = objs_.begin();
                obj_it != objs_.end(); ++obj_it) {
            std::map<std::string, ColumnFromFile*> map;
            DasObject* obj = *obj_it;

            if (obj->is_table()) {
                StorageTransaction::get_columns_from_file(obj, map);
                RawStorageAccess *rsa = dynamic_cast<RawStorageAccess*> (storage_access(obj));
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
                        if (path == "")
                            ss << rsa->get_default_path(true);
                        else
                            ss << rsa->get_custom_path(path, true);

                        ss << m_it->first << "_";

                        ColumnFromFile cffn(cff->file_size(), cff->get_type(), cff->get_array_size(), ss.str());
                        column_from_file(obj, c_name, cffn);

                        ColumnFromFile *cffnp = column_from_file(obj, c_name);
                        cffnp->persist(*(tb_.db()));

                        ss << cffnp->id();
                        errno = 0;
                        if (rename(temp_path.c_str(), ss.str().c_str())) {
                            throw das::io_exception(errno);
                        }

                    }
                }
            } else if (obj->is_image()) {
                ImageFromFile* iff = image_from_file(obj);

                if (iff == NULL)// empty image, skip
                    continue;

                RawStorageAccess *rsa = dynamic_cast<RawStorageAccess*> (storage_access(obj));

                rsa->flush_buffer(iff);
                std::string temp_path = iff->temp_path();
                if (temp_path != "") {
                    std::stringstream ss;
                    if (path == "")
                        ss << rsa->get_default_path(true);
                    else
                        ss << rsa->get_custom_path(path, true);

                    ImageFromFile iffn(iff->pixel_type(), ss.str());
                    unsigned int e0 = iff->file_tiles();
                    unsigned int e1 = iff->extent(1);
                    unsigned int e2 = iff->extent(2);
                    unsigned int e3 = iff->extent(3);
                    unsigned int e4 = iff->extent(4);
                    unsigned int e5 = iff->extent(5);
                    unsigned int e6 = iff->extent(6);
                    unsigned int e7 = iff->extent(7);
                    unsigned int e8 = iff->extent(8);
                    unsigned int e9 = iff->extent(9);
                    unsigned int e10 = iff->extent(10);

                    image_from_file(obj, iffn);

                    ImageFromFile *iffnp = image_from_file(obj);
                    iffnp->file_tiles(e0);
                    iffnp->extent(1, e1);
                    iffnp->extent(2, e2);
                    iffnp->extent(3, e3);
                    iffnp->extent(4, e4);
                    iffnp->extent(5, e5);
                    iffnp->extent(6, e6);
                    iffnp->extent(7, e7);
                    iffnp->extent(8, e8);
                    iffnp->extent(9, e9);
                    iffnp->extent(10, e10);
                    iffnp->persist(*(tb_.db()));

                    ss << iffnp->id();
                    errno = 0;
                    if (rename(temp_path.c_str(), ss.str().c_str())) {
                        throw das::io_exception(errno);
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
            if (obj->is_table()) {
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

                    if (cff == NULL || cff->fname() == "")
                        continue;

                    std::string str = cff->fname();
                    size_t found = str.find_last_of("/");
                    if (found < std::string::npos) {
                        storage_path = str.substr(0, found + 1); //we need the last '/'
                        break;
                    }

                }

                /*
                 * if we didn't find any suitable path, than fall back on 
                 * default path
                 */
                if (storage_path == "")
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

                        ColumnFromFile cffn(cff->file_size(), cff->get_type(), cff->get_array_size(), ss.str());
                        column_from_file(obj, c_name, cffn);

                        ColumnFromFile *cffnp = column_from_file(obj, c_name);
                        cffnp->persist(*(tb_.db()));

                        ss << cffnp->id();
                        errno = 0;
                        if (rename(temp_path.c_str(), ss.str().c_str())) {
                            throw das::io_exception(errno);
                        }

                    }
                }
            } else if (obj->is_image()) {
                std::string storage_path;

                ImageFromFile* iff = image_from_file(obj);
                if (iff == NULL) //image empty, skip
                    continue;

                RawStorageAccess *rsa = dynamic_cast<RawStorageAccess*> (storage_access(obj));

                rsa->flush_buffer(iff);

                std::string str = iff->fname();
                size_t found = str.find_last_of("/");
                if (found < std::string::npos) {
                    storage_path = str.substr(0, found);
                }
                if (storage_path == "")
                    storage_path = rsa->get_default_path(true);

                std::string temp_path = iff->temp_path();
                if (temp_path != "") {
                    std::stringstream ss;

                    ss << storage_path;

                    ImageFromFile iffn(iff->pixel_type(), ss.str());

                    unsigned int e0 = iff->file_tiles();
                    unsigned int e1 = iff->extent(1);
                    unsigned int e2 = iff->extent(2);
                    unsigned int e3 = iff->extent(3);
                    unsigned int e4 = iff->extent(4);
                    unsigned int e5 = iff->extent(5);
                    unsigned int e6 = iff->extent(6);
                    unsigned int e7 = iff->extent(7);
                    unsigned int e8 = iff->extent(8);
                    unsigned int e9 = iff->extent(9);
                    unsigned int e10 = iff->extent(10);

                    image_from_file(obj, iffn);

                    ImageFromFile *iffnp = image_from_file(obj);
                    iffnp->file_tiles(e0);
                    iffnp->extent(1, e1);
                    iffnp->extent(2, e2);
                    iffnp->extent(3, e3);
                    iffnp->extent(4, e4);
                    iffnp->extent(5, e5);
                    iffnp->extent(6, e6);
                    iffnp->extent(7, e7);
                    iffnp->extent(8, e8);
                    iffnp->extent(9, e9);
                    iffnp->extent(10, e10);

                    iffnp->persist(*(tb_.db()));

                    ss << iffnp->id();
                    errno = 0;
                    if (rename(temp_path.c_str(), ss.str().c_str())) {
                        throw das::io_exception(errno);
                    }

                }

            }
        }
    }

    void
    RawStorageAccess::flush_buffer(const std::string &col_name, ColumnFromFile* col) {
        if (col->buffer().empty()) return;
        if (col->temp_path() == "") {

            std::stringstream tmp;
            tmp << get_temp_path(true) << obj_->name() << "_" << col_name << "_XXXXXX";
            std::string str = tmp.str();

            char * c_str = new char [str.length() + 1];
            std::strcpy(c_str, str.c_str());

            // generates unique filename and opens it exclusively
            errno = 0;
            AutoFile dst_fd(mkstemp(c_str));
            if (dst_fd == -1) {
                delete[] c_str;
                throw io_exception(errno);
            }


            col->temp_path(c_str);
            delete[] c_str;

            if (col->fname() != "") {

                std::stringstream src;
                src << col->fname() << col->id();


                struct stat stat_buf;
                off_t offset = 0;

                AutoFile src_fd(open(src.str().c_str(), O_RDONLY));
                fstat(src_fd, &stat_buf);
                sendfile(dst_fd, src_fd, &offset, stat_buf.st_size);
            }
        }
        column_type col_t = DdlInfo::get_instance()->
                get_column_info(type_name(obj_), col_name).type_var_;
        boost::apply_visitor(
                RawStorageAccess_FlushColumnBuffer(col, this),
                col_t);
    }

    void

    RawStorageAccess::flush_buffer(ImageFromFile* img) {
        if (img->buffer().empty()) return;
        if (img->temp_path() == "") {
            std::stringstream tmp;
            tmp << get_temp_path(true) << obj_->name() << "_XXXXXX";
            std::string str = tmp.str();
            size_t len = str.length();
            char *c_str = new char[len + 1];
            str.copy(c_str, len);
            c_str[len] = '\0';
            // generates unique filename and opens it exclusively
            errno = 0;
            AutoFile dst_fd(mkstemp(c_str));
            if (dst_fd == -1) {
                delete[] c_str;
                throw io_exception(errno);
            }


            img->temp_path(c_str);
            delete[] c_str;

            if (img->fname() != "") {

                std::stringstream src;
                src << img->fname() << img->id();

                struct stat stat_buf;
                off_t offset = 0;

                AutoFile src_fd(open(src.str().c_str(), O_RDONLY));
                fstat(src_fd, &stat_buf);
                sendfile(dst_fd, src_fd, &offset, stat_buf.st_size);
            }
        }
        image_type img_t = DdlInfo::get_instance()->
                get_image_info(type_name(obj_)).type_var_;
        boost::apply_visitor(
                RawStorageAccess_FlushImageBuffer(img, this),
                img_t);
    }

    class RawStorageAccess_read_image : public boost::static_visitor<ssize_t> {
    public:

        RawStorageAccess_read_image(
                const ImageFromFile *i,
                const das::TinyVector<int, 11> &offset,
                const das::TinyVector<int, 11> &count,
                const das::TinyVector<int, 11> &stride,
                const char* path)
        : i_(i), off_(offset), cnt_(count), str_(stride), path_(path) {
        }

        template<typename T>
        ssize_t operator() (T* buff) const {
            errno = 0;
            AutoFile fd(open(path_, O_RDONLY));
            if (fd == -1)
                throw io_exception(errno);
            size_t count = 0;

            das::TinyVector<size_t, 11> shape(
                    i_->extent(0),
                    i_->extent(1),
                    i_->extent(2) == 0 ? 1 : i_->extent(2),
                    i_->extent(3) == 0 ? 1 : i_->extent(3),
                    i_->extent(4) == 0 ? 1 : i_->extent(4),
                    i_->extent(5) == 0 ? 1 : i_->extent(5),
                    i_->extent(6) == 0 ? 1 : i_->extent(6),
                    i_->extent(7) == 0 ? 1 : i_->extent(7),
                    i_->extent(8) == 0 ? 1 : i_->extent(8),
                    i_->extent(9) == 0 ? 1 : i_->extent(9),
                    i_->extent(10) == 0 ? 1 : i_->extent(10)
                    );

            size_t dsp[] = {
                /*dim 0*/ shape[1] * shape[2] * shape[3] * shape[4] * shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
                /*dim 1*/ shape[2] * shape[3] * shape[4] * shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
                /*dim 2*/ shape[3] * shape[4] * shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
                /*dim 3*/ shape[4] * shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
                /*dim 4*/ shape[5] * shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
                /*dim 5*/ shape[6] * shape[7] * shape[8] * shape[9] * shape[10],
                /*dim 6*/ shape[7] * shape[8] * shape[9] * shape[10],
                /*dim 7*/ shape[8] * shape[9] * shape[10],
                /*dim 8*/ shape[9] * shape[10],
                /*dim 9*/ shape[10],
                /*dim10*/ 1
            };
            DAS_DBG_NO_SCOPE(
                    size_t DBG_file_size = i_->file_tiles() * shape[1] *
                    shape[2] * shape[3] * shape[4] * shape[5] *
                    shape[6] * shape[7] * shape[8] * shape[9] *
                    shape[10] * sizeof (T);
                    );

            off_t seeks[11];
            seeks[0] = 0;
            for (size_t d0 = 0; d0 < cnt_[0]; ++d0) {
                seeks[0] += d0 == 0 ? dsp[0] * off_[0] : dsp[0] * str_[0];
                seeks[1] = seeks[0];
                for (size_t d1 = 0; d1 < cnt_[1]; ++d1) {
                    seeks[1] += d1 == 0 ? dsp[1] * off_[1] : dsp[1] * str_[1];
                    seeks[2] = seeks[1];
                    for (size_t d2 = 0; d2 < cnt_[2]; ++d2) {
                        seeks[2] += d2 == 0 ? dsp[2] * off_[2] : dsp[2] * str_[2];
                        seeks[3] = seeks[2];
                        for (size_t d3 = 0; d3 < cnt_[3]; ++d3) {
                            seeks[3] += d3 == 0 ? dsp[3] * off_[3] : dsp[3] * str_[3];
                            seeks[4] = seeks[3];
                            for (size_t d4 = 0; d4 < cnt_[4]; ++d4) {
                                seeks[4] += d4 == 0 ? dsp[4] * off_[4] : dsp[4] * str_[4];
                                seeks[5] = seeks[4];
                                for (size_t d5 = 0; d5 < cnt_[5]; ++d5) {
                                    seeks[5] += d5 == 0 ? dsp[5] * off_[4] : dsp[5] * str_[5];
                                    seeks[6] = seeks[5];
                                    for (size_t d6 = 0; d6 < cnt_[6]; ++d6) {
                                        seeks[6] += d6 == 0 ? dsp[6] * off_[6] : dsp[6] * str_[6];
                                        seeks[7] = seeks[6];
                                        for (size_t d7 = 0; d7 < cnt_[7]; ++d7) {
                                            seeks[7] += d7 == 0 ? dsp[7] * off_[7] : dsp[7] * str_[7];
                                            seeks[8] = seeks[7];
                                            for (size_t d8 = 0; d8 < cnt_[8]; ++d8) {
                                                seeks[8] += d8 == 0 ? dsp[8] * off_[8] : dsp[8] * str_[8];
                                                seeks[9] = seeks[8];
                                                for (size_t d9 = 0; d9 < cnt_[9]; ++d9) {
                                                    seeks[9] += d9 == 0 ? dsp[9] * off_[9] : dsp[9] * str_[9];
                                                    seeks[10] = seeks[9];
                                                    for (size_t d10 = 0; d10 < cnt_[10]; ++d10) {
                                                        seeks[10] += d10 == 0 ? dsp[10] * off_[10] : dsp[10] * str_[10];
                                                        DAS_DBG_NO_SCOPE(
                                                                size_t DBG_offset = seeks[10];
                                                        if (DBG_offset * sizeof (T) > DBG_file_size) {
                                                            DAS_LOG_DBG("IMAGE bad seek OVERFLOW!!! requested " << DBG_offset << " in file length " << DBG_file_size);
                                                            return count;
                                                        }
                                                        );
                                                        size_t file_pos = seeks[10] * sizeof (T);
                                                        errno = 0;
                                                        off_t off = lseek(fd, file_pos, SEEK_SET);
                                                        if (off == -1) {
                                                            throw io_exception(errno);
                                                        }
                                                        T* ptr = buff + count;
                                                        errno = 0;
                                                        ssize_t c = read(fd, ptr, sizeof (T));
                                                        DAS_LOG_DBG("buffer[" << count << "] = " << *ptr);
                                                        if (c == -1) {
                                                            throw io_exception(errno);
                                                        }
                                                        ++count;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            }
            return count;
        }

    private:
        const ImageFromFile *i_;
        const das::TinyVector<int, 11> &off_;
        const das::TinyVector<int, 11> &cnt_;
        const das::TinyVector<int, 11> &str_;
        const char* path_;

    };

    size_t
    RawStorageAccess::read_image(
            ImageFromFile* i,
            image_buffer_ptr buffer,
            const das::TinyVector<int, 11> &offset,
            const das::TinyVector<int, 11> &count,
            const das::TinyVector<int, 11> &stride
            ) {

        if (i->temp_path() != "")
            return boost::apply_visitor(
                RawStorageAccess_read_image(i, offset, count, stride, i->temp_path().c_str()),
                buffer);
        else
            if (i->fname() != "") {
            std::stringstream ss;
            ss << i->fname() << i->id();

            return boost::apply_visitor(
                    RawStorageAccess_read_image(i, offset, count, stride, ss.str().c_str()),
                    buffer);
        }

        return 0;


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