<a name='das_config'></a>
Configuring the system
----------------------

In order to configure and build the DAS system you need to edit some configuration files:

  * config.json : is located in the configure directory under the DAS source directory and contains
	  the basic information about the database, the way the data is persisted, and the reference of the
	  ddl files.
	  
  * access.json : is located in the .das directory in the $HOME of each user who runs the das
	  library. This file contains the credentials for the databases which the user has access.
	  
  * ddl.xml : the files located in the ddl directory under the DAS source directory, in XML format which
	  define the data model.

## config.json ##

The configuration file contains an array of objects. Each one contains a configuration for a 
different database. The file is parsed and validated during the configuration step (cmake command)
through the json-schema file resources/config_schema.json. Note that each change to this file must be
followed by a reconfiguration of the DAS system in order to make them active.

~~~{.js} 
	[
		{
			"host"          : "localhost",
			"port"          : 3306,
			"db_type"  	    : "mysql",
			"alias"         : "test_level1",
			"db_name"  	    : "test_level1",
			"ddl"	     	: "ddl_level1_types.xml",
			"storage_engine": {
				"name"          : "Raw",
				"root_dir"      : "/mnt/DBs/data/test_level1/",
				"default_path"  : "%F/%t/%n_%v/",
				"custom_path"   : "%s/%B/%n/%v",
				"temp_path"     : "/mnt/DBs/data/temp/%$LOGNAME$",
				"unref_data_expiration_time" : 864000
			}
		},
		{
			"host"          : "localhost",
			"port"          : 3306,
			"db_type"  	    : "mysql",
			"alias"         : "test_level2",
			"db_name"  	    : "test_level2",
			"ddl"	     	: "ddl_test_types.xml",
			"storage_engine": {
				"name"          : "Raw",
				"root_dir"      : "/mnt/DBs/data/test_level2/",
				"default_path"  : "%F/%t/%n/%v/",
				"custom_path"   : "%s/%B/%n/%v",
				"temp_path"      : "/mnt/DBs/data/temp/%$LOGNAME$",
				"unref_data_expiration_time" : 60
			}
		}
    ]
~~~

Each of the properties shown in this example are mandatory:

  * host : the hostname or ip address where the database is located
  * port : the listening port of the database
  * db_type : the database vendor, currently only mysql is supported
  * alias : human readable name to use in the code to refer to this database
  * db_name : the name of the database to access for storing meta-data
  * ddl : the relative path of the file that contains the DDL for this database
  * storage_engine : this object contains the configuration of the data storage engine.
	Except for name, all other properties change for each engine type. Please refer to the 
	storage engine specific documentation in the following sections. Note that currently we provide
	only the Raw storage engine.

### Raw Storage engine ###

This storage engine creates one file per column if the data type is binary table, and one file per image
otherwise. when the user updates the data, for example a column, a new file is created containing the
updated data while the old one is kept allowing other sessions (users) which refer to out-date data
to keep reading that data. Once in awhile, depending on the crontab configuration, the garbage
collector runs and remove the obsolete files.

this properties must be provided in the config.json file:
  * name : name of the storage engine, the value "Raw" identifies this one.
  * root_dir : absolute path to the data. each data file will be stored in a sub-directory of this path
  *	default_path : expression for the default relative path for the data. The resolved expression 
	  appended to the root_path forms the complete path for the data file. The possible token will
	  be analysed in the next session. Note that the tokens '\%n' and '\%v' are mandatory, as say the
	  have to appear somewhere in the expression.
  * custom_path : expression for the custom relative path for the data. Like the default_path
	  expression, the resolved expression appended to the root_path forms the complete path for
	  the data file. The token '\%s' is resolved run-time with the string provided by the user as 
	  argument to the persist methods. Please note that the tokens '\%n' and '\%v' are still mandatory.
	  If the token '\%s' doesn't appear in the expression the eventual string argument provided in the 
	  persist methods will be ignored.
  * temp_path : this expression represent the absolute path for the temporary data, i.e. the data not
	  yet persisted in a transaction. This data file will be eventually moved in the final path 
	  during the transaction commit of the owning object. Note that if the temp_path and root_dir
	  are mounted in the same file-system volume the move operation does not involve any copy.
  * unref_data_expiration_time : this parameter refers to the garbage collector. Each time the 
	  garbage collector runs, the obsolete data files are deleted if the modification time plus
	  unref_data_expiration_time is less then the current time. this value is expressed in seconds.
	  Note that the current time is retrieved from the machine which runs the garbage collector, hence 
	  this machine must be kept synchronised (e.g. using a common ntp server) with the machine serving
	  the data file-system.
	  
