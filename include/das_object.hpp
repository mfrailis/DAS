#ifndef DAS_DASOBJECT_HPP
#define DAS_DASOBJECT_HPP

#include <odb/core.hxx>
#include <string>
#include "ddl/info.hpp"
#include "ddl/column.hpp"
#include "ddl/image.hpp"
#include <odb/database.hxx>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>
#include "internal/db_bundle.hpp"
#include "internal/array.hpp"
#include "storage_engine.hpp"

using std::tr1::shared_ptr;

class QLVisitor;
namespace das {
    namespace tpl {
        class Database;
        class Transaction;
        template<typename T>
        class result_iterator;
    }
}

#pragma db object abstract

class DasObject {
public:

    const KeywordInfo&
    get_keyword_info(std::string keyword_name) throw (std::out_of_range) {
        return DdlInfo::get_instance()->
                get_keyword_info(type_name_, keyword_name);
    }

    const ColumnInfo&
    get_column_info(std::string column_name) throw (std::out_of_range) {
        return DdlInfo::get_instance()->
                get_column_info(type_name_, column_name);
    }

    bool is_dirty() const {
        return is_dirty_;
    }

    bool is_new() const {
        return das_id_ == 0;
    }

    const std::string&
    dbUserId() const {
        return dbUserId_;
    }

    const long long&
    creationDate() const {
        return creationDate_;
    }

    void
    creationDate(long long &creationDate) {
        creationDate_ = creationDate;
        is_dirty_ = true;
    }

    const short&
    version() const {
        return version_;
    }

    const std::string&
    name() const {
        return name_;
    }

    template <typename T>
    das::Array<T> get_column(const std::string &col_name, size_t start, size_t length) {
        if (sa_.get() == NULL)
            sa_.reset(das::tpl::StorageAccess::create(bundle_.alias(), this));
        return sa_->get_column<T>(col_name, start, length);
    }

    template <typename T>
    void append_column(const std::string &col_name, das::Array<T> &a) {
        if (sa_.get() == NULL)
            sa_.reset(das::tpl::StorageAccess::create(bundle_.alias(), this));
        sa_->append_column<T>(col_name, a);
    }

    template <typename T, int Rank>
    das::Array<T, Rank> get_image() {
        if (sa_.get() == NULL)
            sa_.reset(das::tpl::StorageAccess::create(bundle_.alias(), this));
        sa_->get_image<T, Rank>();
    }
    
    template <typename T, int Rank>
    das::Array<T, Rank> get_image(
            const das::TinyVector<int,Rank> &offset,
            const das::TinyVector<int,Rank> &count,
            const das::TinyVector<int,Rank> &stride) {
        if (sa_.get() == NULL)
            sa_.reset(das::tpl::StorageAccess::create(bundle_.alias(), this));
        sa_->get_image<T, Rank>(offset,count,stride);
    }

    template <typename T, int Rank>
    void set_image(das::Array<T, Rank> &i) {
        if (sa_.get() == NULL)
            sa_.reset(das::tpl::StorageAccess::create(bundle_.alias(), this));
        sa_->set_image<T, Rank>(i);
    }

    template <typename T, int Rank>
    void append_tiles(das::Array<T, Rank> &i) {
        if (sa_.get() == NULL)
            sa_.reset(das::tpl::StorageAccess::create(bundle_.alias(), this));
        sa_->append_tiles<T, Rank>(i);
    }

    //polimorphic interface

    virtual bool is_table() {
        return false;
    }

    virtual bool is_image() {
        return false;
    }

protected:

    DasObject() {
        type_name_ = "DasObject";
        version_ = 0;
        das_id_ = 0;
        is_dirty_ = false;
    }

    DasObject(const std::string &name, const std::string &db_alias)
    : name_(name), bundle_(db_alias) {
        type_name_ = "DasObject";
        version_ = 0;
        das_id_ = 0;
        is_dirty_ = false;
    }
#pragma db transient
    std::string type_name_;

#pragma db transient
    das::tpl::WeakDbBundle bundle_;

#pragma db transient
    bool is_dirty_; // does it need an update?

    // polimorphic internal persistent interface

    virtual void save_data(const std::string &path, das::tpl::TransactionBundle &tb) {
    } // save external data, check if the path is empty.

    virtual void save_data(das::tpl::TransactionBundle &tb) {
    } // update external data.   

    virtual void update(das::tpl::TransactionBundle &tb) {
    } // update self and associated if necessary

    // we need a database pointer because this object is not bound to any db yet

    virtual void persist_associated_pre(das::tpl::TransactionBundle &tb) {
    } // call persist on shared many associated objects

    virtual void persist_associated_post(das::tpl::TransactionBundle &tb) {
    } // call persist on exclusive and oneassociated objects

    virtual void set_dirty_columns() {
    } //set all entries on odb::vector columns as dirty in order to force an update

    // StorageEngine utilities

    virtual void get_columns_from_file(std::map<std::string, ColumnFromFile*> &map) {
        throw das::no_external_data();
    }

    virtual ColumnFromFile* column_from_file(const std::string &col_name) {
        throw das::no_external_data();
    }

    virtual void column_from_file(const std::string &col_name, const ColumnFromFile &cf) {
        throw das::no_external_data();
    }

    virtual ImageFromFile* image_from_file() {
        throw das::no_external_data();
    }

    virtual void image_from_file(const ImageFromFile &cf) {
        throw das::no_external_data();
    }

    das::tpl::StorageAccess*
    storage_access() {
        if (sa_.get() == NULL)
            sa_.reset(das::tpl::StorageAccess::create(bundle_.alias(), this));
        return sa_.get();
    }

    virtual void get_keywords(std::map<std::string, keyword_type> &map) {
        map.insert(std::pair<std::string, keyword_type>("das_id", das_id_));
        map.insert(std::pair<std::string, keyword_type>("name", name_));
        map.insert(std::pair<std::string, keyword_type>("version", version_));
        map.insert(std::pair<std::string, keyword_type>("dbUserId", dbUserId_));
        map.insert(std::pair<std::string, keyword_type>("creationDate", creationDate_));
    }

    static inline
    std::string
    escape_string(const std::string &str) {
        std::string s;
        size_t len = str.length();
        for (size_t i = 0; i < len; ++i) {
            if (str[i] == '\'')
                s.push_back('\\');
            s.push_back(str[i]);
        }
        return s;
    }

    static inline
    std::string
    unescape_string(const std::string &str) {
        std::string s;
        size_t len = str.length();
        for (size_t i = 0; i < len; ++i) {
            if (str[i] == '\'' && i + 1 < len && str[i + 1] == '\'')
                continue;
            s.push_back(str[i]);
        }
        return s;
    }
private:

    std::string get_name() const {
        return escape_string(name_);
    }

    void set_name(const std::string &name) {
        name_ = unescape_string(name);
    }

    friend class odb::access;
    friend class das::tpl::Database;
    friend class das::tpl::Transaction;
    friend class das::tpl::TransactionBundle;
    friend class das::tpl::DbBundle;
    friend class das::tpl::StorageAccess;
    friend class das::tpl::StorageTransaction;
    template <typename T> friend class das::tpl::result_iterator;
    friend class QLVisitor;
    //  template <typename T> friend class DasVector;
    //  template <typename T> friend void ::swap(DasVector<T> &x, DasVector<T> &y);
#pragma db id auto
    long long das_id_;

#pragma db type("VARCHAR(256)")
    std::string dbUserId_;
    long long creationDate_;

#pragma db transient
    std::auto_ptr<das::tpl::StorageAccess> sa_;

protected:
    short version_;
#pragma db type("VARCHAR(256)") set(set_name) get(get_name) index
    std::string name_;

};

template<class T>
struct das_traits {
};


//#include "internal/storage_engine.ipp"
#endif
