#!/bin/bash

# $1 contains the library and headers installation directoy eg /opt
if [ $# -eq 0 ]
then
    echo "usage: $0 [-S] <lib-installation-path>"
    exit 0
fi

if [ "$1" == "--help" ]
then
    echo "usage: $0 [-S] <lib-installation-path>
use -S flag if you need to super user priviledges (provided through sudo command) to install the libraries
"
    exit 0
fi

if [ "$1" == "-S" ]
then
    if [ $# -eq 2 ]
    then
	SUDO="sudo"
	INSTALL_PATH="$2"
    else
	echo "usage: $0 [-S] <lib-installation-path>"
	exit 0
    fi	
else
    if [ $# -eq 1 ]
    then
	SUDO=""
	INSTALL_PATH="$1"
    else
    	echo "usage: $0 [-S] <lib-installation-path>"
	exit 0
    fi
fi

touch ~/.profile

if ! cat ~/.profile | grep "#DAS env" > /dev/null
then
   echo "
#DAS env
if [ -f ~/.das/profile ]
then
    . ~/.das/profile
fi
" >> ~/.profile
fi

if [ -f ~/.bash_profile ]
then
    if ! cat ~/.bash_profile | grep "#DAS env" > /dev/null
    then
	echo "
#DAS env
if [ -f ~/.das/profile ]
then
    . ~/.das/profile
fi
" >> ~/.bash_profile   
    fi
fi


mkdir -p ~/.das
echo "
export CMAKE_INCLUDE_PATH=\"$INSTALL_PATH/mysqlclient/include:$INSTALL_PATH/boost_1_54/include:$INSTALL_PATH/odb/include:$INSTALL_PATH/das/include:$INSTALL_PATH/blitz/include:\$CMAKE_INCLUDE_PATH\"
export CMAKE_LIBRARY_PATH=\"$INSTALL_PATH/mysqlclient/lib:$INSTALL_PATH/boost_1_54/lib:$INSTALL_PATH/odb/lib:$INSTALL_PATH/das/lib:$INSTALL_PATH/blitz/lib:\$CMAKE_LIBRARY_PATH\"
export PATH=\"$INSTALL_PATH/swig/bin:\$PATH\"
export CPLUS_INCLUDE_PATH=\"$INSTALL_PATH/boost_1_54/include:$INSTALL_PATH/odb/include:$INSTALL_PATH/das/include:$INSTALL_PATH/blitz/include:\$CPLUS_INCLUDE_PATH\"
export LIBRARY_PATH=\"$INSTALL_PATH/boost_1_54/lib:$INSTALL_PATH/odb/lib:$INSTALL_PATH/das/lib:$INSTALL_PATH/blitz/lib:\$LIBRARY_PATH\"
export LD_LIBRARY_PATH=\"$INSTALL_PATH/boost_1_54/lib:$INSTALL_PATH/odb/lib:$INSTALL_PATH/das/lib:$INSTALL_PATH/blitz/lib:\$LD_LIBRARY_PATH\"
" > ~/.das/profile

. ~/.das/profile

mkdir -p external_libs
cd  external_libs

if [ ! -f mysql-connector-c-6.1.2-src.tar.gz ]
then
    wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.2-src.tar.gz || exit 1
fi

if [ ! -d mysql-connector-c-6.1.2-src ]
then
    tar zxvf mysql-connector-c-6.1.2-src.tar.gz
fi

cd mysql-connector-c-6.1.2-src
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$INSTALL_PATH/mysqlclient" . && \
make && \
$SUDO make install
if [ $? != 0 ]
then
    exit 1
fi

cd ..
#delete shared objects
rm $INSTALL_PATH/mysqlclient/lib/*.so*

if [ ! -f boost_1_54_0.tar.bz2 ]
then
    wget http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.bz2/download -O boost_1_54_0.tar.bz2 || exit 1
fi

if [ ! -d boost_1_54_0 ]
then
    tar jxvf boost_1_54_0.tar.bz2
fi

cd boost_1_54_0
./bootstrap.sh --prefix="$INSTALL_PATH/boost_1_54" && \
./b2
$SUDO ./b2 install
cd ..

if [ ! -f blitz-0.10.tar.gz ]
then
    wget http://sourceforge.net/projects/blitz/files/blitz/Blitz%2B%2B%200.10/blitz-0.10.tar.gz/download -O blitz-0.10.tar.gz || exit 1
fi

if [ ! -d blitz-0.10 ]
then
    tar zxvf blitz-0.10.tar.gz
fi

cd blitz-0.10
./configure --with-boost-libdir="$INSTALL_PATH/boost_1_54/lib" --prefix="$INSTALL_PATH/blitz" && \
make lib && \
$SUDO make install

if [ $? != 0 ]
then
    exit 1
fi
cd ..

if [ ! -f swig-2.0.11.tar.gz ]
then
    wget http://prdownloads.sourceforge.net/swig/swig-2.0.11.tar.gz || exit 1
fi
if [ ! -d swig-2.0.11 ]
then
    tar zxvf swig-2.0.11.tar.gz
fi

cd swig-2.0.11
./configure --prefix="$INSTALL_PATH/swig" && \
make && \
$SUDO make install
if [ $? != 0 ]
then
    exit 1
fi
cd ..

if [ ! -f libodb-2.3.0.tar.bz2 ]
then
    wget http://www.codesynthesis.com/download/odb/2.3/libodb-2.3.0.tar.bz2 || exit 1
fi
if [ ! -d libodb-2.3.0 ]
then
    tar jxvf libodb-2.3.0.tar.bz2
fi

cd libodb-2.3.0
./configure --prefix="$INSTALL_PATH/odb" && \
make && \
$SUDO make install
if [ $? != 0 ]
then
    exit 1
fi
cd ..

if [ ! -f libodb-mysql-2.3.0.tar.bz2 ]
then
    wget http://www.codesynthesis.com/download/odb/2.3/libodb-mysql-2.3.0.tar.bz2 || exit 1
fi
if [ ! -d libodb-mysql-2.3.0 ]
then
    tar jxvf libodb-mysql-2.3.0.tar.bz2
fi

cd libodb-mysql-2.3.0
./configure --prefix="$INSTALL_PATH/odb" LDFLAGS="-L$INSTALL_PATH/mysqlclient/lib" LIBS="-ldl -lrt" CPPFLAGS="-I$INSTALL_PATH/mysqlclient/include" && \
make && \
$SUDO make install
if [ $? != 0 ]
then
    exit 1
fi
cd ..


if [ ! -f libodb-boost-2.3.0.tar.bz2 ]
then
    wget http://www.codesynthesis.com/download/odb/2.3/libodb-boost-2.3.0.tar.bz2 || exit 1
fi
if [ ! -d libodb-boost-2.3.0 ]
then
    tar jxvf libodb-boost-2.3.0.tar.bz2
fi

cd libodb-boost-2.3.0
./configure --prefix="$INSTALL_PATH/odb" && \
make && \
$SUDO make install
if [ $? != 0 ]
then
    exit 1
fi
cd ..
cd ..
#cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_PATH/das" . && \
#make -j && \
#$SUDO make install
#if [ $? != 0 ]
#then
#    exit 1
#fi
#cd ..
