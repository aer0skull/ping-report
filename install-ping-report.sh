#!/bin/bash
mkdir /opt/ping-report
mkdir /opt/ping-report/bin
make
mv ping-report /opt/ping-report/bin
cp ping-report.sh /opt/ping-report/
current_dir=`pwd`
cd /bin
ln -s /opt/ping-report/ping-report.sh ping-report
cd $current_dir
