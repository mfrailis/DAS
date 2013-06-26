DAS user manual
===============


Managed object model
--------------------

The way which DAS mamages persistent objects is inspired by some concepts in the JPA (Java Persistent
API) standard. Objects in a managed context are automatically synchronized with their parsisted
counterpart.

Preliminary definitions:

  + _ddl-object_ : object, that is, instance of a c++ class, that maps a specifid DDL data type. 
  
  + _managed context_: block of code (scope) where at least one _pu-object_ is reachable.

  + _persistent counterpart_: data stored in a database and shared filesystem representing
	  a _ddl-object_ data structure(s).
  
  + _pu-instance_: (Persistent Unit instance) couple of database and and shared directory fully managed
	  by the system. Each pu-istance is uniquely identified by a string set in the "db-alias" option
	  of the das configuration file.
  
  + _pu-instance-bound_: a _ddl-object_ has this property when is bound to a specific _pu-instance_.
	  As say that the _persistent counterpart_ of this object will eventually be stored on
	  the specific _pu-instance_.
  
  + _pu-object_: object, instance of the class Database, that handles connections, transactions, io
	  and other operations on a specific _pu-instance_.
	  Multiple pu-objects referring the same _pu-instance_ can be active (in the same scope) at
	  the same time.
  
  + _pu-object-bound_: _ddl-object_ bound to a specific _db-object_.
  
A _ddl-object_ can be in one of the following state:

  + _new_: a transient _ddl-object_ without _persistent counterpart_.
	  A fresh created object is in the _new_ state and it can pass to _attached_ state trought
	  the _pu-object_.persist() method.
	  
  + _attached_: a _ddl-object_ with a _persistent counterpart_ managed by a _pu-object_ in a 
	  _persistent context_. Objects in this state are also _pu-instance-bound_ and _pu-object-bound_.
	  Objects in _detached_ state can pass in _attached_ state througth
	  the _pu-object_.attach() method.
	  
  + _detached_: an object with a persistent counterpart stored in a _pu-instance_ but not managed
	  by any _pu-object_. Objects in this state are also _pu-instance-bound_.
	  An _attached_ objects automatically becomes _detached_ when the _managed context_ ends; as say 
	  the related _pu-object_ is destroyed.


### DDL objects life-cycle ###

A ddl-object newly created is in the new state. You can be freely modify and delete without involve
any database operation.

     shared_ptr<measure> m = measure::create("measure_name");
	 m->run_id(12345);
	 m.reset();

Once you want to make a ddl-object persistent, you first have to create a pu-object, then create a
transaction and finally you can call the method persist passing the ddl-object as argoment.
When you have finished persisting the objects, just call the commit method from the transaction.

	shared_ptr<measure> m1 = measure::create("measure1");
	shared_ptr<measure> m2 = measure::create("measure2");
	
    shared_ptr<Database> db = Database::create("test_level1");
	
	Transaction t(db->begin());
	db->persist(m1);
	db->persist(m2);
	t.commit();
	
At this point the ddl-objects (m1, m2) have become persistent and they are attached on a 
pu-object (db). From now on, each modification on the attached objects will be made persistent on next
transaction commit created by the realitive pu-object (db). As your convenience, you can use
the method flush when you want to persist the modifications of each object attached to a specific
pu-object.
    
    ...
	
    m1->run_id(2344443);
    m2->run_id(2344443);
	
	db->flush();
	
If your application spend most of the time elaborating data with no interaction with pu-objects until
the the very end of the programm, you may find convenient load from a pu-object the needed 
ddl-objects, then delete the pu-object allowing the system to release resources, do your 
computation, and finally update the persistent objects attaching the modified ddl-objects to a new
pu-object which refers the same pu-instance as the first one.

    shared_ptr<Database> db = Database::create("test_level1");
	
	shared_ptr<measure> m1 = db->load<measure>(34);
	shared_ptr<measure> m2 = db->load<measure>(35);
	
	db.reset();
	
	
	... // very long computation
	
	db =  Database::create("test_level1");
	
	db->attach(m1);
	db->attach(m2);
	
	db->flush();
	
	

