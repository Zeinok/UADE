#!/bin/bash
#
# UADE QUADMODE REGRESSION TEST
#
# by Airmann 8/2013
#
# Needs installation of "sox"
#
# See test_quad.log for further info

# adjust your config here
UADE2_DATA_DIR="../share/uade2"
INPUT_FILE="songs/AHX.Cruisin"
OUTPUT_DIR="/var/tmp"
OUTPUT_BASE="${OUTPUT_DIR}/test_quad"
KEEP_WAV_FILES="no"
CREATE_SPECTROGRAM="no"

if [ ! -f "${INPUT_FILE}" ]
then
  echo "ERROR: input file not found"
  exit
fi

for RESAMPLER in "none" "default" "sinc"
do
  for FILTER in "none" "A500" "A1200" 
  do
    OUTPUT_FILE="${OUTPUT_BASE}_${RESAMPLER}_${FILTER}.wav" 

    echo "Generating: ${OUTPUT_FILE}"
    src/frontends/uade123/uade123 --basedir="${UADE2_DATA_DIR}" \
      --quadmode --resampler=$RESAMPLER --filter --force-led=1 \
        --filter=$FILTER -v -f "${OUTPUT_FILE}" \
          "${INPUT_FILE}" > /dev/null  2>>test_quad.log

    if [ -f "${OUTPUT_FILE}" ]
    then
      echo -n "Size: "; du -h "${OUTPUT_FILE}"; 
      echo -n "Checksum: "; sha1sum "${OUTPUT_FILE}";

      # Do Deep check
      QC_FILENAME="${OUTPUT_FILE##*/}"     		# remove all prefix until /
      QC_EXTENSION="${OUTPUT_FILE##*.}"    		# remove all prefix until .
      QC_BASEFILE="${QC_FILENAME%.*}"  			# remove extension from filename
      QC_BASEDIR="${OUTPUT_FILE%"$QC_FILENAME"}" 	# remove filename as suffix
      QC_BASE="${QC_BASEDIR}${QC_BASEFILE}"

      # split into mono channels
      sox "${OUTPUT_FILE}" ${QC_BASE}_L1.wav remix 1  
      sox "${OUTPUT_FILE}" ${QC_BASE}_R1.wav remix 2 
      sox "${OUTPUT_FILE}" ${QC_BASE}_L2.wav remix 3 
      sox "${OUTPUT_FILE}" ${QC_BASE}_R2.wav remix 4
 
      echo -n "     Checksum L1:"; sha1sum "${QC_BASE}_L1.wav"
      echo -n "     Checksum L2:"; sha1sum "${QC_BASE}_L2.wav"
      echo -n "     Checksum R1:"; sha1sum "${QC_BASE}_R1.wav"
      echo -n "     Checksum R2:"; sha1sum "${QC_BASE}_R2.wav"

      if [ "${CREATE_SPECTROGRAM}" == "yes" ]
      then
        sox "${OUTPUT_FILE}" -n spectrogram -o "${QC_BASE}.png"
      	sox "${QC_BASE}_L1.wav" -n spectrogram -o "${QC_BASE}_L1.png"
        sox "${QC_BASE}_L2.wav" -n spectrogram -o "${QC_BASE}_L2.png"
      	sox "${QC_BASE}_R1.wav" -n spectrogram -o "${QC_BASE}_R1.png"
      	sox "${QC_BASE}_R2.wav" -n spectrogram -o "${QC_BASE}_R2.png"
      fi

      if [ "${KEEP_WAV_FILES}" == "no" ]
      then
        rm "${OUTPUT_FILE}"
        rm "${QC_BASE}_L1.wav"
        rm "${QC_BASE}_L2.wav"
        rm "${QC_BASE}_R1.wav"
        rm "${QC_BASE}_R2.wav"
      fi
    else
      echo "ERROR"
    fi
  done
done