#### path tokens ####

the default_path, custom_path and temp_path expressions are regular unix paths plus some special
tokens escaped by the character '\%'.

##### Time related tokens #####
  
  | token  |                             replaced by                                | example 
  |:------:|:-----------------------------------------------------------------------|:--------
  | \%a    | Abbreviated weekday name *                                             | Thu
  | \%A    | Full weekday name *                                                    | Thursday
  | \%b    | Abbreviated month name *                                               | Nov 
  | \%B    | Full month name *                                                      | November 
  | \%c    | Date and time representation *	                                        | Thu_Aug_23_14-55-02_2001
  | \%C    | Year divided by 100 and truncated to integer (00-99)                   | 20
  | \%d    | Day of the month, zero-padded (01-31)                                  | 23
  | \%F    | Short YYYY-MM-DD date, equivalent to %Y-%m-%d                          | 2001-08-23
  | \%g    | Week-based year, last two digits (00-99)	                            | 01
  | \%G    | Week-based year                                                        | 2001
  | \%H    | Hour in 24h format (00-23)                                             | 14
  | \%I    | Hour in 12h format (01-12)                                             | 02
  | \%j    | Day of the year (001-366)                                              | 235
  | \%m    | Month as a decimal number (01-12)                                      | 08
  | \%M    | Minute (00-59)                                                         | 55
  | \%p    | AM or PM designation	                                                | PM
  | \%r    | 12-hour clock time *	                                                | 02-55-02_pm
  | \%R    | 24-hour HH:MM time, equivalent to %H-%M	                            | 14-55
  | \%S    | Second (00-61)	                                                        | 02
  | \%T    | ISO 8601 time format (HH:MM:SS), equivalent to %H-%M-%S	            | 14-55-02
  | \%u    | ISO 8601 weekday as number with Monday as 1 (1-7)	                    | 4
  | \%U    | Week number with the first Sunday as the first day of week one (00-53)	| 33
  | \%V    | ISO 8601 week number (00-53)                                           | 34
  | \%w    | Weekday as a decimal number with Sunday as 0 (0-6)                     | 4
  | \%W    | Week number with the first Monday as the first day of week one (00-53) | 34
  | \%X    | Time representation *                                                  | 14-55-02
  | \%y    | Year, last two digits (00-99)                                          | 01
  | \%Y    | Year                                                                   | 2001
  | \%z    | ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100). If timezone cannot be determined, no characters | +100
  | \%Z    | Timezone name or abbreviation. If timezone cannot be determined, no characters * |	CDT
  

* The specifiers marked with an asterisk (*) are locale-dependent.

#####  Object related tokens #####

  | token  |  replaced by            | example 
  |:------:|:------------------------|:--------
  | \%t    | type name               | measure
  | \%n    | object name             | meas_001
  | \%v    | version of the object   | 1
  
##### Other tokens #####
 The '\%s' token can be used in the temp_path expression and represents the custom string provided
 by the user as the persist methods argument.
 
 Environment variables may by used as tokens. You just have to surround the variable name with '\$'
 
     some/path/%$ENV_VARIABLE$some/other/path
 
 
## access.json ##

This file must exists in the $HOME/.das/ directory of the user and contains the credentials to access
the databases. This file is read run-time therefore changes on them may be performed without any further
configuration.

~~~{.js} 
    [
		{
			"alias"     : "test_level1",
			"user"      : "foo",
			"password"  : "bar"
		},
		{
			"alias"     : "test_level2",
			"user"      : "foo",
			"password"  : "secret"
		}
	]
~~~

  * alias : the database alias that match the one specified in the config.json file.
  * user : the username of the database.
  * password : the password for that database.
