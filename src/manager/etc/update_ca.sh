#!/bin/bash

CUR_FILE=`readlink -f $0`
CUR_DIR=`dirname $CUR_FILE`
DST=${CUR_DIR}/../../../amd64/manager
mkdir -p ${DST}/etc
mkdir -p ${DST}/../../arm/manager/etc

SERVER_VIP=$(ip addr | grep "inet " | grep -v 127 | grep -v 172 | awk '{print $2}' | awk -F"/" '{print $1}')
echo ${SERVER_VIP} > tmp_server.txt

if [ ! -f $DST/etc/crt_last_server.txt ]; then
  touch $DST/etc/crt_last_server.txt
fi
TMP_MD5=$(md5sum tmp_server.txt | awk '{print $1}')
LAST_MD5=$(md5sum $DST/etc/crt_last_server.txt | awk '{print $1}')

if [ ! -f server.key ]; then
  openssl genrsa -out server.key 2048
fi

if [ "$TMP_MD5" != "$LAST_MD5" ]; then
  set -e
  openssl req -new -subj "/C=CN/ST=Beijing/L=Beijing/O=My/CN=${SERVER_VIP}/OU=My" -sha256 -key server.key -out server.csr
  cp -f private.ext /tmp/
  i=1
  while read line
  do
    echo "IP.$i = $line" >> /tmp/private.ext
    ((i=i+1))
  done < tmp_server.txt
  openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -extfile /tmp/private.ext -extensions SAN -out server.crt -days 7300
  rm -f server.csr
  rm -f /tmp/private.ext
  cp -f server.crt $DST/etc/
  cp -f server.crt $DST/../../arm/manager/etc/
  cp -f server.key $DST/etc/
  cp -f server.key $DST/../../arm/manager/etc/
  set +e
  mv -f tmp_server.txt $DST/etc/crt_last_server.txt
else
  rm -f tmp_server.txt
fi

#if [ ! -f /etc/pki/ca-trust/source/anchors/ca.crt ]; then
#  cp /home/work/app/etc/ca.crt /etc/pki/ca-trust/source/anchors/
#  ln -sf /etc/pki/ca-trust/source/anchors/ca.crt /etc/ssl/certs/ca.crt
#  update-ca-trust
#fi

# vim: set expandtab ts=4 sw=4 sts=4:
