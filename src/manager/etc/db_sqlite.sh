#!/bin/bash

CUR_FILE=`readlink -f $0`
CUR_DIR=`dirname $CUR_FILE`
DST=${CUR_DIR}/../../../amd64/manager

rm -f manager.bin
echo "
PRAGMA KEY='etISzn01z4w7BbQnTYZIShPaXQBq37Ld';

-- Type
-- 0: Admin
-- 1: User
-- 2: Inner
CREATE TABLE system_user(
  'id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  'type' INTEGER,
  'user' TEXT(64),
  'passwd' TEXT(64),
  'mail' TEXT(64),
  'enable' INTEGER DEFAULT 1,
  CONSTRAINT 'user' UNIQUE ('user')
);
INSERT INTO system_user('type', 'user', 'passwd') VALUES(0, 'admin', 'admin');
INSERT INTO system_user('type', 'user', 'passwd') VALUES(2, 'root', 'KPywdueP');

CREATE TABLE template(
  'id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  'time' TEXT(64),
  'filename' TEXT(64),
  'name' TEXT(64),
  'content' TEXT(1024),
  'size' INTEGER,
  'user_id' INTEGER
);

CREATE TABLE generate(
  'id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  'time' TEXT(64),
  'content' TEXT(1024),
  'template_id' INTEGER,
  'user_id' INTEGER,
  'time_used_ms' INTEGER
);

.quit
" | $DST/../local/bin/sqlcipher manager.bin

mkdir -p $DST/etc
mkdir -p $DST/../../arm/manager/etc
cp -f manager.bin $DST/etc/manager.bin
cp -f manager.bin $DST/../../arm/manager/etc/manager.bin

# vim: set expandtab ts=4 sw=4 sts=4:
