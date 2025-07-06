#!/bin/bash

openssl genrsa -out ca.key 2048

openssl req -new -subj "/C=CN/ST=Beijing/L=Beijing/O=Ruiguard/CN=Ruiguard" -sha256 -key ca.key -out ca.csr 

openssl x509 -req -in ca.csr -signkey ca.key -out ca.crt -days 7300

rm -f ca.csr

# vim: set expandtab ts=4 sw=4 sts=4:
