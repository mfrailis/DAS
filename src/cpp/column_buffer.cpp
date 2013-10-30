#include "internal/column_buffer.hpp"
#include "ddl/info.hpp"
#include <iostream>

ColumnBuffer::ColumnBuffer(const std::string &type, const std::string &array_size)
: is_init_type_(false), is_init_shape_(false), shape_(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) {
    init_type(type);
    init_shape(array_size);
}

ColumnBuffer::ColumnBuffer() : is_init_type_(false), is_init_shape_(false), shape_(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) {
}

void
ColumnBuffer::init_type(const std::string &type) {
    if (is_init_type_) {
        std::cout << "type already initialized" << std::endl;
        throw std::exception();
    }

    if (type == "byte" || type == "char") {
        buffer_ = std::vector< das::ArrayStore<char> >();
    } else if (type == "int16") {
        buffer_ = std::vector< das::ArrayStore<short> >();
    } else if (type == "int32") {
        buffer_ = std::vector< das::ArrayStore<int> >();
    } else if (type == "int64") {
        buffer_ = std::vector< das::ArrayStore<long long> >();
    } else if (type == "float32") {
        buffer_ = std::vector< das::ArrayStore<float> >();
    } else if (type == "float64") {
        buffer_ = std::vector< das::ArrayStore<double> >();
    } else if (type == "boolean") {
        buffer_ = std::vector < das::ArrayStore<bool> >();
    } else if (type == "uint8") {
        buffer_ = std::vector< das::ArrayStore<unsigned char> >();
    } else if (type == "uint16") {
        buffer_ = std::vector< das::ArrayStore<unsigned short> >();
    } else if (type == "uint32") {
        buffer_ = std::vector< das::ArrayStore<unsigned int> >();
    } else if (type == "string") {
        buffer_ = std::vector< das::ArrayStore<std::string> >();
    } else {
        std::cout << "type not supported" << std::endl;
        throw std::exception();
    }

    is_init_type_ = true;
}

void
ColumnBuffer::init_shape(const std::string &array_size) {
    if (is_init_shape_) {
        std::cout << "type already initialized" << std::endl;
        throw std::exception();
    }

    std::vector<int> parsed = ColumnInfo::array_extent(array_size);

    rank_ = parsed.size();

    for (size_t i = 0; i < rank_; ++i)
        shape_(i) = parsed[(rank_ - i) - 1];

    is_init_shape_ = true;
}

bool ColumnBuffer::empty() {
    if (!is_init()) {
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();
    }

    if (buffer_.empty())
        return true;
    else
        return size() == 0;
}

class ColumnBuffer_size : public boost::static_visitor<size_t> {
public:

    ColumnBuffer_size(bool is_column_array) : is_column_array_(is_column_array) {

    }

    template<typename T>
    size_t operator() (T &vec) const {
        if (is_column_array_)
            return vec.size();
        
        size_t s = 0;
        for (typename T::const_iterator it = vec.begin(); it != vec.end(); ++it)
            s += it->size();
        return s;
    }

private:
    bool is_column_array_;
};

size_t ColumnBuffer::size() const {
    if (!is_init()) {
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();
    }
    if (rank_ == 1 && shape_(0) == 1)
        return boost::apply_visitor(ColumnBuffer_size(false), buffer_);
    else
        return boost::apply_visitor(ColumnBuffer_size(true), buffer_);
}