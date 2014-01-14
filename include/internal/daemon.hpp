#ifndef DAEMON_HPP
#define	DAEMON_HPP
#include <vector>
#include <string>
#include "../tpl/database.hpp"
#include "../storage_engine.hpp"
#include "../ddl/types.hpp"

namespace das {
    namespace data_gc {

        template<typename DasType, typename DataType>
        class Collector {
        public:

            void operator() (const shared_ptr<tpl::Database> &ptr,
                    const std::string &fk,
                    const std::string &tb) {
                typedef odb::result<DataType> result;

                shared_ptr<DasType> dummy = DasType::create("dummy",ptr->bundle_.alias());
                
                shared_ptr<StorageAccess> sa(StorageAccess::create(
                        ptr->bundle_.alias(), dummy.get()));

                Transaction t = ptr->begin(serializable);
                shared_ptr<odb::database> db = ptr->bundle_.db();

                std::string q_str = "id NOT IN (SELECT " + fk + " FROM " + tb + ")";
                result r = db->query<DataType>(q_str);
                for(typename result::iterator it = r.begin(); it != r.end(); ++it){
                    if(sa->release(&(*it)))
                        db->erase(*it);
                }
                t.commit();
            }
        };

        template<typename DasType>
        class Collector<DasType,void> {
        public:

            void operator() (const shared_ptr<tpl::Database> &ptr,
                    const std::string &fk,
                    const std::string &tb) {
            }
        };

        template<typename DasType>
        void collect(const shared_ptr<tpl::Database> &ptr) {
            typedef das_traits<DasType> Tr;
            typedef typename Tr::data_type DataType;

            Collector<DasType,DataType> c;
            c(ptr, Tr::foreign_key, Tr::data_config_table);
        }

    } // namespace garbage_data_collector
} // namespace das
#endif	/* DAEMON_HPP */

