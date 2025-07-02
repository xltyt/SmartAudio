#!/bin/bash

CUR_FILE=`readlink -f $0`
CUR_DIR=`dirname $CUR_FILE`
set -e
set -x

PARAM="-fPIC"
DST=$CUR_DIR/../../../${PLATFORM}/local
DST_C=$(echo $DST | sed 's/\//\\\//g')
#CPU_COUNT=$(cat /proc/cpuinfo | grep "processor" | awk -F": " '{print $2}' | wc -l)
CPU_COUNT=4

mkdir -p ${CUR_DIR}/../../../build
cd ${CUR_DIR}/../../../build

export PATH=$DST/bin:$PATH
export LD_LIBRARY_PATH=$DST/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$DST/lib/pkgconfig:$PKG_CONFIG_PATH

tar zxf $CUR_DIR/pkg/esp-7.0.1-src.tgz
cd esp-7.0.1
sed -i '1998a\        mprWriteFileFmt(app->combineFile, "#include <web_common.h>\\n\\n");' src/esp.c
sed -i '934,937s/^/#/' projects/esp-linux-default.mk
LIBS="-lssl -lcrypto" ARCH=x64 OS=linux ME_ROOT_PREFIX=$DST/esp SHOW=1 make
LIBS="-lssl -lcrypto" ARCH=x64 OS=linux ME_ROOT_PREFIX=$DST/esp SHOW=1 make install
sed -i 's/'\''${CC} -shared ${DEBUG} -Wall.*'\''/'\''date'\''/g' $DST/esp/usr/local/lib/esp/latest/bin/esp-compile.json
cd ..
rm -rf esp-7.0.1
ln -sf $DST/esp/usr/local/bin/esp $DST/bin/esp

tar zxf $CUR_DIR/pkg/expansive-0.7.2-src.tgz
cd expansive-0.7.2
sed -i '925,928s/^/#/' projects/expansive-linux-default.mk
LIBS="-lssl -lcrypto" ARCH=x64 OS=linux ME_ROOT_PREFIX=$DST/expansive SHOW=1 make
LIBS="-lssl -lcrypto" ARCH=x64 OS=linux ME_ROOT_PREFIX=$DST/expansive SHOW=1 make install
cd ..
rm -rf expansive-0.7.2
ln -sf $DST/expansive/usr/local/bin/expansive $DST/bin/expansive

tar zxf $CUR_DIR/pkg/appweb-7.0.1-src.tgz
cd appweb-7.0.1
patch -p1 < $CUR_DIR/pkg/appweb-7.0.1.patch
patch -p1 < $CUR_DIR/pkg/appweb-7.0.1-ssl.patch
sed -i '883,886s/^/#/' projects/appweb-linux-static.mk
APPWEB_PROFILE=static
sed -i 's/PROFILE := default/PROFILE ?= default/g' Makefile
sed -i 's/\/usr\/local//g' projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i '/^DEPS_62 += stop/d' projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i '/^DEPS_62 += start/d' projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i "1023a\\\tmkdir -p \$(ME_VAPP_PREFIX)/bin ; \\\\" projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i "1024a\\\tcp \$(BUILD)/bin/libappweb.a \$(ME_VAPP_PREFIX)/bin/libappweb.a ; \\\\" projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i "1025a\\\tcp \$(BUILD)/bin/libesp.a \$(ME_VAPP_PREFIX)/bin/libesp.a ; \\\\" projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i "1026a\\\tcp \$(BUILD)/bin/libhttp.a \$(ME_VAPP_PREFIX)/bin/libhttp.a ; \\\\" projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i "1027a\\\tcp \$(BUILD)/bin/libmpr-openssl.a \$(ME_VAPP_PREFIX)/bin/libmpr-openssl.a ; \\\\" projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i "1028a\\\tcp \$(BUILD)/bin/libmpr-version.a \$(ME_VAPP_PREFIX)/bin/libmpr-version.a ; \\\\" projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i "1029a\\\tcp \$(BUILD)/bin/libmpr.a \$(ME_VAPP_PREFIX)/bin/libmpr.a ; \\\\" projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i "1030a\\\tcp \$(BUILD)/bin/libpcre.a \$(ME_VAPP_PREFIX)/bin/libpcre.a ; \\\\" projects/appweb-linux-$APPWEB_PROFILE.mk
sed -i '25,33s/^/\/\//' src/server/appweb.c
sed -i '262,267s/^/\/\//' src/server/appweb.c
sed -i '1998a\        mprWriteFileFmt(app->combineFile, "#include <web_common.h>\\n\\n");' src/esp/esp.c
CFLAGS="$PARAM" LDFLAGS="$PARAM -L$DST/lib" LIBS="-lssl -lcrypto" ME_COM_CGI=1 ME_COM_OPENSSL=1 ME_COM_MBEDTLS=0 ME_COM_ESP=1 ARCH=x64 OS=linux ME_ROOT_PREFIX=$DST/appweb SHOW=1 PROFILE=$APPWEB_PROFILE make
CFLAGS="$PARAM" LDFLAGS="$PARAM -L$DST/lib" LIBS="-lssl -lcrypto" ME_COM_CGI=1 ME_COM_OPENSSL=1 ME_COM_MBEDTLS=0 ME_COM_ESP=1 ARCH=x64 OS=linux ME_ROOT_PREFIX=$DST/appweb SHOW=1 PROFILE=$APPWEB_PROFILE make install
sed -i 's/'\''${CC} -shared ${DEBUG} -Wall.*'\''/'\''date'\''/g' $DST/appweb/lib/appweb/latest/bin/esp-compile.json
cd ..
rm -rf appweb-7.0.1

# vim: set expandtab ts=4 sw=4 sts=4:
