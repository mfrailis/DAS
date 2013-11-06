DAS user manual
===============

Configuring the system
----------------------

In order to co figure and build the DAS system you need to edit some configuration files:

  * config.json : is located in the configure directory and contains the basic information 
	  about the database, the way the data is persisted, and the reference of the ddl files.
	  
  * access.json : is located in the .das directory in the $HOME of each user who runs the das
	  library. This file contains the credentials for the databases which the user has access.
	  
  * ddl.xml : the files located in the ddl directory, in XML format which define the data model.

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
	  be analyzed in the next session. Note that the tokens '\%n' and '\%v' are mandatory, as say the
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
	  this machine must be kept synchronized (e.g. using a common ntp server) with the machine serving
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

Managed object model
--------------------

The way which DAS manages persistent objects is inspired by some concepts in the JPA (Java Persistent
API) standard. Objects in a managed context are automatically synchronized with their persisted
counterpart.

Preliminary definitions:

  * ddl-object : object, that is, instance of a c++ class, that maps a specified DDL data type.
  
  * pu-instance: (Persistence Unit instance) couple of database and and shared directory fully managed
	 by the system. Each pu-instance is uniquely identified by a string set in the "db-alias" option
	 of the das configuration file.
	  
  * pm-object: (Persistence Manager object) instance of the class Database that handles connections,
	 transactions, I/O and other operations on a specific pu-instance.
	 Multiple pm-objects referring the same pu-instance can be active (in the same scope)
	 at the same time.
	 
  * session: block of code where ddl-objects can be synchronized with a pm-object.
	 A session, by default, starts with the creation of a transaction and ends on the commit of the 
	 latter.

  * persistent counterpart: data stored in a database and shared file system representing
	 a ddl-object data structure(s).

<a name='ddl_object_state'></a>   
A ddl-object can be in one of the following states:

  - new: a transient ddl-object without persistent counterpart.
	 A fresh created object is in the new state and it can pass to attached state trough
	 the Database::persist() method.
	  
  - attached: a ddl-object with a persistent counterpart managed by a database instance 
     (pm-object) in a session.
	  
  - detached: an object with a persistent counterpart stored in a pu-instance but not attached to any
	 session.  Attached objects automatically become detached when the session ends; as say the 
	 transaction is committed/rolled-back for default session, the method Database::end_session() is
	 called for extended sessions.
     A detached ddl-object can be later attached to the owner pu-instance session through the
	 Database::attach() method. This means that the new and the old database instance must refer the
	 same pu-instance.

### DDL objects ###

A ddl-object newly created is in the new state. You can freely modify and delete it without involve
any database operations.

~~~{.cpp} 
    shared_ptr<measure> m = measure::create("measure_name");
    m->run_id(12345);
    m.reset();
~~~

Once you want to make a ddl-object persistent, you first have to create a pm-object, then create a
transaction and finally you can call the method persist passing the ddl-object as argument.
When you have finished persisting the objects, just call the commit method on the transaction object.

~~~{.cpp} 
    shared_ptr<measure> m1 = measure::create("measure1");
    shared_ptr<measure> m2 = measure::create("measure2");
	
    shared_ptr<Database> db = Database::create("test_level1");
	
    Transaction t(db->begin());
    db->persist(m1);
    db->persist(m2);
    t.commit();
~~~
	
If your application spend most of the time elaborating data with no interaction with pm-objects until
the the very end of the program, you may find convenient load from a pm-object the needed 
ddl-objects, then delete the pm-object allowing the system to release resources, do your 
computation, and finally update the persistent objects attaching the modified ddl-objects to a new
pm-object which refers the same pu-instance as the first one.
You must be aware, however, that the persistent counterpart of the objects you want to attach may
not be the same ones you loaded before, because of the concurrent access to the system. If this is the
case, attaching those objects will end up losing the updates made by the other(s) user(s) on them.


~~~{.cpp}
    shared_ptr<Database> db = Database::create("test_level1");

	Transaction t(db->begin());
    shared_ptr<measure> m1 = db->load<measure>(34);
    shared_ptr<measure> m2 = db->load<measure>(35);
	t.commit();
    db.reset();	

    /*
	 * long computation goes here
	 */

    db =  Database::create("test_level1");

	Transaction t2(db->begin());
    db->attach(m1);
    db->attach(m2);
	t2.commit();
~~~
  
### Sessions ###

