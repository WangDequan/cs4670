#!/bin/bash
for file in *.jpg
do
  mogrify -rotate "90" "$file"
  convert $file -resize 25% $(basename $file .jpg).rsz.jpg
done

