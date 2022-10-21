#!/bin/bash
INSTALL_PATH=$HOME/sshfs/

rm -r .build
mkdir .build
cd .build

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/poky.cmake \
    -DPOKY_TOOLCHAIN_DIR=$OECORE_NATIVE_SYSROOT \
    -DCMAKE_FIND_ROOT_PATH=$OECORE_TARGET_SYSROOT \
    -DTARGET_ROOTFS_DIR=$SDKTARGETSYSROOT \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
    .. && make -j4
