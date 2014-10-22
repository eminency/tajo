#!/bin/bash  -x

if [ "$(uname)" == "Darwin" ]; then
  OS="DARWIN"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  OS="LINUX"
else 
    echo "Do not support $(uname) machine"
    exit -1
fi

GTEST_VERSION=1.7.0     # New BSD License
GFLAGS_VERSION=v2.1.1   # New BSD License
GPERF_VERSION=2.1     # New BSD License
GLOG_VERSION=0.3.3      # New BSD License
JSON_VERSION=4.08       # The MIT License
PROTOBUF_VERSION=2.5.0  # New BSD License
HADOOP_VERSION=2.2.0    # Apache License 2.0
RE2_VERSION=20140304    # New BSD License
DOUBLE_CONVERSION_VERSION=1.1.5 # New BSD License

ROOT=`pwd`
INSTALL=`pwd`/local

if [ ${OS} == "DARWIN" ]; then
  _CXXFLAGS="-std=c++11 -stdlib=libc++"
  _CC=/usr/bin/clang
  _CXX=/usr/bin/clang++
else
  _CXXFLAGS=""
fi

for i in "$@"
do
case $i in
    --without-hadoop)
    WITHOUT_HADOOP=YES
    shift
    ;;
    *)
            # unknown option
    ;;
esac
done

if [ ! -d ${INSTALL} ]
then
    mkdir -p ${INSTALL}/lib
    mkdir -p ${INSTALL}/include
fi

if [ ! -f ${INSTALL}/lib/libgtest_main.a ]
then
    echo "Fetching gtest"
    curl -O http://googletest.googlecode.com/files/gtest-${GTEST_VERSION}.zip
    unzip gtest-${GTEST_VERSION}.zip
    rm -f gtest-${GTEST_VERSION}.zip

    echo "Building gtest"
    cd gtest-${GTEST_VERSION}
    CXXFLAGS=${_CXXFLAGS} cmake .
    make -j4
    cp libgtest* $INSTALL/lib
    cp -a include/gtest $INSTALL/include/
else
    echo "gtest is already installed."
fi

cd ${ROOT}

if [ ! -f ${INSTALL}/lib/libgflags.a ]
then
    echo "Fetching gflags"
    N=gflags-${GFLAGS_VERSION}
    if [ ! -d ${N} ]
    then
      git clone -b ${GFLAGS_VERSION} https://github.com/schuhschuh/gflags.git ${N}
    fi

    echo "Building gflags"
    cd ${N}
    if [ -f Makefile ]; then
      make clean
    fi
    CXXFLAGS="${_CXXFLAGS} -fPIC" cmake -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL} && make -j8 && make install
    cd ..
else
    echo "gflags is already installed."
fi

cd ${ROOT}

#if [ ! -f ${INSTALL}/lib/libflatbuffer.a ]
#then
#    echo "Fetching flatbuffer"
#    N=flatbuffer-trunk
#    if [ ! -d ${N} ]
#    then
#      git clone https://github.com/google/flatbuffers.git ${N}
#    fi
#
#    echo "Building flatbuffer"
#    if [ ! -d ${N} ]; then
#      echo "${N} directory does not exist"
#    fi
#    cd ${N}
#    if [ -f Makefile ]; then
#      make clean
#    fi
#    CXXFLAGS="${_CXXFLAGS} -fPIC" cmake -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL} && make -j8 && make install
#    cd ..
#else
#    echo "flatbuffer is already installed."
#fi

cd ${ROOT}

if [ ! -f ${INSTALL}/lib/libprofiler.a ]
then
    N=gperftools-${GPERF_VERSION}.zip
    if [ ! -f ${N} ]
    then
      curl -O https://gperftools.googlecode.com/files/${N}
      unzip ${N}
    fi

    echo "Building gperftools"
    cd gperftools-${GPERF_VERSION}
    CXXFLAGS=${_CXXFLAGS} ./configure --prefix=${INSTALL} --enable-frame-pointers --with-pic
    make -j8 install

    cd ..
else
    echo "gperftools is already installed."
fi

cd ${ROOT}

if [ ! -f ${INSTALL}/lib/libglog.a ]
then
    if [ ! -d glog-trunk ]
    then
      echo "Fetching glog"
      svn checkout http://google-glog.googlecode.com/svn/trunk/ glog-trunk
    else
      cd glog-trunk
      make clean
      cd ..
    fi

    echo "Building glog"
    cd glog-trunk
    CXXFLAGS=${_CXXFLAGS} ./configure --prefix=${INSTALL} --with-pic
    make -j8 install

    cd ..
else
    echo "glog is already installed."
fi

cd ${ROOT}

