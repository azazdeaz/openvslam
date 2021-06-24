DEPS_DIR="$PWD/dependencies"
TMP_DIR="/tmp"
mkdir -p $DEPS_DIR

THREADS=4

cd $TMP_DIR
git clone --depth 1 https://github.com/zeromq/libzmq.git
cd libzmq
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$DEPS_DIR ..
make -j$THREADS
make install

cd $TMP_DIR
git clone --depth 1 https://github.com/zeromq/cppzmq.git
cd cppzmq
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$DEPS_DIR ..
make -j$THREADS
make install
