import json as _j
import jsonschema as _js
#import subprocess as _s
import MySQLdb as _my
import ddl as _d
import os as _os

MYSQL_DDL_SCHEMA='''
CREATE TABLE IF NOT EXISTS `ddl_schema_types` (
   `schema` TEXT
)
ENGINE=InnoDB;
'''


class JsonAccessParser:
    def __init__(self,f_conf_sc,f_acc_sc,sql_dir,prefix):
        f = open(f_conf_sc,'r')
        self._conf_sc = _j.load(f)
        f.close()

        f = open(f_acc_sc,'r')
        self._acc_sc = _j.load(f)
        f.close()

        self._access = []
        self._config = []
        self._host = ""
        self._port = -1
        self._db_name = ""
        self._db_type = ""
        self._found = False
        self._user_name = ""
        self._password = ""
        self._pref = prefix
        self._dir = sql_dir

    def parse(self,f_conf,f_acc,alias):
        f = open(f_conf,'r')
        self._config = _j.load(f)
        f.close()
        _js.validate(self._config, self._conf_sc, format_checker=_js.FormatChecker())

        aliases = []        
        for db in self._config:
            if db['alias'] in aliases:
                print 'ERROR: found repeated  "'+db['alias']+'" alias in configuration file'
                exit(1)
            aliases.append(db['alias'])
            if db['alias'] == alias:
                self._host    = db['host']
                self._port    = db['port']
                self._db_name = db['db_name']
                self._db_type = db['db_type']

        if alias not in aliases:
            print 'ERROR: "'+alias+'" alias not found in configuration file'
            exit(1)           

        f = open(f_acc,'r')
        self._access = _j.load(f)
        f.close()
        _js.validate(self._access, self._acc_sc, format_checker=_js.FormatChecker())

        u_aliases = []
        for db_a in self._access:
            if db_a['alias'] in u_aliases:
                print 'ERROR: found repeated  "'+db_a['alias']+'" alias in access file'
                exit(1)
            if db_a['alias'] == alias:
                self._found = True
                self._user_name = db_a['user']
                self._password = db_a['password']
            u_aliases.append(db_a['alias'])
        
        if not self._found:
            print 'ERROR: no credentials for "'+alias+'" found'
            exit(1)

    def execute(self, xsd_schema, xml_new):
        is_new = True
        if  self._db_type == 'mysql':
            db = _my.connect(host=self._host,
                             port=self._port,
                             user=self._user_name,
                             passwd=self._password,
                             db=self._db_name)
            db.autocommit(False)
            c = db.cursor()
            c.execute(MYSQL_DDL_SCHEMA)
            c.close()
            #TODO load from database
#            schema_cur = "" #load from database
            parser = _d.DdlParser(xsd_schema)
            
            cur_types = _d.DdlTypeList()#parser.parse_ddl_from_string(schema_cur)
            new_types = parser.parse_ddl(xml_new)
            tree_string = db.escape_string(parser.serialize_tree(xml_new))
            #DBG
#            print tree_string

            code = self._generate_mysql(cur_types,new_types)
            for i in code:
                cc = db.cursor()
                cc.execute(i)
                cc.close()

            c = db.cursor()           
            if is_new:
                c.execute("INSERT INTO `ddl_schema_types`(`schema`) VALUES (%s)",[tree_string])
            else:
                c.execute("UPDATE `ddl_schema_types` SET `schema` = %s)",[tree_string])

            c.close()
            db.commit()
            db.close()

        elif self._db_type == 'oracle':
            print "ERROR: DBMS vendor not supported (yet)"
            exit(1)
        elif self._db_type == 'pgsql':
            print "ERROR: DBMS vendor not supported (yet)"
            exit(1)
        elif self._db_type == 'sqlite':
            print "ERROR: DBMS vendor not supported (yet)"
            exit(1)
        elif self._db_type == 'mssql':
            print "ERROR: DBMS vendor not supported (yet)"
            exit(1)

#TODO modify this function in order to perform types upgrades
    def _generate_mysql(self,cur,new):
        code = []
        for t in new.type_map.values():
            ct = cur.type_map.get(t.name,None)
            if ct is None:
                if t.name != "essentialMetadata":
                    f_name =_os.path.join(self._dir,self._pref + t.name + ".sql")
                    f = open(f_name, 'r')
                    text = f.read()
                    f.close()
                    code.append(text)
            else:
                # perform type upgrade operations
                pass
            
        return code
                   
    

            
if __name__ == '__main__':
    import sys
    
    conf_schema = sys.argv[1]
    acc_schema  = sys.argv[2]
    config      = sys.argv[3]
    access      = sys.argv[4]
    alias       = sys.argv[5]
    sql_dir     = sys.argv[6]
    ddl_schema  = sys.argv[7]
    ddl_types   = sys.argv[8]
    type_prefix = sys.argv[9]
    
    c = JsonAccessParser(conf_schema,acc_schema,sql_dir,type_prefix)
    c.parse(config,access,alias)

    c.execute(ddl_schema,ddl_types)
    
