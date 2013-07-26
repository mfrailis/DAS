#include "internal/column_buffer.hpp"
#include <iostream>

ColumnBuffer::ColumnBuffer(const std::string &type) : is_init_(false) {
    init(type);
}

ColumnBuffer::ColumnBuffer() : is_init_(false) {}

void
ColumnBuffer::init(const std::string &type) {
    if(is_init_){
        std::cout << "type already initialized" << std::endl;
        throw std::exception(); 
    }
    
    if (type == "byte" || type == "char") {
        buffer_ = std::vector< das::Array<char> >();
    } else if (type == "int16") {
        buffer_ = std::vector< das::Array<short> >();
    } else if (type == "int32") {
        buffer_ = std::vector< das::Array<int> >();
    } else if (type == "int64") {
        buffer_ = std::vector< das::Array<long long> >();
    } else if (type == "float32") {
        buffer_ = std::vector< das::Array<float> >();
    } else if (type == "float64") {
        buffer_ = std::vector< das::Array<double> >();
    } else if (type == "boolean") {
        buffer_ = std::vector < das::Array<bool> >();
    } else if (type == "uint8") {
        buffer_ = std::vector< das::Array<unsigned char> >();
    } else if (type == "uint16") {
        buffer_ = std::vector< das::Array<unsigned short> >();
    } else if (type == "uint32") {
        buffer_ = std::vector< das::Array<unsigned int> >();
    } else if (type == "string") {
        buffer_ = std::vector< das::Array<std::string> >();
    } else {
        std::cout << "type not supported" << std::endl;
        throw std::exception();
    }
    is_init_=true;
}

bool ColumnBuffer::empty() {
    if(!is_init_){
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();   
    }
    
    if(buffer_.empty())
        return true;
    else
        return size() == 0;
}

class ColumnBuffer_size : public boost::static_visitor<size_t> {
public:

    template<typename T>
    size_t operator() (T &vec) const {
        size_t s = 0;
        for (typename T::iterator it = vec.begin(); it != vec.end(); ++it)
            s += it->size();
        return s;
    }
};

size_t ColumnBuffer::size() {
    if(!is_init_){
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();   
    }
    return boost::apply_visitor(ColumnBuffer_size(), buffer_);
}