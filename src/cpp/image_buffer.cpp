#include "internal/image_buffer.hpp"
#include "ddl/image.hpp"
#include <iostream>

unsigned int
ImageBuffer::num_elements() const {
    unsigned int elems = size0_;
    
    for(size_t i=1; i<iff_->rank(); ++i)
        elems *= iff_->extent(i);
    
    return elems;

}

void
ImageBuffer::init(const std::string &type) {
    if(is_init_){
        std::cout << "type already initialized" << std::endl;
        throw std::exception(); 
    }
    
    if (type == "byte" ) {
        type_ = static_cast<char> (0);
    } else if (type == "int16") {
        type_ = static_cast<short> (0);
    } else if (type == "int32") {
        type_ = static_cast<int> (0);
    } else if (type == "float32") {
        type_ = static_cast<float> (0);
    } else if (type == "float64") {
        type_ = static_cast<double> (0);
    } else {
        std::cout << "type not supported" << std::endl;
        throw std::exception();
    }
    is_init_=true;
}

bool ImageBuffer::empty() {
    if(!is_init_){
        std::cout << "buffer type uninitialized" << std::endl;
        throw std::exception();   
    }
    
    if(buffer_.empty())
        return true;
    else
        return num_elements() == 0; 
}
