#!/bin/bash

# $1 contains the library and headers installation directoy eg /opt
if [ $# -eq 0 ]
then
    echo "usage: $0 <lib-installation-path>"
    exit 1
fi

touch ~/.bashrc

if ! cat ~/.bashrc | grep "#DAS env" > /dev/null
then
   echo "#DAS env
if [ -f ~/.das/profile ]
then
    . ~/.das/profile
fi
" >> ~/.bashrc
fi

mkdir -p ~/.das
echo export CMAKE_INCLUDE_PATH="$1/mysqlclient/include:$1/boost_1_54/include:$1/odb/include:$1/das/include:$1/blitz/include:$CMAKE_INCLUDE_PATH" > ~/.das/profile
echo export CMAKE_LIBRARY_PATH="$1/mysqlclient/lib:$1/boost_1_54/lib:$1/odb/lib:$1/das/lib:$1/blitz/lib:$CMAKE_LIBRARY_PATH" >> ~/.das/profile
echo export PATH="$1/swig/bin:$PATH" >> ~/.das/profile
echo export CPLUS_INCLUDE_PATH="$1/mysqlclient/include:$1/boost_1_54/include:$1/odb/include:$1/das/include:$1/blitz/include:$CPLUS_INCLUDE_PATH" >> ~/.das/profile
echo export LIBRARY_PATH="$1/mysqlclient/lib:$1/boost_1_54/lib:$1/odb/lib:$1/das/lib:$1/blitz/lib:$LIBRARY_PATH" >> ~/.das/profile
echo export LD_LIBRARY_PATH="$1/mysqlclient/lib:$1/boost_1_54/lib:$1/odb/lib:$1/das/lib:$1/blitz/lib:$LD_LIBRARY_PATH" >> ~/.das/profile

. ~/.das/profile

if [ ! -f mysql-connector-c-6.1.2-src.tar.gz ]
then
    wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.2-src.tar.gz || exit 1
fi

if [ ! -d mysql-connector-c-6.1.2-src ]
then
    tar zxvf mysql-connector-c-6.1.2-src.tar.gz
fi

cd mysql-connector-c-6.1.2-src
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$1/mysqlclient" -DMYSQL_UNIX_ADDR="/var/run/mysqld/mysqld.sock" . && \
make && \
make install
if [ $? != 0 ]
then
    exit 1
fi

cd ..

if [ ! -f boost_1_54_0.tar.bz2 ]
then
    wget http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.bz2/download
fi

if [ ! -d boost_1_54_0 ]
then
    tar jxvf boost_1_54_0.tar.bz2
fi

cd boost_1_54_0
./bootstrap.sh --prefix="$1/boost_1_54" && \
./b2
./b2 install
cd ..

if [ ! -f blitz-0.10.tar.gz ]
then
    wget http://sourceforge.net/projects/blitz/files/blitz/Blitz%2B%2B%200.10/blitz-0.10.tar.gz/download -O blitz-0.10.tar.gz
fi

if [ ! -d blitz-0.10 ]
then
    tar zxvf blitz-0.10.tar.gz
fi

cd blitz-0.10
./configure --with-boost-libdir="$1/boost_1_54/lib" --prefix="$1/blitz" && \
make lib && \
make install

if [ $? != 0 ]
then
    exit 1
fi
cd ..

if [ ! -f swig-2.0.11.tar.gz ]
then
    wget http://prdownloads.sourceforge.net/swig/swig-2.0.11.tar.gz
fi
if [ ! -d swig-2.0.11 ]
then
    tar zxvf swig-2.0.11.tar.gz
fi

cd swig-2.0.11
./configure --prefix="$1/swig" && \
make && \
make install
if [ $? != 0 ]
then
    exit 1
fi
cd ..

if [ ! -f libodb-2.3.0.tar.bz2 ]
then
    wget http://www.codesynthesis.com/download/odb/2.3/libodb-2.3.0.tar.bz2
fi
if [ ! -d libodb-2.3.0 ]
then
    tar jxvf libodb-2.3.0.tar.bz2
fi

if [ $? != 0 ]
then
    echo "ldconfig did not work"
    exit 1
fi

cd libodb-2.3.0
./configure --prefix="$1/odb" && \
make && \
make install
if [ $? != 0 ]
then
    exit 1
fi
cd ..

if [ ! -f libodb-mysql-2.3.0.tar.bz2 ]
then
    wget http://www.codesynthesis.com/download/odb/2.3/libodb-mysql-2.3.0.tar.bz2
fi
if [ ! -d libodb-mysql-2.3.0 ]
then
    tar jxvf libodb-mysql-2.3.0.tar.bz2
fi

cd libodb-mysql-2.3.0
./configure --prefix="$1/odb" && \
make && \
make install
if [ $? != 0 ]
then
    exit 1
fi
cd ..


if [ ! -f libodb-boost-2.3.0.tar.bz2 ]
then
    wget http://www.codesynthesis.com/download/odb/2.3/libodb-boost-2.3.0.tar.bz2
fi
if [ ! -d libodb-boost-2.3.0 ]
then
    tar jxvf libodb-boost-2.3.0.tar.bz2
fi

cd libodb-boost-2.3.0
./configure --prefix="$1/odb" && \
make && \
make install
if [ $? != 0 ]
then
    exit 1
fi
cd ..

#cmake -DCMAKE_INSTALL_PREFIX="$1/das" . && \
#make -j && \
#make install
#if [ $? != 0 ]
#then
#    exit 1
#fi
#cd ..
