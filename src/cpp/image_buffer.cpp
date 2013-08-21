#include "internal/image_buffer.hpp"
#include <iostream>


void
ImageBuffer::init(const std::string &type) {
    if(is_init_){
        std::cout << "type already initialized" << std::endl;
        throw std::exception(); 
    }
    
    if (type == "byte" ) {
        type_ = static_cast<char*> (NULL);
    } else if (type == "int16") {
        type_ = static_cast<short*> (NULL);
    } else if (type == "int32") {
        type_ = static_cast<int*> (NULL);
    } else if (type == "float32") {
        type_ = static_cast<float*> (NULL);
    } else if (type == "float64") {
        type_ = static_cast<double*> (NULL);
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
