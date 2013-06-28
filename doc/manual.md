DAS user manual
===============


Managed object model
--------------------

The way which DAS mamages persistent objects is inspired by some concepts in the JPA (Java Persistent
API) standard. Objects in a managed context are automatically synchronized with their parsisted
counterpart.

Preliminary definitions:

  * _ddl-object_ : object, that is, instance of a c++ class, that maps a specifid DDL data type.
  
  * _pu-instance_: (Persistence Unit instance) couple of database and and shared directory fully managed
	  by the system. Each pu-istance is uniquely identified by a string set in the "db-alias" option
	  of the das configuration file.
	  
  * _pm-object_: (Persistance Manager object) instance of the class Database, that handles connections,
	 transactions, io and other operations on a specific pu-instance.
	 Multiple pm-objects referring the same pu-instance can be active (in the same scope) at
	 the same time.
	 
  * _managed context_: block of code (scope) where at least one pm-object is alive.

  * _persistent counterpart_: data stored in a database and shared filesystem representing
	  a ddl-object data structure(s).
    
A ddl-object can be in one of the following states:

  - _new_: a transient ddl-object without persistent counterpart.
	  A fresh created object is in the new state and it can pass to attached state trought
	  the Database::persist() method.
	  
  - _attached_: a ddl-object with a persistent counterpart managed by a database instance 
      (pm-object) in a managed context.
	  
  - _detached_: an object with a persistent counterpart stored in a pu-instance but not managed
	  by any pm-object.  An attached objects automatically becomes detached when the managed context
	  ends; as say the related database instance goes out of scope or is manually destroyed.
	  A detachted ddl-object can be later attached to the owner pu-instance througth the
	  Database::attach() method. This means that the new and the old database instace must referr the
	  same pu-instance.

### DDL objects life-cycle ###

A ddl-object newly created is in the new state. You can be freely modify and delete without involve
any database operation.

~~~{.cpp} 
    shared_ptr<measure> m = measure::create("measure_name");
    m->run_id(12345);
    m.reset();
~~~

Once you want to make a ddl-object persistent, you first have to create a pm-object, then create a
transaction and finally you can call the method persist passing the ddl-object as argoment.
When you have finished persisting the objects, just call the commit method from the transaction.

~~~{.cpp} 
    shared_ptr<measure> m1 = measure::create("measure1");
    shared_ptr<measure> m2 = measure::create("measure2");
	
    shared_ptr<Database> db = Database::create("test_level1");
	
    Transaction t(db->begin());
    db->persist(m1);
    db->persist(m2);
    t.commit();
~~~
	
At this point the ddl-objects (m1, m2) have become persistent and they are attached on a 
pm-object (db). From now on, each modification on the attached objects will be made persistent on next
transaction commit created by the corresponding pm-object (db). As your convenience, you can use
the method flush when you want to persist the modifications of each object attached to a specific
pm-object.

~~~{.cpp} 
    ...

    m1->run_id(2344443);
    m2->run_id(2344443);

    db->flush();
~~~

If your application spend most of the time elaborating data with no interaction with pm-objects until
the the very end of the programm, you may find convenient load from a pm-object the needed 
ddl-objects, then delete the pm-object allowing the system to release resources, do your 
computation, and finally update the persistent objects attaching the modified ddl-objects to a new
pm-object which refers the same pu-instance as the first one.


~~~{.cpp}
    shared_ptr<Database> db = Database::create("test_level1");

    shared_ptr<measure> m1 = db->load<measure>(34);
    shared_ptr<measure> m2 = db->load<measure>(35);

    db.reset();	

    ... // very long computation

    db =  Database::create("test_level1");

    db->attach(m1);
    db->attach(m2);

    db->flush();
~~~
