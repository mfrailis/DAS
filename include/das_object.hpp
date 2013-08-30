#ifndef DAS_DASOBJECT_HPP
#define DAS_DASOBJECT_HPP

#include <odb/core.hxx>
#include <odb/database.hxx>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>

#include <string>
#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

#include "ddl/info.hpp"
#include "ddl/column.hpp"
#include "ddl/image.hpp"
#include "internal/db_bundle.hpp"
#include "internal/array.hpp"
#include "storage_engine.hpp"

using std::tr1::shared_ptr;

class QLVisitor;
namespace das {
    class Transaction;
    namespace tpl {
        class Database;
        template<typename T>
        class result_iterator;
    }
}

class Key_set : public boost::static_visitor<void> {
public:

    void operator() (std::string& keyword, const std::string& value) const {
        keyword = value;
    }

    void operator() (std::string& keyword, const char* value) const {
        keyword = value;
    }

    template<typename Key_type, typename Arg_type>
    void operator() (Key_type& keyword, const Arg_type& value) const {
        keyword = value;
    }

    template<typename Arg_type>
    void operator() (std::string& keyword, const Arg_type& value) const {
        std::cout << "cannot assign numbers to string" << std::endl;
    }

    template<typename Key_type>
    void operator() (Key_type& keyword, const std::string& value) const {
        std::cout << "cannot assign string to numeric type" << std::endl;
    }
};

template<typename T>
class Key_get : public boost::static_visitor<T> {
public:

    template<typename Key_type>
    T operator() (Key_type& key) const {
        return key;
    }

    T operator() (std::string& key) const {
        T lhs = 0;
        std::cout << "cannot assign string to numeric type" << std::endl;
    }
};

template<>
class Key_get<std::string> : public boost::static_visitor<std::string> {
public:

    template<typename Key_type>
    std::string operator() (Key_type& key) const {
        std::cout << "cannot assign numbers to string" << std::endl;
        return std::string("bad value");
    }

    std::string operator() (std::string& key) const {
        return key;
    }
};

#pragma db object abstract

class DasObject {
public:
    typedef boost::variant<
    signed char,
    char,
    short,
    int,
    long long,
    float,
    double,
    bool,
    std::string
    > keyword_type;

    typedef boost::variant<
    signed char&,
    char&,
    short&,
    int&,
    long long&,
    float&,
    double&,
    bool&,
    std::string&
    > keyword_type_ref;

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
    
    long long
    das_id() const{
        return das_id_;
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

    const std::string&
    type_name() const {
        return type_name_;
    }

    template <typename T>
    das::Array<T> get_column(const std::string &col_name, size_t start, size_t length) {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        return sa_->get_column<T>(col_name, start, length);
    }

    template <typename T>
    void append_column(const std::string &col_name, das::Array<T> &a) {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        sa_->append_column<T>(col_name, a);
    }

    template <typename T, int Rank>
    das::Array<T, Rank> get_image() {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        return sa_->get_image<T, Rank>();
    }

    template <typename T, int Rank>
    das::Array<T, Rank> get_image(
            const das::TinyVector<int, Rank> &offset,
            const das::TinyVector<int, Rank> &count,
            const das::TinyVector<int, Rank> &stride) {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        return sa_->get_image<T, Rank>(offset, count, stride);
    }

    template <typename T, int Rank>
    void set_image(das::Array<T, Rank> &i) {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        sa_->set_image<T, Rank>(i);
    }

    template <typename T, int Rank>
    void append_tiles(das::Array<T, Rank> &i) {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        sa_->append_tiles<T, Rank>(i);
    }

    //polimorphic interface
    static
    shared_ptr<DasObject>
    create(const std::string &type_name, const std::string &name, const std::string &db_alias){
        return DdlInfo::get_instance()->get_type_info(type_name).ctor->create(name,db_alias);
    }

    virtual bool is_table() const {
        return false;
    }

    virtual bool is_image() const {
        return false;
    }

    template<typename T>
    void
    set_key(const std::string &keyword_name, const T& value) {
        if (keyword_name == "name" || keyword_name == "version" || keyword_name == "dbUserId")
            throw das::read_only_keyword();

        keyword_type val = value;
        boost::apply_visitor(Key_set(), keywords_.at(keyword_name), val);
    }

    void
    set_key(const std::string &keyword_name, const char* value) {
        if (keyword_name == "name" || keyword_name == "version" || keyword_name == "dbUserId")
            throw das::read_only_keyword();

        keyword_type val = std::string(value);
        boost::apply_visitor(Key_set(), keywords_.at(keyword_name), val);
    }

    template<typename T>
    T
    get_key(const std::string &keyword_name) {
        return boost::apply_visitor(Key_get<T>(), keywords_.at(keyword_name));
    }

    shared_ptr<DasObject>
    get_associated_object(const std::string &assoc_name) {
        return DdlInfo::get_instance()->get_association_info(type_name_, assoc_name).access->get_one(this);
    }

    std::vector< shared_ptr<DasObject> >
    get_associated_objects(const std::string &assoc_name) {
        return DdlInfo::get_instance()->get_association_info(type_name_, assoc_name).access->get_many(this);
    }

    void
    set_associated_object(const std::string &assoc_name, shared_ptr<DasObject> &obj) {
        DdlInfo::get_instance()->get_association_info(type_name_, assoc_name).access->set_one(this, obj);
    }

    void
    set_associated_objects(const std::string &assoc_name, std::vector< shared_ptr<DasObject> >&vec) {
        DdlInfo::get_instance()->get_association_info(type_name_, assoc_name).access->set_many(this, vec);
    }

protected:

    DasObject()
    : version_(0),
    das_id_(0),
    is_dirty_(false),
    type_name_("DasObject") {
        init();
    }

    DasObject(const std::string &name, const std::string &db_alias)
    : name_(name),
    bundle_(db_alias),
    type_name_("DasObject"),
    version_(0),
    das_id_(0),
    is_dirty_(false) {
        init();
    }
#pragma db transient
    std::string type_name_;

#pragma db transient    
    boost::unordered_map<std::string, keyword_type_ref> keywords_;

#pragma db transient
    das::WeakDbBundle bundle_;

#pragma db transient
    bool is_dirty_; // does it need an update?

    // polimorphic internal persistent interface

    virtual void save_data(const std::string &path, das::TransactionBundle &tb) {
    } // save external data, check if the path is empty.

    virtual void save_data(das::TransactionBundle &tb) {
    } // update external data.   

    virtual void update(das::TransactionBundle &tb) {
    } // update self and associated if necessary

    // we need a database pointer because this object is not bound to any db yet

    virtual void persist_associated_pre(das::TransactionBundle &tb) {
    } // call persist on shared many associated objects

    virtual void persist_associated_post(das::TransactionBundle &tb) {
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

    das::StorageAccess*
    storage_access() {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
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

    void init() {
        keywords_.insert(std::pair<std::string, keyword_type_ref>("das_id", das_id_));
        keywords_.insert(std::pair<std::string, keyword_type_ref>("name", name_));
        keywords_.insert(std::pair<std::string, keyword_type_ref>("version", version_));
        keywords_.insert(std::pair<std::string, keyword_type_ref>("dbUserId", dbUserId_));
        keywords_.insert(std::pair<std::string, keyword_type_ref>("creationDate", creationDate_));
    }

    std::string get_name() const {
        return escape_string(name_);
    }

    void set_name(const std::string &name) {
        name_ = unescape_string(name);
    }

    friend class odb::access;
    friend class das::tpl::Database;
    friend class das::Transaction;
    friend class das::TransactionBundle;
    friend class das::DbBundle;
    friend class das::StorageAccess;
    friend class das::StorageTransaction;
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
    std::auto_ptr<das::StorageAccess> sa_;

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
