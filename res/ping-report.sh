#!/bin/bash

BIN=/opt/ping-report/bin/ping-report
PID=/var/log/ping-report/pid.log
STATUS=/var/log/ping-report/status.log

case $1 in
start)      sudo echo "STARTED" > $STATUS;
            sudo $BIN;
            echo "ping-report started";;
end)        sudo echo "STOP" > $STATUS;
            sudo rm $PID;
            sleep 2;
            sudo echo "ENDED" > $STATUS;
            echo "ping-report ended";;
kill)       sudo kill `cat $PID`;
            sudo rm $PID;
            sudo echo "ENDED" > $STATUS;
            echo "ping-report killed";;
restart)    sudo echo "STOP" > $STATUS;
            sleep 2;
            sudo rm $PID;
            sudo echo "STARTED" > $STATUS;
            sudo $BIN;
            echo "ping-report restarted";;
status)     if test -f "$PID"; then
                echo "ping-report is alive";
            else
                echo "ping-report is not started";
            fi;;
*)          echo "Usage : ping-report [start | end | kill | restart]";
            exit 1;;
esac

