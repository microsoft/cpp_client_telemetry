#!/bin/sh

tc qdisc del dev eth0 root

echo ==========================================================================
echo All traffic control filters:
tc qdisc show dev eth0

echo ==========================================================================
echo Adding new filters...
tc qdisc add dev eth0 root handle 1: htb
tc class add dev eth0 parent 1: classid 1:1 htb rate 100mbit
tc filter add dev eth0 parent 1: protocol ip prio 1 u32 flowid 1:1 match ip dst 104.208.28.54

tc qdisc add dev eth0 root netem delay 100ms 10ms 25%
# tc qdisc add dev eth0 root netem corrupt 50%

echo ==========================================================================
echo All traffic control filters:
tc qdisc show dev eth0
echo ==========================================================================
