#!/bin/bash

set -e
set -x

awk '
/esp_app_smart_combine/ { print; in_combine=1; next }
!in_combine { print; next }
{ rest[++n]=$0 }
END {
    for(i=1;i<=n;i++) if(rest[i]~/esp_view/) print rest[i]
    for(i=1;i<=n;i++) if(rest[i]!~/esp_view/) print rest[i]
}' cache/smart.c > cache/smart_awk.c

mv -f cache/smart_awk.c cache/smart.c

# vim: set expandtab ts=4 sw=4 sts=4:
