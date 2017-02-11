#!/bin/bash

READ_DATA=/home/pcjc2/gedasrc/Altium_import/git/libopenaltium/read_data

sourcedir=$(pwd)

while IFS= read -d $'\0' -r file ; do
  filename="${file%.*}"
  printf 'File found: %s, making dir %s\n' "$file" "$filename"
  mkdir -p "converted/$filename"
  pushd "converted/$filename"
  gsf cat "$sourcedir/$file" PCBLib/0.pcblib | tail -c +2 | zlib-flate -uncompress > tmp.pcblib
  if [ $? -eq "0" ] ; then
    $READ_DATA -f tmp.pcblib | tee log ;
    (exit ${PIPESTATUS[0]}) || { echo "ERROR PROCESSING FILE '$file'" 2>&1 ; exit -1 ; }
    #rm tmp.pcblib
  fi
  popd
done < <(find . -iname '*.intlib' -print0)

