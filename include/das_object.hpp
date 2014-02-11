#ifndef DAS_DASOBJECT_HPP
#define DAS_DASOBJECT_HPP

#include <odb/core.hxx>
#include <odb/database.hxx>
#include <odb/tr1/memory.hxx>
#include <odb/tr1/lazy-ptr.hxx>

#include <string>
#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "optional.hpp"
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

    void operator() (boost::optional<std::string>& keyword, const std::string& value) const {
        keyword = value;
    }

    void operator() (boost::posix_time::ptime& keyword, const boost::posix_time::ptime& value) const {
        keyword = value;
    }

    template<typename Key_type, typename Arg_type>
    void operator() (Key_type& keyword, const Arg_type& value) const {
        keyword = value;
    }

    template<typename Arg_type>
    void operator() (boost::optional<std::string>& keyword, const Arg_type& value) const {
        throw das::bad_keyword_type();
    }

    template<typename Arg_type>
    void operator() (boost::posix_time::ptime& keyword, const Arg_type& value) const {
        throw das::bad_keyword_type();
    }

    void operator() (boost::posix_time::ptime& keyword, const std::string& value) const {
        throw das::bad_keyword_type();
    }

    template<typename Key_type>
    void operator() (Key_type& keyword, const std::string& value) const {
        throw das::bad_keyword_type();
    }

    template<typename Key_type>
    void operator() (Key_type& keyword, const boost::posix_time::ptime& value) const {
        throw das::bad_keyword_type();
    }

    void operator() (boost::optional<std::string>& keyword, const boost::posix_time::ptime& value) const {
        throw das::bad_keyword_type();
    }


};

template<typename T>
class Key_get : public boost::static_visitor<das::optional<T> > {
public:

    template<typename Key_type>
    das::optional<T> operator() (boost::optional<Key_type>& key) const {
        boost::optional<T> opt(key);
        return das::optional<T>(opt);
    }

    template<typename Key_type>
    das::optional<T> operator() (Key_type& key) const {
        return key;
    }

    das::optional<T> operator() (boost::posix_time::ptime& key) const {
        throw das::bad_keyword_type();
    }

    das::optional<T> operator() (boost::optional<std::string>& key) const {
        throw das::bad_keyword_type();
    }

    das::optional<T> operator() (std::string& key) const {
        throw das::bad_keyword_type();
    }

};

template<>
class Key_get<std::string> : public boost::static_visitor<das::optional<std::string> > {
public:

    template<typename Key_type>
    das::optional<std::string> operator() (Key_type& key) const {
        throw das::bad_keyword_type();
    }

    das::optional<std::string> operator() (std::string& key) const {
        return das::optional<std::string>(key);
    }

    das::optional<std::string> operator() (boost::optional<std::string>& key) const {
        return das::optional<std::string>(key);
    }
};

template<>
class Key_get<boost::posix_time::ptime> : public boost::static_visitor<boost::posix_time::ptime> {
public:

    template<typename Key_type>
    boost::posix_time::ptime operator() (Key_type& key) const {
        throw das::bad_keyword_type();
    }

