#!/bin/sh
TTY=/dev/ttyUSB0
echo "M106 P0 S255" >$TTY
echo "M42 P27 S0" >$TTY
