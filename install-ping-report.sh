#!/bin/bash

#Variables
CUR_DIR=`pwd`
OPT_DIR=/opt/ping-report
BIN_OPT_DIR=/opt/ping-report/bin
BIN_DIR=/bin
LOG_DIR=/var/log/ping-report
STATUS_LOG=/var/log/ping-report/status.log
CONF_DIR=/etc/opt/ping-report
CONF=./res/ping-report.conf
DB_SCRIPT=./res/ping-report-db.sql
DB_DIR=/srv/ping-report
DB=/srv/ping-report/ping-report.db
BIN=ping-report
SCRIPT=./res/ping-report.sh
SCRIPT_DIR=/opt/ping-report/ping-report.sh
DYN_LINK=/bin/ping-report

#Start install script

#Check 1st arg
case $1 in
--full-install) rm $DB;
                echo "full install of ping-report ...";;
-f)             rm $DB;
                echo "full install of ping-report ...";;
*)              echo "default install of ping-report ...";;
esac

#Create OPT_DIR
if test -d "$OPT_DIR"; then
    echo "opt dir already exists, no actions needed."
else
    mkdir $OPT_DIR
fi

#Create BIN_OPT_DIR
if test -d "$BIN_OPT_DIR"; then
    echo "bin opt dir already exists, no actions needed."
else
    mkdir $BIN_OPT_DIR
fi

#Create LOG_DIR
if test -d "$LOG_DIR"; then
    echo "log dir already exists, no actions needed"
else
    mkdir $LOG_DIR
fi

#Create / Erase STATUS_LOG
touch $STATUS_LOG
chmod 666 $STATUS_LOG

#Create CONF_DIR
if test -d "$CONF_DIR"; then
    echo "conf dir already exists, no actions needed"
else
    mkdir $CONF_DIR
fi

#Create DB_DIR
if test -d "$DB_DIR"; then
    echo "database dir already exists, no actions needed"
else
    mkdir $DB_DIR
fi

#Compile ping-report
make

#Move ping-report bin to BIN_OPT_DIR
mv $BIN $BIN_OPT_DIR

#Copy launch script to OPT_DIR
cp $SCRIPT $OPT_DIR

#Copy conf file to CONF_DIR
cp $CONF $CONF_DIR

#Create DYN_LINK
if test -f "$DYN_LINK"; then
    echo "dynamic link already exists, no actions needed."
else
    #Change current directory to /bin to create dynamic link
    cd $BIN
    #Create the dynamic link to SCRIPT
    ln -s $SCRIPT_DIR $BIN
    #Change directory to the previous one
    cd $CUR_DIR
fi

#Create SQLITE DB

if test -f "$DB"; then
    echo "db already exists, no actions needed"
else
    cd $DB_DIR
    sqlite3 ping-report.db < $CUR_DIR/$DB_SCRIPT
    cd $CUR_DIR
fi
