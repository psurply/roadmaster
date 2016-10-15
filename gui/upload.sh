#!/bin/bash

HOST="192.168.103.230"
USER="root"
PASSWD="root"

ftp -i -n $HOST << END_SCRIPT
quote USER $USER
quote PASS $PASSWD
binary
cd /opt/Qtopia/bin/
put ./roadmaster-gui
cd ../apps/Applications/
put ./roadmaster.desktop
quit

END_SCRIPT
