#!/bin/bash
INPUT=0
WRKDIR=$(pwd)
BUILDDIRBIN=/usr/src/binutils-mc9s12x
SRCDIRBIN=../binutils-2.20/

BUILDDIRGCC=/usr/src/gcc-mc9s12x

BUILDDIRNEWLIB=/usr/src/newlib-mc9s12x



rm -Rf $BUILDDIRBIN
mkdir $BUILDDIRBIN
tar -cjf ${BUILDDIRBIN}/binutils-2.20.tar.bz2 $SRCDIRBIN

cp -R binutils/debian $BUILDDIRBIN
cp -R binutils/example $BUILDDIRBIN
cp -R binutils/patches $BUILDDIRBIN

cd $BUILDDIRBIN
fakeroot debian/rules binary

if [ -f /usr/src/binutils-mc9s12x_2.20-0_i386.deb ]; then
  echo "press enter to install binutils deb package"
  read $INPUT
  dpkg -i /usr/src/binutils-mc9s12x_2.20-0_i386.deb; 
else 
  echo "error building binutils debian package, please review output"; 
  exit -1; 
fi

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

if [ -f /usr/src/gcc-mc9s12x/gcc-m68hc1x_3.3.6+3.1+dfsg-3_i386.deb ]; then
  echo "press enter to install gcc deb package"
  read $INPUT
  dpkg -i /usr/src/gcc-mc9s12x/gcc-m68hc1x_3.3.6+3.1+dfsg-3_i386.deb; 
else 
  echo "error building gcc debian package, please review output"; 
  exit -1; 
fi

cd $WRKDIR


# Build GCC with new binutils #
rm -Rf $BUILDDIRNEWLIB
mkdir $BUILDDIRNEWLIB
apt-get source newlib-m68hc1x
mv newlib-* $BUILDDIRNEWLIB





