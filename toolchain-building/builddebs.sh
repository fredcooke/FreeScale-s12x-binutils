#!/bin/bash

WRKDIR=$(pwd)
BUILDDIRBIN=/usr/src/binutils-mc9s12x
SRCDIRBIN=../binutils-2.20/

BUILDDIRGCC=/usr/src/gcc-mc9s12x



rm -Rf $BUILDDIRBIN
mkdir $BUILDDIRBIN
tar -cjf ${BUILDDIRBIN}/binutils-2.20.tar.bz2 $SRCDIRBIN

cp -R binutils/debian $BUILDDIRBIN
cp -R binutils/example $BUILDDIRBIN
cp -R binutils/patches $BUILDDIRBIN
read
cd $BUILDDIRBIN
fakeroot debian/rules binary
cd $WRKDIR
read


# Build GCC with new binutils #
rm -Rf $BUILDDIRGCC
mkdir $BUILDDIRGCC
apt-get source gcc-m68hc1x
mv gcc-* $BUILDDIRGCC


cp gcc/rules ${BUILDDIRGCC}/gcc-m68hc1x-3.3.6*/debian/rules
cp gcc/control ${BUILDDIRGCC}/gcc-m68hc1x-3.3.6*/debian/control

cd ${BUILDDIRGCC}/gcc-m68hc1x-3.3.6*
fakeroot debian/rules binary
cd $WRKDIR






