#!/bin/sh
args=""
dir=`dirname $0`
if [ -z "$1" ]
then
  args="-t 0"
fi
args="$args `$dir/camera_settings.sh`"
exec /opt/vc/bin/raspistill -fp -fli 60hz -mm matrix $args $@
