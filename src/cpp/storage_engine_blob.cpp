#include "../../include/internal/storage_engine_blob.hpp"
#include "../../include/exceptions.hpp"
#include "../../include/ddl/info.hpp"
#include "../../include/das_object.hpp"
#include "../../include/internal/storage_engine.ipp"
#include "internal/log.hpp"
#include "../../include/internal/database_config.hpp"
#include <vector>
#include <stdio.h>
#include <string.h>

enum SeekEnum {
    se_SET = 0, se_CUR = 1
};

class BufferStream {
public:

    BufferStream(const ColumnFromBlob::blob_type& blob) : blob_(blob), off_(0) {
        size_ = blob_.size();
    }

    void
    read(void* dest, size_t count) {
        if (off_ + count > size_)
            throw das::io_exception();

        const char * data = &(blob_.front());
        memcpy(dest, data + off_, count);
        off_ += count;
    }

    void
    seek(size_t count, SeekEnum mode) {
        size_t o = off_;
        switch (mode) {
            case se_SET:
                o = count;
                break;
            case se_CUR:
                o += count;
                break;
            default: //unimplemented mode
                throw das::io_exception();

        }
        if (o > size_)
            throw das::io_exception();
        off_ = o;
    }


private:
    const ColumnFromBlob::blob_type& blob_;
    size_t off_;
    size_t size_;
};

/* class for managing files containing strings
 */

class BlobStringReader {
public:

    BlobStringReader(ColumnFromBlob::blob_type& blob)
    : blob_(blob), off_(0) {
        size_ = blob_.size();
    }

    void skip(size_t offset) {
        size_t terms = 0;
        while (terms < offset) {
            if (off_ >= size_)
                throw das::io_exception();
            while (terms < offset && off_ < size_)
                if (blob_[off_++] == '\0')
                    ++terms;
        }
    }

    void copy(std::string* array, size_t c_) {
        size_t i = 0;
        while (i < c_) {
            if (off_ > size_)
                throw das::io_exception();
            if (blob_[off_] == '\0') {
                ++off_;
                ++i;
            } else {
                array[i].push_back(blob_[off_++]);
            }
        }
    }

    chunk_t read_size() {
        chunk_t dim = 0;
        if (off_ + sizeof (chunk_t) > size_)
            throw das::io_exception();
        char* data = &(blob_.front());
        memcpy(&dim, data + off_, sizeof (chunk_t));
        off_ += sizeof (chunk_t);

        return dim;
    }

private:
    ColumnFromBlob::blob_type& blob_;
    size_t off_;
    size_t size_;
};

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
                /*ImageFromFile* iff = image_ptr(obj);
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
            T* data = (T*) & blob_.front();

            data += o_;
            for (size_t i = 0; i < c_; ++i)
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

        ColumnFromBlob * c = dynamic_cast<ColumnFromBlob *> (col);

        return boost::apply_visitor(
                BlobStorageAccess_read_column(offset, count, c->blob()),
                buffer);
    }

    class BlobStorageAccess_read_column_array : public boost::static_visitor<ssize_t> {
    public:

        BlobStorageAccess_read_column_array(size_t offset, size_t count, const std::string &array_size, ColumnFromBlob::blob_type& blob)
        : o_(offset), c_(count), array_size_(array_size), blob_(blob) {
        }

        template<typename T>
        ssize_t operator() (ColumnArrayBuffer<T> &buff) const {
            using boost::interprocess::unique_ptr;

            std::vector<int> shape = ColumnInfo::array_extent(array_size_);

            BufferStream bs(blob_);
            size_t elems = 1;
            if (shape[shape.size() - 1] == -1) { // last extent variable

                for (int i = 0; i < shape.size() - 1; ++i)
                    elems *= shape[i];


                for (size_t i = 0; i < o_; ++i) {
                    chunk_t c_size = 0;
                    bs.read(&c_size, sizeof (chunk_t));

                    if (c_size % elems != 0)
                        throw das::data_corrupted();

                    bs.seek(c_size * sizeof (T), se_CUR);
                }

                for (size_t i = 0; i < c_; ++i) {
                    chunk_t c_size = 0;
                    bs.read(&c_size, sizeof (chunk_t));

                    if (c_size % elems != 0)
                        throw das::data_corrupted();

                    unique_ptr<T, ArrayDeleter<T> > buffer(new T[c_size]);
                    bs.read(buffer.get(), c_size * sizeof (T));

                    buff.add(buffer.release(), c_size);
                }

            } else {
                for (int i = 0; i < shape.size(); ++i)
                    elems *= shape[i];

                bs.seek(o_ * elems * sizeof (T), se_SET);

                for (size_t i = 0; i < c_; ++i) {
                    unique_ptr<T, ArrayDeleter<T> > buffer(new T[elems]);

                    bs.read(buffer.get(), elems * sizeof (T));

                    buff.add(buffer.release(), elems);
                }
            }
            return buff.size();
        }

        ssize_t operator() (ColumnArrayBuffer<std::string> &buff) const {
            using boost::interprocess::unique_ptr;

            std::vector<int> shape = ColumnInfo::array_extent(array_size_);

            BlobStringReader SR(blob_);

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
    };

    size_t
    BlobStorageAccess::read_column_array(
            const std::string &col_name,
            Column* col,
            column_array_buffer_ptr &buffer,
            size_t offset,
            size_t count
            ) {
        ColumnFromBlob * c = dynamic_cast<ColumnFromBlob *> (col);
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

    class BlobStorageAccess_append_image : public boost::static_visitor<size_t> {
    public:

        BlobStorageAccess_append_image(ImageBlob::blob_type& blob, size_t count)
        : c_(count), blob_(blob) {
        }

        template<typename T>
        size_t operator() (const T* buff) const {
            char *bc = (char*) buff;
            blob_.insert(blob_.end(), bc, bc + c_ * sizeof (T));
            return c_;
        }
    private:
        size_t c_;
        ImageBlob::blob_type& blob_;
    };

    class BlobStorageAccess_FlushImageBuffer : public boost::static_visitor<void> {
    public:

        BlobStorageAccess_FlushImageBuffer(
                ImageBlob *i,
                BlobStorageAccess *s)
        : i_(i), s_(s) {

        }

        template<typename T>
        void operator() (T &native_type) const {
            typedef std::vector<ImageBufferEntry> buckets_type;
            const buckets_type &bks = i_->buffer().buckets();
            size_t tiles = 0;

            for (typename buckets_type::const_iterator it = bks.begin(); it != bks.end(); ++it) {

                StorageAccess::image_buffer_ptr buffer = it->data<T>();
                size_t count = boost::apply_visitor(
                        BlobStorageAccess_append_image(i_->blob(), it->num_elements()),
                        buffer);

                if (count != it->num_elements())
                    throw io_exception();

                tiles += it->shape()[0];
            }

            tiles += i_->store_tiles();
            i_->store_tiles(tiles);
            i_->buffer().clear();
        }

    private:
        ImageBlob* i_;
        BlobStorageAccess* s_;
    };

    void
    BlobStorageAccess::flush_buffer(Image* img) {
        ImageBlob* i = dynamic_cast<ImageBlob*> (img);
        if (i->buffer().empty()) return;
        image_type img_t = DdlInfo::get_instance()->
                get_image_info(type_name(obj_)).type_var_;
        boost::apply_visitor(
                BlobStorageAccess_FlushImageBuffer(i, this),
                img_t);
    }

    class BlobStorageAccess_read_image : public boost::static_visitor<ssize_t> {
    public:

        BlobStorageAccess_read_image(
                const ImageBlob *i,
                const das::TinyVector<int, 11> &offset,
                const das::TinyVector<int, 11> &count,
                const das::TinyVector<int, 11> &stride)
        : i_(i), off_(offset), cnt_(count), str_(stride) {
        }

        template<typename T>
        ssize_t operator() (T* buff) const {

            BufferStream store(i_->blob());
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
                    size_t DBG_file_size = i_->store_tiles() * shape[1] *
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
                                                        store.seek(file_pos,se_SET);
                                                        //off_t off = lseek(fd, file_pos, SEEK_SET);
                                                        T* ptr = buff + count;
                                                        errno = 0;
                                                        store.read(ptr, sizeof (T));
                                                        //ssize_t c = read(fd, ptr, sizeof (T));
                                                        DAS_LOG_DBG("buffer[" << count << "] = " << *ptr);
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
        const ImageBlob *i_;
        const das::TinyVector<int, 11> &off_;
        const das::TinyVector<int, 11> &cnt_;
        const das::TinyVector<int, 11> &str_;
    };

    size_t
    BlobStorageAccess::read_image(
            Image* img,
            image_buffer_ptr buffer,
            const das::TinyVector<int, 11> &offset,
            const das::TinyVector<int, 11> &count,
            const das::TinyVector<int, 11> &stride
            ) {
        ImageBlob* i = dynamic_cast<ImageBlob*> (img);

        return boost::apply_visitor(
                BlobStorageAccess_read_image(i, offset, count, stride),
                buffer);
    }
}