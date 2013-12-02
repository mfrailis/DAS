INSTALL                                                                                       {#install}
=======

This guide will help you to set-up the CIWSdev VM in order to install the DAS system.

Prerequisites
-------------

First of all, you need to install this packages from yum packet manager:

    MySQL-python
    python-ordereddict

as root, you can simply type

    yum install MySQL-python python-ordereddict


### odb compiler ###

The odb compiler is a program used internally by the DAS system during configuration steps.
You can find the rpm package at
    
	http://www.codesynthesis.com/download/odb/2.3/odb-2.3.0-1.i686.rpm

To install it type, as root

    rpm --install odb-2.3.0-1.i686.rpm

## Additional libraries ##

In order to work, the DAS system needs a few libraries, presented in the following sub-sections.
For your convenience we provide the ciwsprod_install.sh script which will:

  * add the needed environment variables to the user profile file
  * download the source packages
  * build the libraries
  * install the libraries in the given path
  
to run the script just type
  
    ciwsprod_install.sh <lib-path>
   
where <lib-path> is the absolute path where the libs will be installed.
If you choose to use this script you can skip the next sections, until [DAS system](#das_system).

### Boost ###

You need a version of boost libraries at least 1.47 which is newer then the one provided by the packet
manager. Following this commands you will install the boost 1.54 libraries
    
    wget http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.bz2/download
    tar jxvf boost_1_54_0.tar.bz2 
	cd boost_1_54_0
	./bootstrap.sh --prefix=<lib-path>/boost_1_54
	./b2
	./b2 install


### mySQL connector ###

The python wrapper needs mySQL library compiled with a specific __MYSQL_UNIX_ADDR__ option.

    wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.2-src.tar.gz
    tar zxvf mysql-connector-c-6.1.2-src.tar.gz
    cd mysql-connector-c-6.1.2-src
    cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="<lib-path>/mysqlclient" -DMYSQL_UNIX_ADDR="/var/run/mysqld/mysqld.sock"
	make
	make install
	cd ..
	
### odb libraries ###

We need 3 libraries from odb suite: odb common, odb-boost and odb-mysql.
Please note that you may need to configure your environment variables, such as __CPLUS_INCLUDE_PATH__,
__LIBRARY_PATH__ and  __LD_LIBRARY_PATH__ in order to make boost and mySQL libs available to the odb
and DAS system.

    wget http://www.codesynthesis.com/download/odb/2.3/libodb-2.3.0.tar.bz2
    tar jxvf libodb-2.3.0.tar.bz2
	cd libodb-2.3.0
	./configure --prefix="<lib-path>/odb"
	make
	make install
	cd ..
	
	wget http://www.codesynthesis.com/download/odb/2.3/libodb-mysql-2.3.0.tar.bz2
	tar jxvf libodb-mysql-2.3.0.tar.bz2
	cd libodb-mysql-2.3.0
	./configure --prefix="<lib-path>/odb"
	make
	make install
	cd ..
	
	wget http://www.codesynthesis.com/download/odb/2.3/libodb-boost-2.3.0.tar.bz2
	tar jxvf libodb-boost-2.3.0.tar.bz2
	cd libodb-boost-2.3.0
	./configure --prefix="<lib-path>/odb"
	make
	make install
	cd ..
	
### Blitz++ ###
Blitz++ is a C++ class library for scientific computing which provides performance on par with
Fortran 77/90. It uses template techniques to achieve high performance.
We use Blitz++ as a back-end of the das::Array template class.
	
	wget http://sourceforge.net/projects/blitz/files/blitz/Blitz%2B%2B%200.10/blitz-0.10.tar.gz/download -O blitz-0.10.tar.gz
	tar zxvf blitz-0.10.tar.gz
	cd blitz-0.10
	./configure --prefix="<lib-dir>/blitz"
	make lib
	make install
	cd ..
	
### SWIG ###
SWIG is a software development tool that connects programs written in C and C++ with a variety of
high-level programming languages. We use SWIG to provide the python wrapper.

    wget http://prdownloads.sourceforge.net/swig/swig-2.0.11.tar.gz
	tar zxvf swig-2.0.11.tar.gz
	cd swig-2.0.11
	./configure --prefix="<lib-path>/swig"
	make
	make install   
	
<a name='das_system'></a>
## DAS system ##

First of all, you need to set two cmake environment variables in order to allow cmake to find boost
and odb. Assuming you've installed boost and odb in /opt and you also want to install das under /opt
you can follow this example. If you used the ciwsprod_install.sh to install the libraries, you
should already have these variables set.

    export CMAKE_INCLUDE_PATH=/opt/boost_1_54/include:/opt/odb/include:/opt/das/include
    export CMAKE_LIBRARY_PATH=/opt/boost_1_54/lib:/opt/odb/lib:/opt/das/lib
   
If you used the ciwsprod_install.sh and you still don't have those environment variables, you can
load them from the DAS profile:

    . ~/.das/profile


#### Build das ####
The das system needs 3 configuration files in order to work:
  * $DAS_ROOT/configure/config.json
  * $DAS_ROOT/ddl/ddl.xml
  * ~/.das/access.json

  
You can find the related documentation in the [CONFIGURE](md_CONFIGURE.html#das_config) section. 

When you've edit the configuration files, you need to run the configuration step, allowing cmake
to find the needed libraries and the das system to validate the files edited by the user (DDL files
and config.json). Furthermore you need to specify the installation directory if 
differs from the system one (/usr/local). Again, assuming you want to install the das libraries
under /opt directory, you can simply type
       
    cmake -DCMAKE_INSTALL_PREFIX=/opt/das .

Note that the current building system allows only in-tree building. That is to say that you must run
the cmake command inside the das building root directory.

If the configuration was successfully done, you can build the library executing the make command.

    make

Now, you need to instantiate the schemas on the database. Before running the following target, you need
to create in the back-end DBMS a database named "test_level2" and grant to the user referenced in the
~/.das/access.json config file the db-manager priviledges.

    make db-test_level2

To test if everything if properly set, the unit tests should build and run without any error.

	make unit_tests
    ./unit_tests


#### make targets ####
Install the header files and the library in __CMAKE_INSTALL_PREFIX__ directory, if provided on
configuration time, in /usr/local otherwise. Note that you may need root privileges.

    make install

Generate .sql sources for the database referred by the alias property in the config.json file in 
build/db/<alias> directory

    make schema-<alias>

Instantiate the schemas in the database referred by the alias property in the config.json file

    make db-<alias>

Compile the library as shared object
    
    make DAS_SO

Compile the library as static object
    
    make DAS_A
	

Compile examples. The executable files will be located in build directory

    make examples

If doxygen is installed in the system, an additional target for make is created in order to generate
the code documentation

    make doc

