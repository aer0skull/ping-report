#!/bin/bash

BIN=/opt/ping-report/bin/ping-report
PID=/var/log/ping-report/pid.log

case $1 in
start) sudo $BIN;;
end) sudo kill `cat $PID`;sudo rm $PID;;
restart) sudo kill `cat $PID`;sudo $BIN;;
*) echo "Usage : ping-report [start | end | restart]";exit 1;;
esac

