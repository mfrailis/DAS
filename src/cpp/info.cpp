#include "ddl/info.hpp"

boost::unordered_map< std::string, TypeInfo > DdlInfo::all_types_;

DdlInfo* DdlInfo::instance_ = 0;

std::vector<int>
ColumnInfo::array_extent(const std::string &array_size) {
    std::vector<int> v;

    if (array_size == "*") {
        v.push_back(-1);
        return v;
    }

    std::istringstream stream(array_size);
    int ext = 0;
    char c;
    while (stream) {
        if (!(stream >> ext))
            break;

        v.push_back(ext);
        stream.get(c);
    }

    if (array_size[array_size.length() - 1] == '*') {
        if (c == '*')
            v[v.size() - 1] = -1;
        else
            v.push_back(-1);
    }

    return v;
}