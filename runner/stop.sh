#!/bin/sh
TTY=/dev/ttyUSB0
echo "M42 P27 S255" >$TTY
echo "M106 P0 S0" >$TTY
echo "M18" >$TTY
