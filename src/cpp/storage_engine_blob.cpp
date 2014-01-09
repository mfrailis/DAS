#include "../../include/internal/storage_engine_blob.hpp"
#include "../../include/exceptions.hpp"
#include "../../include/ddl/info.hpp"
#include "../../include/das_object.hpp"
#include "../../include/internal/storage_engine.ipp"
#include "internal/log.hpp"
#include "../../include/internal/database_config.hpp"
#include <vector>

namespace das {

    class BlobStorageAccess_append_column : public boost::static_visitor<size_t> {
    public:

        BlobStorageAccess_append_column(ColumnFromBlob::blob_type& blob, size_t count, bool include_size)
        : c_(count), blob_(blob), include_size_(include_size) {
        }

        size_t operator() (std::string* buff) const {
            if (include_size_) {
                chunk_t size = c_;
                char * b = (char*) &size;
                blob_.insert(blob_.end(), b, b + sizeof (chunk_t));
            }
            for (size_t i = 0; i < c_; ++i) {
                size_t len = buff[i].length();
                const char * str = buff[i].c_str();
                blob_.insert(blob_.end(), str, str + len + 1);
            }
            return c_;
        }

        template<typename T>
        size_t operator() (T* buff) const {
            if (include_size_) {
                chunk_t size = c_;
                char * b = (char*) &size;
                blob_.insert(blob_.end(), b, b + sizeof (chunk_t));
            }
            char *bc = (char*) buff;
            blob_.insert(blob_.end(), bc, bc + c_ * sizeof (T));
            return c_;
        }
    private:
        size_t c_;
        ColumnFromBlob::blob_type& blob_;
        bool include_size_;
    };

    class BlobStorageAccess_FlushColumnBuffer : public boost::static_visitor<void> {
    public:

        BlobStorageAccess_FlushColumnBuffer(
                ColumnFromBlob *c,
                BlobStorageAccess *s)
        : c_(c), s_(s) {
        }

        template<typename T>
        void operator() (T &native_type) const {
            typedef std::vector<std::pair<T*, size_t> > buckets_type;
            buckets_type bks = c_->buffer().buckets<T>();
            size_t size = 0;

            bool is_variable = false;
            bool is_array_column = true;
            std::vector<int> sp = ColumnInfo::array_extent(c_->get_array_size());
            if (sp[sp.size() - 1] == -1)
                is_variable = true;
            if (sp.size() == 1 && sp[0] == 1)
                is_array_column = false;

            for (typename buckets_type::iterator it = bks.begin(); it != bks.end(); ++it) {

                StorageAccess::column_buffer_ptr buffer = it->first;
                size_t count = boost::apply_visitor(
                        BlobStorageAccess_append_column(c_->blob(), it->second, is_variable),
                        buffer);
                if (is_array_column)
                    size += 1; // we count the arrays we store
                else
                    size += it->second;
            }

            size += c_->store_size();
            c_->store_size(size);
            c_->buffer().clear();
        }

    private:
        ColumnFromBlob* c_;
        BlobStorageAccess* s_;
    };

    void
    BlobStorageTransaction::add(DasObject *ptr) {
        objs_.push_back(ptr);
    }

    void
    BlobStorageTransaction::add(DasObject *ptr, const Extension &e) {
        objs_.push_back(ptr);
    }

    void
    BlobStorageTransaction::save() {
        for (std::vector<DasObject*>::iterator obj_it = objs_.begin();
                obj_it != objs_.end(); ++obj_it) {
            DasObject* obj = *obj_it;
            std::map<std::string, Column*> map;
            if (obj->is_table()) {
                StorageTransaction::get_columns_from_file(obj, map);
                BlobStorageAccess *rsa = dynamic_cast<BlobStorageAccess*> (storage_access(obj));
                for (std::map<std::string, Column*>::iterator m_it = map.begin();
                        m_it != map.end(); ++m_it) {
                    std::string c_name = m_it->first;
                    ColumnFromBlob* cfb = dynamic_cast<ColumnFromBlob*> (m_it->second);

                    if (cfb == NULL) // empty column, skip
                        continue;

                    rsa->flush_buffer(c_name, cfb);
                }
            } else if (obj->is_image()) {
                //TODO
                /*ImageFromFile* iff = image_from_file(obj);
                if (iff == NULL) //image empty, skip
                    continue;

                RawStorageAccess *rsa = dynamic_cast<RawStorageAccess*> (storage_access(obj));

                rsa->flush_buffer(iff);
                 */
            }
        }
    }

    class BlobStorageAccess_read_column : public boost::static_visitor<ssize_t> {
    public:

        BlobStorageAccess_read_column(size_t offset, size_t count, ColumnFromBlob::blob_type& blob)
        : o_(offset), c_(count), blob_(blob) {
        }

