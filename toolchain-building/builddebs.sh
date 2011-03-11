#!/bin/bash
set -e

INPUT=0
WRKDIR=$(pwd)
BUILDDIR=../../freeems-toolchain-build/
RELEASE_VER=5
BUILDDIRBIN=${BUILDDIR}/binutils-mc9s12x
SRCDIRBIN=../binutils-2.20/
BUILDDIRBINXGATE=${BUILDDIR}/binutils-mc9xgate
SRCDIRBIN=../binutils-2.20/
BUILDDIRGCC=${BUILDDIR}/gcc-mc9s12x
BUILDDIRNEWLIB=${BUILDDIR}newlib-mc9s12x
AUTOINSTALL=NO

function builds12x(){
rm -Rf $BUILDDIRBIN
mkdir $BUILDDIRBIN
tar -cjf ${BUILDDIRBIN}/binutils-2.20.tar.bz2 $SRCDIRBIN

cp -R binutils-s12x/debian $BUILDDIRBIN
cp -R binutils-s12x/example $BUILDDIRBIN
#cp -R binutils-s12x/patches $BUILDDIRBIN

cd $BUILDDIRBIN
#fakeroot debian/rules clean
#fakeroot debian/rules build
fakeroot debian/rules binary

if [ -f ${BUILDDIRBIN}/binutils-mc9s12x_2.20-0+${RELEASE_VER}_i386.deb ]; then
  echo "binutils-mc9s12x package built successfully"
  #read 
  dpkg -i ${BUILDDIRBIN}/binutils-mc9s12x_2.20-0+${RELEASE_VER}_i386.deb; 
else 
  echo "exiting please review output"; 
  read
  exit -1; 
fi
cd $WRKDIR
}


function buildxgate(){
rm -Rf $BUILDDIRBINXGATE
mkdir $BUILDDIRBINXGATE
tar -cjf ${BUILDDIRBINXGATE}/binutils-2.20.tar.bz2 $SRCDIRBIN

cp -R binutils-xgate/debian $BUILDDIRBINXGATE
cp -R binutils-xgate/example $BUILDDIRBINXGATE
#cp -R binutils-xgate/patches $BUILDDIRBINXGATE

cd $BUILDDIRBINXGATE
fakeroot debian/rules binary

if [ -f ${BUILDDIRBINXGATE}/binutils-mc9xgate_2.20-0+${RELEASE_VER}_i386.deb ]; then
  echo "binutils-mc9xgate packages built successfully"
  #read 
  dpkg -i ${BUILDDIRBINXGATE}/binutils-mc9xgate_2.20-0+${RELEASE_VER}_i386.deb; 
else 
  echo "please review output"; 
  read
#  exit -1; 
fi
cd $WRKDIR
}


function buildgcc {
# Build GCC with new binutils #
rm -Rf $BUILDDIRGCC
mkdir $BUILDDIRGCC
apt-get source gcc-m68hc1x
mv gcc-* $BUILDDIRGCC


cp gcc/rules ${BUILDDIRGCC}/gcc-m68hc1x-3.3.6*/debian/rules
cp gcc/control ${BUILDDIRGCC}/gcc-m68hc1x-3.3.6*/debian/control
cp gcc/changelog ${BUILDDIRGCC}/gcc-m68hc1x-3.3.6*/debian/changelog

cd ${BUILDDIRGCC}/gcc-m68hc1x-3.3.6*
#fakeroot debian/rules binary
fakeroot dpkg-buildpackage -b -uc 

if [ -f ${BUILDDIRGCC}/gcc-mc9s12x_3.3.6+3.1+dfsg-3+${RELEASE_VER}_i386.deb ]; then
  echo "gcc-mc9s12x packages built successfully"
  #read 
  dpkg -i ${BUILDDIRGCC}/gcc-mc9s12x_3.3.6+3.1+dfsg-3+${RELEASE_VER}_i386.deb; 
else 
  echo "error building gcc debian package, please review output"; 
  exit -1; 
fi

cd $WRKDIR
}

function buildnewlib {
# Build GCC with new binutils #
rm -Rf $BUILDDIRNEWLIB
mkdir $BUILDDIRNEWLIB
apt-get source newlib-m68hc1x
mv newlib-* $BUILDDIRNEWLIB
mv newlib_* $BUILDDIRNEWLIB

cp newlib/rules ${BUILDDIRNEWLIB}/newlib-1.18.0/debian/rules
cp newlib/control ${BUILDDIRNEWLIB}/newlib-1.18.0/debian/control
cp newlib/changelog ${BUILDDIRNEWLIB}/newlib-1.18.0/debian/changelog

cd ${BUILDDIRNEWLIB}/newlib-1.18.0
fakeroot debian/rules binary

cd $WKDIR
}

mkdir $BUILDDIR
#read
builds12x
#read
buildxgate
#read
buildgcc
#read
buildnewlib
#read
#read
exit 0;
#read








