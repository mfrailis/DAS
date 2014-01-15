#ifndef DATABASE_CONFIG_HPP
#define	DATABASE_CONFIG_HPP
#include <string>
#include <boost/unordered_map.hpp>
#include <memory>
#include <odb/tr1/memory.hxx>
#include <boost/property_tree/ptree.hpp>
using std::tr1::shared_ptr;
namespace das {

    class DatabaseInfo {
    public:
        DatabaseInfo() : port(0), buffered_(true) {
        }
        
        bool valid() const {
            return host != "" && db_type != "" && db_name != "";
        }
        
        bool accessible() const {
            return valid() && user != "";
        }

        std::string host;
        unsigned int port;
        std::string db_type;
        std::string db_name;
        std::string user;
        std::string password;
        std::string mysql_socket;
        boost::property_tree::ptree storage_engine;
        
        const bool&
        buffered_data() const{
            return buffered_;
        }
        
        void
        buffered_data(const bool& value) const{
            buffered_ = value;
        }
        
    private:
        mutable bool buffered_;
    };

    class DatabaseConfig {
    public:
        typedef boost::unordered_map<std::string, DatabaseInfo>::const_iterator const_iterator;
        
        const_iterator
        cbegin() const {return db_map_.cbegin();}
         
        const_iterator
        cend() const {return db_map_.cend();}  
        
        static const DatabaseInfo&  database(const std::string &alias);
    private:
        DatabaseConfig() : ready(false){}
        bool ready;
        //void parse_config(const std::string &config);
        void prepare_config();
        void parse_access(const std::string &access); 
        void get_ready();
        
        boost::unordered_map<std::string, DatabaseInfo> db_map_;
        
        static DatabaseConfig database_config;
    };
}
#endif	/* DATABASE_CONFIG_HPP */

