#include "internal/database_config.hpp"
#include "internal/log.hpp"
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
            try{
                DatabaseInfo &info = db_map_.at(v.second.get<std::string>("alias"));
                info.user = v.second.get<std::string>("user");
                info.password = v.second.get<std::string>("password", "");
            }catch(std::out_of_range &e){
                DAS_LOG_DBG("DAS warning: database '"<<  v.second.get<std::string>("alias") <<"' is not present in confi.json file" );       
            }
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
            DAS_LOG_DBG("unable to read HOME environment variable");      
            throw bad_path();
        }
        std::string access(home);
        access.append("/.das/access.json");
        
        database_config.prepare_config();
        database_config.parse_access(access);
        database_config.ready = true;
    }
    
}