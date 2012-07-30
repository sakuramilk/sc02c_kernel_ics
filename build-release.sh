#!/bin/bash

KERNEL_DIR=$PWD

if [ -f $KERNEL_DIR/release.conf ]; then
	BUILD_SAM=`grep BUILD_SAM $KERNEL_DIR/release.conf | cut -d'=' -f2`
	BUILD_AOSP=`grep BUILD_AOSP $KERNEL_DIR/release.conf | cut -d'=' -f2`
	BUILD_MULTI=`grep BUILD_MULTI $KERNEL_DIR/release.conf | cut -d'=' -f2`
	BUILD_COMMON=`grep BUILD_COMMON $KERNEL_DIR/release.conf | cut -d'=' -f2`
else
	BUILD_SAM=1
	BUILD_AOSP=1
	BUILD_MULTI=1
	BUILD_COMMON=1
fi


if [ -z ../sc02c_initramfs ]; then
  echo 'error: sc02c_initramfs directory not found'
  exit -1
fi

cd ../sc02c_initramfs
if [ ! -n "`git status | grep clean`" ]; then
  echo 'error: sc02c_initramfs is not clean'
  exit -1
fi
git checkout ics
cd $KERNEL_DIR

read -p "select build type? [(r)elease/(n)ightly] " BUILD_TYPE
if [ "$BUILD_TYPE" = 'release' -o "$BUILD_TYPE" = 'r' ]; then
  export RELEASE_BUILD=y
else
  unset RELEASE_BUILD
fi

# create release dir＿
RELEASE_DIR=../release/`date +%Y%m%d`
mkdir -p $RELEASE_DIR


# build for samsung
if [ $BUILD_SAM == 1 ]; then
	bash ./build-samsung.sh a $1
	if [ $? != 0 ]; then
	  echo 'error: samsung build fail'
	  exit -1
	fi
	mkdir $RELEASE_DIR/SAM
	cp -v ./out/SAM/bin/* $RELEASE_DIR/SAM/
fi

# build for aosp
if [ $BUILD_AOSP == 1 ]; then
	bash ./build-aosp.sh a $1
	if [ $? != 0 ]; then
	  echo 'error: aosp build fail'
	  exit -1
	fi
	mkdir $RELEASE_DIR/AOSP
	cp -v ./out/AOSP/bin/* $RELEASE_DIR/AOSP
fi

# build for common
if [ $BUILD_COMMON == 1 ]; then
	bash ./build-common.sh a $1
	if [ $? != 0 ]; then
	  echo 'error: common build fail'
	  exit -1
	fi
	mkdir $RELEASE_DIR/COMMON
	cp -v ./out/COMMON/bin/* $RELEASE_DIR/COMMON
fi

# build for multiboot
if [ $BUILD_MULTI == 1 ]; then
	cd ../sc02c_initramfs
	git checkout ics_multiboot
	cd $KERNEL_DIR
	bash ./build-multi.sh a $1
	if [ $? != 0 ]; then
	  echo 'error: multi build fail'
	  exit -1
	fi
	mkdir $RELEASE_DIR/MULTI
	cp -v ./out/MULTI/bin/* $RELEASE_DIR/MULTI
fi