The contract of the session is to guarantee the uniqueness of a ddl-object; as say that for each
persistent counterpart, at most one ddl-object is managed by the session.
Furthermore, each modified ddl-object held by the session will trigger an update in the database
during the next transaction commit.
This contract allow us to associate ddl-objects and work with them without warring about dealing with
multiple copies of the same persistent counterpart.
A database transaction is always paired with exactly one session. If there is an extended session,
then this one is used; Otherwise, the system creates a new session which ends the transaction is either
committed or rolled-back.

<a name='extended_session'></a>
#### Extended Sessions ####

As a best practice, you want the database transaction last as little as possible. But you also want to
take advantage of the guarantees (uniqueness and synchronization) offered by the session.
You can fulfill this two goals using the extended sessions. An extended session life is managed by the
methods Database::begin_session() and Database::end_session() and can span multiple transactions.


~~~{.cpp}
    shared_ptr<Database> db = Database::create("test_level1");

	db->begin_session();
  
	Transaction t(db->begin());
    shared_ptr<measure> m1 = db->load<measure>("measure_456");
	t.commit();


	Transaction t2(db->begin());
    shared_ptr<measure> m2 = db->load<measure>("measure_789");
	t2.commit();

	db->end_session();
~~~

Because of the unicity guarantee, if we try to load a ddl-object more than once, even in different 
transactions spanned by the same extended session, the system will return us the same object pointer
without even touching the database.
Note that when you use extended sessions you can occur in the same lost update problem described
before: in a subsequent transaction (spanned by the same extended transaction) the system may override
a persistent counterpart updated concurrently by another user.

In order to provide you a valid result, before any query execution the system automatically flushes any
modification made on the ddl-objects attached on the current session. As you might expect, this can
also cause lost updates.

~~~{.cpp}
    shared_ptr<Database> db = Database::create("test_level1");

	db->begin_session();
  
	Transaction t(db->begin());
    shared_ptr<measure> m = db->load<measure>("measure_456");
	t.commit();

	m->run_id(555);

	Transaction t2(db->begin());
	// m related persistent counterpart is flushed in the database
    Result r = db->query<measure>("run_id > 500","name asc");
	t2.commit();

	db->end_session();
~~~


<a name='ql'></a>
Query Language
--------------
The Das system provides a powerful query language using the simple and familiar object-oriented point
notation.
A Das query is composed by two clauses: a query expression and an optional ordering clause.
The query expression in turn is a composition of one or more boolean expressions.

#### Keyword reference ####
We can refer to the keywords of the ddl-type query target by simply typing its name in an expression.
    
	<keyword_name_1> + <keyword_name_2> == constant-value
	
To refer a keyword of an associated ddl-type we prepend the name of the association to the keyword.

    <association_name>.<associated_keyword_name> == constant-value

Given this ddl configuration:
    
	+---------+      +---------+      +---------+     
    | type A  |  +-->| type B  |  +-->| type C  |
	+---------+  |   +---------+  |	  +---------+
	| assoc_b |--+	 | assoc_c |--+	  | key_1   |
	+---------+	     +---------+      +---------+

We can navigate through the association chain with the same point notation.

	assoc_b.assoc_c.key_1 == const-value
	
#### Boolean expressions ####
Boolean expressions may be composed with the C boolean operators: && , || , !  .
We can also use round brackets in order to explicit the operator associativity and priority.

    exp1 && exp2 || !(exp3 || exp4)

The Das query system provides two string operators: startsWith() and endsWith() as valid boolean
expressions. We can use this operators like a method invocation on a keyword of type string with a
const string argument (a string enclosed by single quotes).

    keyword_name.startsWith('cmp') && keyword_name.endsWith('_test')
	
Other valid boolean expressions are the comparison expressions.
	
#### Comparison expressions ####
Comparison expression allow us to compare arithmetic expressions, keyword references and constant
values. Again, we use the C syntax with the well known semantic: < , > , <= , >= , == , !=  .
Round brackets may be used here as well for more readability.

    keyword_name == 'test_1'
	
	keyword_1 >= keyword_2 + keyword_3
	
	keyword_1 < 5.6749

#### Arithmetic expressions ####
We can compose an arithmetic expression using keyword references, integer and floating point constants
and C arithmetic operators: + , - , * , / .

    keyword_1 * (keyword_2 + keyword_3)

<a name='order_clause'></a>
### Ordering Clause ###
The Ordering clause allow us to order the result set according to the values of the specified keywords.
We can specify a list of comma separate couples keyword/order where keyword is a name of the target 
ddl-type and order may be the word ascending, descending, asc or desc.

    keyword_1 asc
	
	keyword_1 asc, keyword_2 desc
