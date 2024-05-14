#!/bin/sh

BINUTILS_VERSION=2.42
GCC_VERSION=13.2.0

PREFIX="$HOME/opt/cross"
TARGET=x86_64-elf
export PATH=$PATH:$HOME/opt/cross

GCC_TAR_URL=https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz
BINUTILS_TAR_URL=https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz

mkdir -p $HOME/opt/patch

cp patch/x86_64-elf/* $HOME/opt/patch
cd $HOME/opt

echo "Downloading .xz files"
curl -o gcc-$GCC_VERSION.tar.xz $GCC_TAR_URL
curl -o binutils-$BINUTILS_VERSION.tar.xz $BINUTILS_TAR_URL

echo "Extracting .xz files"
tar -xf gcc-$GCC_VERSION.tar.xz
tar -xf binutils-$BINUTILS_VERSION.tar.xz

echo "Patching gcc-$GCC_VERSION"
cp patch/gcc.config gcc-$GCC_VERSION/gcc/
cp patch/t-x86_64-elf gcc-$GCC_VERSION/gcc/config/i386/

mkdir build-binutils
cd build-binutils

echo "Configuring binutils-$BINUTILS_VERSION"
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror

echo "Building binutils-$BINUTILS_VERSION"
make -j$(nproc)
make -j$(nproc) install

echo "Configuring gdb-$BINUTILS_VERSION"
make -j$(nproc) all-gdb
make -j$(nproc) install-gdb

cd ..
rm -rf build-binutils

mkdir build-gcc
cd build-gcc

echo "Configuring gcc-$GCC_VERSION"
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx

echo "Building gcc-$GCC_VERSION"
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make -j$(nproc) all-target-libstdc++-v3

echo "Installing gcc-$GCC_VERSION"
make -j$(nproc) install-gcc
make -j$(nproc) install-target-libgcc
make -j$(nproc) install-target-libstdc++-v3

cd ..
rm -rf build-gcc