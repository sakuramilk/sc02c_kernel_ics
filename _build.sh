#!/bin/bash

KERNEL_DIR=$PWD
INITRAMFS_SRC_DIR=../sc02c_initramfs
INITRAMFS_TMP_DIR=/tmp/sc02c_initramfs

cpoy_initramfs()
{
  if [ -d /tmp/sc02c_initramfs ]; then
    rm -rf /tmp/sc02c_initramfs  
  fi
  cp -a $INITRAMFS_SRC_DIR $(dirname $INITRAMFS_TMP_DIR)
  rm -rf $INITRAMFS_TMP_DIR/.git
  find $INITRAMFS_TMP_DIR -name .gitignore | xargs rm
}


# check target
BUILD_TARGET=$1
case "$BUILD_TARGET" in
  "AOSP" ) BUILD_DEFCONFIG=u1_sc02c_aosp_defconfig ;;
  "SAM" ) BUILD_DEFCONFIG=u1_sc02c_samsung_defconfig ;;
  "MULTI" ) BUILD_DEFCONFIG=u1_sc02c_multi_defconfig ;;
  * ) echo "error: not found BUILD_TARGET" && exit -1 ;;
esac


# generate LOCALVERSION
. mod_version

# set build env
export ARCH=arm
#export CROSS_COMPILE=/opt/toolchains/arm-eabi-4.4.3/bin/arm-eabi-
#export CROSS_COMPILE=/opt/toolchains/arm-2011.03/bin/arm-none-eabi-
export CROSS_COMPILE=/opt/toolchains/gcc-linaro-arm-linux-gnueabi-2012.03-20120326_linux/bin/arm-linux-gnueabi-
export USE_SEC_FIPS_MODE=true
export LOCALVERSION="-$BUILD_LOCALVERSION"

echo "=====> BUILD START $BUILD_KERNELVERSION-$BUILD_LOCALVERSION"

if [ ! -n "$2" ]; then
  echo ""
  read -p "select build? [(a)ll/(u)pdate/(z)Image default:update] " BUILD_SELECT
else
  BUILD_SELECT=$2
fi

# copy initramfs
echo ""
echo "=====> copy initramfs"
cpoy_initramfs


# make start
if [ "$BUILD_SELECT" = 'all' -o "$BUILD_SELECT" = 'a' ]; then
  echo ""
  echo "=====> cleaning"
  make clean
  cp -f ./arch/arm/configs/$BUILD_DEFCONFIG ./.config
  make -C $PWD oldconfig || exit -1
fi

if [ "$BUILD_SELECT" != 'zImage' -a "$BUILD_SELECT" != 'z' ]; then
  echo ""
  echo "=====> build start"
  if [ -e make.log ]; then
    mv make.log make_old.log
  fi
  nice -n 10 make -j12 2>&1 | tee make.log
fi

# check compile error
COMPILE_ERROR=`grep 'error:' ./make.log`
if [ "$COMPILE_ERROR" ]; then
  echo ""
  echo "=====> ERROR"
  grep 'error:' ./make.log
  exit -1
fi

# *.ko replace
find -name '*.ko' -exec cp -av {} /tmp/sc02c_initramfs/lib/modules/ \;

# build zImage
echo ""
echo "=====> make zImage"
nice -n 10 make -j2 zImage CONFIG_INITRAMFS_SOURCE="$INITRAMFS_TMP_DIR" CONFIG_INITRAMFS_ROOT_UID=`id -u` CONFIG_INITRAMFS_ROOT_GID=`id -g` || exit 1

if [ ! -e out ]; then
  mkdir out
fi

echo ""
echo "=====> CREATE RELEASE IMAGE"
# clean release dir
rm $KERNEL_DIR/out/*

# copy zImage
cp arch/arm/boot/zImage ./out/
echo "  out/zImage"

# create odin image
cd $KERNEL_DIR/out
tar cf $BUILD_LOCALVERSION-odin.tar zImage
md5sum -t $BUILD_LOCALVERSION-odin.tar >> $BUILD_LOCALVERSION-odin.tar
mv $BUILD_LOCALVERSION-odin.tar $BUILD_LOCALVERSION-odin.tar.md5
echo "  out/$BUILD_LOCALVERSION-odin.tar.md5"

# create cwm image
cd $KERNEL_DIR/out
if [ -d tmp ]; then
  rm -rf tmp
fi
mkdir -p tmp/META-INF/com/google/android
cp zImage ./tmp/
cp $KERNEL_DIR/release-tools/update-binary $KERNEL_DIR/out/tmp/META-INF/com/google/android/
sed -e "s/@VERSION/$BUILD_LOCALVERSION/g" $KERNEL_DIR/release-tools/updater-script.sed > $KERNEL_DIR/out/tmp/META-INF/com/google/android/updater-script
cd tmp && zip -rq ../cwm.zip ./* && cd ../
SIGNAPK_DIR=$KERNEL_DIR/release-tools/signapk
java -jar $SIGNAPK_DIR/signapk.jar $SIGNAPK_DIR/testkey.x509.pem $SIGNAPK_DIR/testkey.pk8 cwm.zip $BUILD_LOCALVERSION-signed.zip
rm cwm.zip
rm -rf tmp
echo "  out/$BUILD_LOCALVERSION-signed.zip"

cd $KERNEL_DIR
echo ""
echo "=====> BUILD COMPLETE $BUILD_KERNELVERSION-$BUILD_LOCALVERSION"
exit 0
