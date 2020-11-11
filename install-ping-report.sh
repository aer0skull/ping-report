#!/bin/bash

#Variables
CUR_DIR=`pwd`
OPT_DIR=/opt/ping-report
BIN_OPT_DIR=/opt/ping-report/bin
BIN_DIR=/bin
BIN=ping-report
SCRIPT=ping-report.sh
SCRIPT_DIR=/opt/ping-report/ping-report.sh
DYN_LINK=/bin/ping-report

#Start install script

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

#Compile ping-report
make

#Move ping-report bin to BIN_OPT_DIR
mv $BIN $BIN_OPT_DIR

#Copy launch script to OPT_DIR
cp $SCRIPT $OPT_DIR

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

