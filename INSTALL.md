INSTALL
=======

Installation, step by step, of the das library on the CIWSdev VM.


Prerequisites
-------------

You need to install this packages from yum packet manager:

    mysql-devel
    MySQL-python
    python-ordereddict

as root, you can simply type
    yum install MySQL-python python-ordereddict

### Boost ###

You need a version of boost libraries at least 1.47 which is newer then the one provided by the packet
manager. We provide the archive boost_1_54.tar.bz2 with the compiled boost libraries.
We suggest to install it in /opt directory.

If you want to compile the boost libraries by your seelf, you can follow this commands.
    
   wget http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.bz2/download
   tar jxvf boost_1_54_0.tar.bz2 
   cd boost_1_54_0
   ./bootstrap.sh --prefix=/opt/boost_1_54
   ./b2

then as root

   ./b2 install


### odb compiler ###

Odb compiler is a program used internally by the das system during configuration steps.
You can find the rpm package at
    
	http://www.codesynthesis.com/download/odb/2.3/odb-2.3.0-1.i686.rpm

To install it type, as root

    rpm --install odb-2.3.0-1.i686.rpm


### odb libraries ###

The current odb libraries need to be patched in order to fix a bug and work properly with the das system.
To ease the task we provide the odb.tar.bz2 archive with odb and odb-mysql libs already patched
and compiled. We suggest to install it in /opt directory.


### das system ###

First retrive the lastest version of the system

   svn checkout svn://ciws.iasfbo.inaf.it/Ciws/DAS/trunk das

Then you need to set two cmake environment variables in order to allow cmake to find boost and odb.
Assuming you've installed boost and odb in /opt and you also want to install das under /opt you can
follow this example

   export CMAKE_INCLUDE_PATH=/opt/boost_1_54/include:/opt/odb/include:/opt/das/include
   export CMAKE_LIBRARY_PATH=/opt/boost_1_54/lib:/opt/odb/lib:/opt/das/lib


#### Configure das  ####

The das system needs 2 configuration files in order to work:
    configure/config.json
    ~/.das/access.json

config.json contains information needed to access the database and is read in the configuration step,
while access.json contains the database access information and is read runtime.

The access.json file looks like this:

    [
        {
	    "alias"     : "test_level1",
	    "user"      : "username",
	    "password"  : "secret"
    	},
    	{
	    "alias"     : "test_level2",
	    "user"      : "username",
	    "password"  : "secret"
        }
    ]


We provide a sample configure/config.json. You may need to edit the file according to your database
configuration. To allow the sample programs to work properly you need to edit the host property,
which must contain a name or an IP address where to find a mysql server.
The mysql server must contain a database named as specified in db_name property. Moreover the user
specified in the access.json must have SELECT, UPDATE and DELETE privileges on that database.
In order to instantiate the das-generated schemas, the user also needs CREATE permission.


#### Build das ####

Firt you need run the configuration step, allowing cmake to find the needed libraries and the das system
to validate the files edited by the user (DDL files and config.json). Fourtermore you need to specify 
the installation directory if differes from the system one (/usr/local).
Again, assuming you want to install the das libraries under /opt directory type
       
    cmake -DCMAKE_INSTALL_PREFIX=/opt/das .

Note that the current building system allows only in-tree building. That is to say that you must run
the cmake command inside the das building root directory.

If the configuration was succesfully done, you can build the library executing the make command.

    make

Now, you need to instantiate the schemas on the database.

    make db-all

To test if everything if properly set, the test program should run without throw any exceptions

    ./build/test


##### make targets #####
Install the header files and the library in CMAKE_INSTALL_PREFIX directory, if provided on configuration
time, in /usr/local otherwise. Note that you may need root privileges.

    make install

Generate .sql sources for the database referred by the alias property in the config.json file in 
build/db/<alias> directory

    make schema-<alias>

Instantiate the schemas in the database referred by the alias property in the config.json file

    make db-<alias>

Compile the library
    
    make das

Compile the test program. The executable file will be located in buld directory

    make test

Compile examples. The executable files will be located in build directory

    make examples

If doxygen is installed in the system, an additional target for make is created in order to generate the 
code documentation

    make doc

Currently, it creates only the html documentation in the directory doc/html. There is only a main page
containing an high level view of the DAS library, while the public classes and methods are not yet 
documented inline.


### DDL file ###
In the source folder 'ddl' you find a first example of DDL file, 'ddl_level1_types.xml', providing
some data type definitions, modeling the DPS measure relational model and some additional types
representing housekeeping data and raw image frames.
