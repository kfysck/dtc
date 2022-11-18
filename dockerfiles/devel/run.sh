#!/bin/bash

sleep_count=0

while [ $sleep_count -le 100 ]
do
    if [ -f "/usr/local/dtc/conf/dtc.yaml" ]; then 
        mkdir /usr/local/dtc/conf
        cp /usr/local/etc/conf/log4cplus.conf /usr/local/dtc/conf/
        cp /usr/local/etc/conf/log4cplus-hwc.conf /usr/local/dtc/conf/
        cp /usr/local/etc/conf/log4cplus-wd.conf /usr/local/dtc/conf/
        cp /usr/local/etc/conf/log4cplus-agent.conf /usr/local/dtc/conf/
        cp /usr/local/etc/conf/log4cplus-async.conf /usr/local/dtc/conf/
        cp /usr/local/etc/conf/log4cplus-life.conf /usr/local/dtc/conf/
        cp /usr/local/etc/conf/my.conf /usr/local/dtc/conf/
        echo "Start running process: "$DTC_BIN","$DTC_ARGV
        if [ $DTC_ARGV == "-a" ]; then
            echo "sleeping for agent"
            sleep 5
        fi
        cd /usr/local/dtc/bin/
        ./$DTC_BIN $DTC_ARGV
        break
    else
        echo "sleeping: "$sleep_count"s"
        sleep 1s
        let sleep_count+=1
    fi
done
echo "Timeout waitting for dtc conf files."
