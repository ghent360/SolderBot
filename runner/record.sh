#!/bin/sh
args=""
dir=`dirname $0`
if [ -z "$1" ]
then
  args="-t 0"
fi
args="$args -fps 40 -stm -b 12000000"
args="$args `$dir/video_settings.sh`"
echo /opt/vc/bin/raspivid -fli 60hz -mm matrix $args $@
exec /opt/vc/bin/raspivid -fli 60hz -mm matrix $args $@