    boost::posix_time::ptime operator() (boost::posix_time::ptime& key) const {
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
    std::string,
    boost::posix_time::ptime
    > keyword_type;

    typedef boost::variant<
    long long&, // das_id
    std::string&, // name, dbUserId
    short&, // version
    boost::posix_time::ptime&, // creationDate
    boost::optional<signed char>&,
    boost::optional<char>&,
    boost::optional<short>&,
    boost::optional<int>&,
    boost::optional<long long>&,
    boost::optional<float>&,
    boost::optional<double>&,
    boost::optional<bool>&,
    boost::optional<std::string>&
    > keyword_type_ref;

    typedef boost::unordered_map<std::string, keyword_type_ref> keyword_map;

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
    das_id() const {
        return das_id_;
    }

    const std::string&
    dbUserId() const {
        return dbUserId_;
    }

    const boost::posix_time::ptime&
    creationDate() const {
        return creationDate_;
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
    das::Array<T> get_column(
            const std::string &col_name,
            size_t start = 0,
            ssize_t length = -1) {
        size_t len = length;
        if (length < 0)
            len = get_column_size(col_name);
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        return sa_->get_column<T>(col_name, start, len);
    }

    long long
    get_column_size(const std::string &col_name) {
        Column *c = column_ptr(col_name);
        if (c)
            return c->size();
        else
            return 0;
    }

    template <typename T>
    void append_column(const std::string &col_name, das::Array<T> &a) {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        sa_->append_column<T>(col_name, a);
    }

    template <typename T, int Rank>
    das::ColumnArray<T, Rank>
    get_column_array(
            const std::string &col_name,
            size_t start = 0,
            ssize_t length = -1) {
        size_t len = length;
        if (length < 0)
            len = get_column_array_size(col_name);

        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        return sa_->get_column_array<T, Rank>(col_name, start, len);
    }

    long long
    get_column_array_size(const std::string &col_name) {
        return get_column_size(col_name);
    }

    template <typename T, int Rank>
    void append_column_array(const std::string &col_name, das::ColumnArray<T, Rank> &a) {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        sa_->append_column_array<T, Rank>(col_name, a);
    }

    template <typename T, int Rank>
    das::Array<T, Rank> get_image(
            das::Range r0 = das::Range::all(),
            das::Range r1 = das::Range::all(),
            das::Range r2 = das::Range::all(),
            das::Range r3 = das::Range::all(),
            das::Range r4 = das::Range::all(),
            das::Range r5 = das::Range::all(),
            das::Range r6 = das::Range::all(),
            das::Range r7 = das::Range::all(),
            das::Range r8 = das::Range::all(),
            das::Range r9 = das::Range::all(),
            das::Range r10 = das::Range::all()
            ) {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        return sa_->get_image<T, Rank>(r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    }

    unsigned int
    get_image_extent(int extent) {
        Image *i = image_ptr();
        if (i)
            return image_ptr()->extent(extent);
        else
            return 0;
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
    create(const std::string &type_name, const std::string &name, const std::string &db_alias) {
        return DdlInfo::get_instance()->get_type_info(type_name)(name, db_alias);
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
        if (keyword_name == "name" ||
                keyword_name == "version" ||
                keyword_name == "dbUserId" ||
                keyword_name == "creationDate")
            throw das::read_only_keyword();

        keyword_type val = value;
        boost::apply_visitor(Key_set(), keywords_.at(keyword_name), val);
    }

    void
    set_key(const std::string &keyword_name, const char* value) {
        if (keyword_name == "name" ||
                keyword_name == "version" ||
                keyword_name == "dbUserId" ||
                keyword_name == "creationDate")
            throw das::read_only_keyword();
        
        keyword_type val = std::string(value);
        boost::apply_visitor(Key_set(), keywords_.at(keyword_name), val);
    }

    template<typename T>
    das::optional<T>
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
        dbUserId_ = das::DatabaseConfig::database(db_alias).user;
    }

    ~DasObject(){}
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

    virtual void populate_column_map(std::map<std::string, Column*> &map) {
        throw das::no_external_data();
    }

    virtual Column* column_ptr(const std::string &col_name) {
        throw das::no_external_data();
    }

    virtual void column_ptr(const std::string &col_name, const Column &cf) {
        throw das::no_external_data();
    }

    virtual Image* image_ptr() {
        throw das::no_external_data();
    }

    virtual void image_ptr(const Image &cf) {
        throw das::no_external_data();
    }

    das::StorageAccess*
    storage_access() {
        if (sa_.get() == NULL)
            sa_.reset(das::StorageAccess::create(bundle_.alias(), this));
        return sa_.get();
    }

    const
    keyword_map&
    get_keywords() {
        return keywords_;
    }

    static inline
    boost::optional<std::string>
    escape_string(const boost::optional<std::string> &opt) {
        boost::optional<std::string> result;
        if (opt) {
            std::string str = opt.get();
            std::string s;
            size_t len = str.length();
            for (size_t i = 0; i < len; ++i) {
                if (str[i] == '\'')
                    s.push_back('\\');
                s.push_back(str[i]);
            }
            result = s;
        }
        return result;
    }

    static inline
    boost::optional<std::string>
    unescape_string(const boost::optional<std::string> &opt) {
        boost::optional<std::string> result;
        if (opt) {
            std::string str = opt.get();
            std::string s;
            size_t len = str.length();
            for (size_t i = 0; i < len; ++i) {
                if (str[i] == '\\' && i + 1 < len && str[i + 1] == '\'')
                    continue;
                s.push_back(str[i]);
            }
            result = s;
        }
        return result;
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
        return escape_string(boost::make_optional(name_)).get();
    }

    void set_name(const std::string &name) {
        name_ = unescape_string(boost::make_optional(name)).get();
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

#pragma db id auto
    long long das_id_;

#pragma db type("VARCHAR(256)")
    std::string dbUserId_;

#pragma db type("TIMESTAMP") options("DEFAULT CURRENT_TIMESTAMP()") not_null
    boost::posix_time::ptime creationDate_;


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

#endif