# JSON Spirit - MIT LICENSE
if [ ! -f ${INSTALL}/lib/libjson_spirit.a ]
then
    N=json_spirit_v${JSON_VERSION}.zip
    D=json_spirit_v${JSON_VERSION}
    if [ ! -f ${N} ]
    then
      echo "INFO: Please manually download json_spirit."
      exit 1
    fi

    unzip $N
    if [ -d ${D} ]
    then
      cd ${D}
      make clean
      rm -rf CMakeCache.txt
      cd ..
    fi
    
    echo "Building json split"
    cd ${D}
  
# For compatibility, removed c++11 flag
#    CXXFLAGS=${_CXXFLAGS} cmake -DCMAKE_INSTALL_PREFIX=${INSTALL} .
    cmake -DCMAKE_INSTALL_PREFIX=${INSTALL} .
    make -j4 install

    cd ..
else
    echo "json_spirit is already installed."
fi

cd ${ROOT}

if [ ! -f ${INSTALL}/lib/libprotobuf.a ]
then
    echo "Fetching protobuf"
    N="protobuf-${PROTOBUF_VERSION}"
    if [ ! -f ${N}.tar.gz ]
    then
      wget https://protobuf.googlecode.com/files/${N}.tar.gz
    fi
    if [ ! -d ${N} ]
    then
      tar -xzvf ${N}.tar.gz
    fi

    echo "Building protobuf"
    cd ${N}
    CXXFLAGS=${_CXXFLAGS} ./configure --with-pic --prefix=${INSTALL}
    make -j4 install

    cd ..
else
    echo "protobuf is already installed."
fi

cd ${ROOT}

if [ ! -f ${INSTALL}/lib/libre2.a ]
then
    echo "Fetching re2"
    F=re2-${RE2_VERSION}.tgz

    if [ ! -f ${F} ]
    then
      wget https://re2.googlecode.com/files/${F}
    fi
    if [ ! -d re2 ]
    then
      tar -xvzf ${F}
    fi

    echo "Building Google RE2"
    cd re2
    sed  -e "s:prefix=/usr/local:prefix=${INSTALL}:g" Makefile > Makefile.tmp
    mv Makefile.tmp Makefile
    CXXFLAGS=${_CXXFLAGS} make -j8 install

    cd ..
else
    echo "re2 is already installed."
fi

cd ${ROOT}

if [ ! -f ${INSTALL}/lib/libdouble-conversion.a ]
then
    N=double-conversion-${DOUBLE_CONVERSION_VERSION}
    F=${N}.tar.gz
    if [ ! -f ${F} ]
    then
      wget https://double-conversion.googlecode.com/files/$F
    fi
    if [ ! -d ${N} ]
    then
      mkdir ${N} && cd ${N}
      tar -xvzf ../${F}
      cd ..
    fi
    
    echo "Building double conversion"
    if [ ! -d ${N} ]; then
      echo "${N} directory does not exist"
    fi
    cd ${N}
    if [ -f Makefile ]; then
      make clean
    fi
    cmake -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL} && make -j8 && make install
    cd ..
else
    echo "flatbuffer is already installed."
fi



cd ${ROOT}

if [[ $WITHOUT_HADOOP -ne "YES" ]] && [[ ! -f ${INSTALL}/lib/libhdfs.a ]];
then
    export PATH=${INSTALL}/bin:${PATH}
    if [ ! -d "hadoop-${HADOOP_VERSION}-src" ]
    then
        curl -O http://apache.mirror.cdnetworks.com/hadoop/common/hadoop-${HADOOP_VERSION}/hadoop-${HADOOP_VERSION}-src.tar.gz
        tar -xvzf hadoop-${HADOOP_VERSION}-src.tar.gz
        rm hadoop-${HADOOP_VERSION}-src.tar.gz
    fi

    OS=`uname`

    if [ $OS = "Darwin" ]
    then
        patch -p1 < hadoop_native.patch
    fi

    echo "Building Hadoop"
    cd hadoop-${HADOOP_VERSION}-src
    mvn package -DskipTests -Pnative -Pdist -Dtar
    if [ $OS = "Darwin" ]
    then
        echo "################################################"
        echo "OSX might failed to build all the native files."
        echo "But it would be fine as Tajo only requires libhdfs."
        echo "################################################"
    fi

    echo "Copying native files"
    cp ./hadoop-hdfs-project/hadoop-hdfs/target/native/target/usr/local/lib/* ${INSTALL}/lib/
    cp ./hadoop-hdfs-project/hadoop-hdfs/src/main/native/libhdfs/hdfs.h ${INSTALL}/include/
    cp -a ./hadoop-dist/target/hadoop-${HADOOP_VERSION} ${INSTALL}/hadoop

    cd ..
else
    echo "Hadoop native is already installed."
fi