        ssize_t operator() (std::string* buff) const {
            size_t terms_ = 0;
            size_t off = 0;
            ssize_t count = 0;
            errno = 0;
            count = blob_.size();

            while (terms_ < o_) {
                off = 0;
                while (terms_ < o_ && off < count)
                    if (blob_[off++] == '\0')
                        ++terms_;
            }
            size_t i = 0;
            while (i < c_) {
                while (off < count) {
                    if (blob_[off] == '\0') {
                        ++off;
                        ++i;
                        break;
                    } else
                        buff[i].push_back(blob_[off++]);
                }
            }
            return c_;
        }

        template<typename T>
        ssize_t operator() (T* buff) const {
            T* data = (T*) &blob_.front(); 

            data += o_;
            for(size_t i= 0; i<c_; ++i)
                buff[i] = data[i];

            return c_;
        }

    private:
        size_t o_;
        size_t c_;
        ColumnFromBlob::blob_type& blob_;
    };  
    
    size_t
    BlobStorageAccess::read_column(
            const std::string &col_name,
            Column* col,
            column_buffer_ptr buffer,
            size_t offset,
            size_t count
            ) {

        ColumnFromBlob * c = dynamic_cast<ColumnFromBlob *>(col);

        return boost::apply_visitor(
                BlobStorageAccess_read_column(offset, count, c->blob()),
                buffer);
    }

    class BlobStorageAccess_read_column_array : public boost::static_visitor<ssize_t> {
    public:

        BlobStorageAccess_read_column_array(size_t offset, size_t count, const std::string &array_size,  ColumnFromBlob::blob_type& blob)
        : o_(offset), c_(count), array_size_(array_size), blob_(blob) {
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

                for (int i = 0; i < shape.size() - 1; ++i)
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

        ssize_t operator() (ColumnArrayBuffer<std::string> &buff) const {
            using boost::interprocess::unique_ptr;

            std::vector<int> shape = ColumnInfo::array_extent(array_size_);

            errno = 0;
            int file_desc = open(path_, O_RDONLY);
            if (file_desc == -1)
                throw io_exception(errno);

            StringReader SR(file_desc);

            size_t elems = 1;
            if (shape[shape.size() - 1] == -1) { // last extent variable

                for (int i = 0; i < shape.size() - 1; ++i)
                    elems *= shape[i];


                for (size_t i = 0; i < o_; ++i) {
                    chunk_t c_size = SR.read_size();

                    if (c_size % elems != 0)
                        throw das::data_corrupted();

                    SR.skip(c_size);
                }

                for (size_t i = 0; i < c_; ++i) {
                    chunk_t c_size = SR.read_size();

                    if (c_size % elems != 0)
                        throw das::data_corrupted();

                    unique_ptr<std::string, ArrayDeleter<std::string> >
                            buffer(new std::string[c_size]);
                    SR.copy(buffer.get(), c_size);
                    buff.add(buffer.release(), c_size);
                }

            } else {
                for (int i = 0; i < shape.size(); ++i)
                    elems *= shape[i];

                SR.skip(elems * o_);

                for (size_t i = 0; i < c_; ++i) {
                    unique_ptr<std::string, ArrayDeleter<std::string> >
                            buffer(new std::string[elems]);
                    SR.copy(buffer.get(), elems);
                    buff.add(buffer.release(), elems);
                }
            }
            return buff.size();
        }

    private:
        size_t o_;
        size_t c_;
        std::string array_size_;
        ColumnFromBlob::blob_type& blob_;
        const static size_t BUFF_SIZE = 512;
    };


    size_t
    BlobStorageAccess::read_column_array(
            const std::string &col_name,
            Column* col,
            column_array_buffer_ptr &buffer,
            size_t offset,
            size_t count
            ) {
        ColumnFromBlob * c = dynamic_cast<ColumnFromBlob *>(col);
        return boost::apply_visitor(
                    BlobStorageAccess_read_column_array(offset, count, c->get_array_size(), c->blob()),
                    buffer);
    }

    void
    BlobStorageAccess::flush_buffer(
            const std::string &col_name,
            Column* c
            ) {
        ColumnFromBlob* col = dynamic_cast<ColumnFromBlob*> (c);
        if (col->buffer().empty()) return;
        column_type col_t = DdlInfo::get_instance()->
                get_column_info(type_name(obj_), col_name).type_var_;
        boost::apply_visitor(
                BlobStorageAccess_FlushColumnBuffer(col, this),
                col_t);
    }

    void
    BlobStorageAccess::flush_buffer(ImageFromFile* img) {
        //TODO
    }

    size_t
    BlobStorageAccess::read_image(
            ImageFromFile* col,
            image_buffer_ptr buffer,
            const das::TinyVector<int, 11> &offset,
            const das::TinyVector<int, 11> &count,
            const das::TinyVector<int, 11> &stride
            ) {
        //TODO
        return 0;
    }
}