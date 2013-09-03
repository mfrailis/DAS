DAS user manual
===============


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
