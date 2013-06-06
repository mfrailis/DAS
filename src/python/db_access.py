import json as _j
import jsonschema as _js
import subprocess as _s

class JsonAccessParser:
    def __init__(self,f_conf_sc,f_acc_sc):
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

    def execute(self, schema_file):
        if  self._db_type == 'mysql':
            cmd = "mysql"
            cmd = cmd+" --host="+self._host
            cmd = cmd+" --port="+str(self._port)
            cmd = cmd+" --user="+self._user_name
            cmd = cmd+" --password="+self._password
            cmd = cmd+" "+self._db_name
            cmd = cmd+" < "+schema_file
#FIXME: use pipes and not shell=True
            exit_code = _s.call(cmd, shell=True)
            return exit_code

        elif self._db_type == 'oracle':
            print "ERROR: DBMS vendor not supported (yet)"
            return 1
        elif self._db_type == 'pgsql':
            print "ERROR: DBMS vendor not supported (yet)"
            return 1
        elif self._db_type == 'sqlite':
            print "ERROR: DBMS vendor not supported (yet)"
            return 1
        elif self._db_type == 'mssql':
            print "ERROR: DBMS vendor not supported (yet)"
            return 1
            
if __name__ == '__main__':
    import sys
    
    conf_schema = sys.argv[1]
    acc_schema  = sys.argv[2]
    config      = sys.argv[3]
    access      = sys.argv[4]
    alias       = sys.argv[5]
    schema_file = sys.argv[6]
    
    c = JsonAccessParser(conf_schema,acc_schema)
    c.parse(config,access,alias)

    exit_code = c.execute(schema_file)
    exit(exit_code)
