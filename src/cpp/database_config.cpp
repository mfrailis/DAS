#include "internal/database_config.hpp"
#include "exceptions.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <stdlib.h>

namespace das {
    DatabaseConfig DatabaseConfig::database_config;

    void
    DatabaseConfig::parse_access(const std::string& access) {
        using boost::property_tree::ptree;
        ptree pt;

        boost::property_tree::read_json(access, pt);

        BOOST_FOREACH(ptree::value_type &v, pt.get_child("")) {
            DatabaseInfo &info = db_map_.at(v.second.get<std::string>("alias"));
            info.user = v.second.get<std::string>("user");
            info.password = v.second.get<std::string>("password", "");
        }

    }

    const
    DatabaseInfo&
    DatabaseConfig::database(const std::string& alias) {
        database_config.get_ready();
        return database_config.db_map_.at(alias);
    }
    
    void
    DatabaseConfig::get_ready()
    {
        if(database_config.ready) return;

        char* home;
        home = getenv ("HOME");
        if (!home)
        {
#ifdef VDBG
            std::cout << "unable to read HOME environment variable" << std::endl;      
#endif
            throw bad_path();
        }
        std::string access(home);
        access.append("/.das/access.json");
        
        database_config.prepare_config();
        database_config.parse_access(access);
        database_config.ready = true;
    }
    
/*
    void
    DatabaseConfig::parse_config(const std::string& config) {
        using boost::property_tree::ptree;
        ptree pt;

        boost::property_tree::read_json(config, pt);

        BOOST_FOREACH(ptree::value_type &v, pt.get_child("")) {
            unsigned int port;
            std::istringstream(v.second.get<std::string>("port", "0")) >> port;

            DatabaseInfo &info = db_map_[v.second.get<std::string>("alias")];
            info.host = v.second.get<std::string>("host");
            info.port = port;
            info.db_name = v.second.get<std::string>("db_name");
            info.db_type = v.second.get<std::string>("db_type");
            info.data_root_dir = v.second.get<std::string>("data_root_dir");
            info.time_interval = v.second.get<std::string>("time_interval");
        }

    }
*/
}