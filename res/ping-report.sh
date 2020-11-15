#!/bin/bash

BIN=/opt/ping-report/bin/ping-report
PID=/var/log/ping-report/pid.log

case $1 in
start)      sudo $BIN;
            echo "ping-report started";;
end)        sudo kill `cat $PID`;
            sudo rm $PID;
            echo "ping-report ended";;
restart)    sudo kill `cat $PID`;
            sudo $BIN;
            echo "ping-report restarted";;
status)     if test -f "$PID"; then
                echo "ping-report is alive";
            else
                echo "ping-report is not started";
            fi;;
*)          echo "Usage : ping-report [start | end | restart]";
            exit 1;;
esac

