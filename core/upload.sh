#!/bin/bash

HOST="192.168.103.230"
USER="root"
PASSWD="root"

ftp -i -n $HOST << END_SCRIPT
quote USER $USER
quote PASS $PASSWD
binary
cd /usr/bin/
put ./roadmaster-core
quit

END_SCRIPT
