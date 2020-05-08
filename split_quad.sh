#!/bin/sh
#
# SPLIT QUAD
#
# Converts a four channel wav file intro four mono channel wav files.
# Needs installation of "sox" 
# 
# by Airmann 7/2013

if [ -z "${1}" ]
then
  echo "split_quad.sh <multichannel_file.wav>"
  exit
fi

if [ ! -f "${1}" ]
then
  echo "File not found: ${1}"
  exit
fi

QS_FILENAME="${1##*/}"        # remove all prefix until /
QS_EXTENSION="${1##*.}"       # remove all prefix until .
QS_BASEFILE="${QS_FILENAME%.*}"  # remove extension from filename
QS_BASEDIR="${1%"$QS_FILENAME"}" # remove filename as suffix
QS_BASE=${QS_BASEDIR}${QS_BASEFILE}

sox "${1}" ${QS_BASE}_L1.wav remix 1  
sox "${1}" ${QS_BASE}_R1.wav remix 2 
sox "${1}" ${QS_BASE}_L2.wav remix 3 
sox "${1}" ${QS_BASE}_R2.wav remix 4 
