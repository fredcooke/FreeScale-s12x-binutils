#!/bin/bash


#
#  Makes Debs for Jaunty, Karmic, Lucid, Maverick and Lenny
#

WORKDIR=`pwd`
BINUTILS="${WORKDIR}"/binutils-2.20.tar.bz2
OUTDIR="${WORKDIR}"/Output
OTHERMIRROR=/var/cache/pbuilder/repo
BUILDDIR="${WORKDIR}"/build
NEWLIBDIR=newlib-1.18.0
BINUTILSPKGS="binutils-s12x binutils-xgate"
GCCTARBALL="gcc-mc9s12x_3.3.6+3.1+dfsg-3+7.tar.gz"

# Builds the deb pkgs.  Assumes pdebuild has been setup and configured
# previously and has the rootimages setup for the distros specified
#
function build_debs {
#DEB_RELEASES="lenny jaunty karmic lucid"
DEB_RELEASES="lucid"
for dist in `echo "${DEB_RELEASES}"` ; do
	echo "Building for Distro $dist"
	DESTDIR="${OUTDIR}"/"${dist}"
	if [ ! -d "${DESTDIR}" ] ; then
		mkdir -p "${DESTDIR}"
	fi
	find ${OTHERMIRROR}  -type f -exec rm -f {} \;
	find ${DESTDIR} -type f -name "*.deb" -exec cp -a {} ${OTHERMIRROR} \;
#pdebuild --buildresult "${DESTDIR}" --pbuilderroot "sudo DIST=${dist}" -- --othermirror "deb file:///${DESTDIR} ./"
	pdebuild --buildresult "${DESTDIR}" --pbuilderroot "sudo DIST=${dist}"
	if [ $? -ne 0 ] ; then
		echo "Build failure for dist $dist"
		exit -1
	fi
	find ${OTHERMIRROR}  -type f -exec rm -f {} \;
done
}


# Need to make multiple versions of binutils, 
# one for S12x, and one for Xgate
function build_binutils {
if [ ! -f "${BINUTILS}" ] ; then
	pushd ..
	tar cvzf "${BINUTILS}" binutils-2.20
	popd
fi

for pkg in `echo "${BINUTILSPKGS}"` ; do
	mkdir -p "${BUILDDIR}"/"${pkg}"
	cp -a "${pkg}"/* "${BUILDDIR}"/"${pkg}"
	cp -a "${WORKDIR}"/binutils-2.20.tar.bz2 "${BUILDDIR}"/"${pkg}"
	pushd "${BUILDDIR}"/"${pkg}"
	build_debs 
	popd
done
return $?
}

# Need to make GCC next
function build_gcc {
if [ ! -f "${WORKDIR}"/"${GCCTARBALL}" ] ; then
	echo "MISSING GCC tarball ${GCCTARBALL}"
	exit -1
fi
mkdir -p "${BUILDDIR}"/gcc
pushd gcc
tar xfz "${WORKDIR}"/"${GCCTARBALL}"
pushd gcc-m68hc1x-3.3.6+3.1+dfsg
cp -a "${WORKDIR}"/gcc/* ./debian
build_debs
popd
popd
return $?
}

# Need to make Newlib
function build_newlib {
pushd "${BUILDDIR}"
apt-get source newlib-source
pushd "${NEWLIBDIR}"
cp -a "${WORKDIR}"/newlib/* ./debian
build_debs
popd
popd
return $?
}

function clean {
rm  -f "${BINUTILS}"
rm -rf "${BUILDDIR}"
}

build_binutils
build_gcc
build_newlib

