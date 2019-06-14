#!/bin/bash
if [ -z "$1" ]
then
  echo Usage cvt.sh {file name}
  exit 1
fi
exec MP4Box -add "$1:fps=17" "${1%.*}.mp4" 
