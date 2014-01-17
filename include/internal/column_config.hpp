#ifndef COLUMN_CONFIG_HPP
#define	COLUMN_CONFIG_HPP

#include "../ddl/column.hpp"

class ColumnConfig {
public:    
    virtual
    ColumnFile *
    column_from_file() const = 0;

    virtual
    void
    column_from_file(const ColumnFile &cff) = 0;
    
    virtual
    const std::string&
    column_name() const = 0;
};
#endif	/* COLUMN_CONFIG_HPP */

