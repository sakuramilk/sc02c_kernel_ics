#!/bin/bash

KERNEL_DIR=$PWD

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
bash ./build-samsung.sh a $1
if [ $? != 0 ]; then
  echo 'error: samsung build fail'
  exit -1
fi
mkdir $RELEASE_DIR/SAM
cp -v ./out/SAM/* $RELEASE_DIR/SAM/

# build for aosp
bash ./build-aosp.sh a $1
if [ $? != 0 ]; then
  echo 'error: aosp build fail'
  exit -1
fi
mkdir $RELEASE_DIR/AOSP
cp -v ./out/AOSP/* $RELEASE_DIR/AOSP

# build for multiboot

cd ../sc02c_initramfs
git checkout ics_multiboot
cd $KERNEL_DIR
bash ./build-multi.sh a $1
if [ $? != 0 ]; then
  echo 'error: multi build fail'
  exit -1
fi
mkdir $RELEASE_DIR/MULTI
cp -v ./out/MULTI/* $RELEASE_DIR/MULTI
