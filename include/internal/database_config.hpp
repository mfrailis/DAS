#ifndef DATABASE_CONFIG_HPP
#define	DATABASE_CONFIG_HPP
#include <string>
#include <boost/unordered_map.hpp>
#include <memory>
#include <odb/tr1/memory.hxx>
using std::tr1::shared_ptr;
namespace das {

    struct DatabaseInfo {

        DatabaseInfo() : port(0) {
        }

        bool valid() const {
            return host != "" && db_type != "" && db_name != "" &&
                    data_root_dir != "" && time_interval != "";
        }

        bool accessible() const {
            return valid() && user != "";
        }

        std::string host;
        unsigned int port;
        std::string db_type;
        std::string db_name;
        std::string data_root_dir;
        std::string time_interval;
        std::string user;
        std::string password;
    };

    class DatabaseConfig {
    public:
        typedef boost::unordered_map<std::string, DatabaseInfo>::const_iterator const_iterator;
        
        const_iterator
        cbegin() const {return db_map_.cbegin();}
         
        const_iterator
        cend() const {return db_map_.cend();}  
        
        static const DatabaseInfo&  database(const std::string &alias);
        static const std::string&  temp_dir(); 
    private:
        DatabaseConfig() : ready(false){}
        bool ready;
        //void parse_config(const std::string &config);
        void prepare_config();
        void parse_access(const std::string &access); 
        void get_ready();
        
        boost::unordered_map<std::string, DatabaseInfo> db_map_;
        
        static DatabaseConfig database_config;
        static std::string temp_;
    };
}
#endif	/* DATABASE_CONFIG_HPP */

